import re

import pandas as pd


def resolve_item(paths, obj):
    """Find item in document from a reference path"""

    if len(paths) == 0:
        return obj

    if paths[0] == "#":
        return resolve_item(paths[1:], obj)

    try:
        key = int(paths[0])
    except:
        key = paths[0]

    if len(paths) == 1:
        if isinstance(key, str) and key in obj:
            return obj[key]
        elif isinstance(key, int) and key < len(obj):
            return obj[key]
        else:
            return None

    elif len(paths) > 1:
        if isinstance(key, str) and key in obj:
            return resolve_item(paths[1:], obj[key])
        elif isinstance(key, int) and key < len(obj):
            return resolve_item(paths[1:], obj[key])
        else:
            return None

    else:
        return None


def to_legacy_document_format(doc_glm, doc_leg={}, update_name_label=False):
    """Convert Document object (with `body`) to its legacy format (with `main-text`)"""

    doc_leg["main-text"] = []
    doc_leg["figures"] = []
    doc_leg["tables"] = []
    doc_leg["page-headers"] = []
    doc_leg["page-footers"] = []
    doc_leg["footnotes"] = []
    doc_leg["equations"] = []

    if "properties" in doc_glm:
        props = pd.DataFrame(
            doc_glm["properties"]["data"], columns=doc_glm["properties"]["headers"]
        )
    else:
        props = pd.DataFrame()

    for pelem in doc_glm["page-elements"]:
        ptype = pelem["type"]
        span_i = pelem["span"][0]
        span_j = pelem["span"][1]

        if "iref" not in pelem:
            # print(json.dumps(pelem, indent=2))
            continue

        iref = pelem["iref"]

        if re.match("#/figures/(\\d+)/captions/(.+)", iref):
            # print(f"skip {iref}")
            continue

        if re.match("#/tables/(\\d+)/captions/(.+)", iref):
            # print(f"skip {iref}")
            continue

        path = iref.split("/")
        obj = resolve_item(path, doc_glm)

        if obj is None:
            print(f"warning: undefined {path}")
            continue

        if ptype == "figure":
            text = ""
            for caption in obj["captions"]:
                text += caption["text"]

                for nprov in caption["prov"]:
                    npaths = nprov["$ref"].split("/")
                    nelem = resolve_item(npaths, doc_glm)

                    if nelem is None:
                        print(f"warning: undefined caption {npaths}")
                        continue

                    span_i = nelem["span"][0]
                    span_j = nelem["span"][1]

                    text = caption["text"][span_i:span_j]

                    pitem = {
                        "text": text,
                        "name": nelem["name"],
                        "type": nelem["type"],
                        "prov": [
                            {
                                "bbox": nelem["bbox"],
                                "page": nelem["page"],
                                "span": [0, len(text)],
                            }
                        ],
                    }
                    doc_leg["main-text"].append(pitem)

            find = len(doc_leg["figures"])

            figure = {
                "confidence": obj.get("confidence", 0),
                "created_by": obj.get("created_by", ""),
                "type": obj.get("type", "figure"),
                "cells": [],
                "data": [],
                "text": text,
                "prov": [
                    {
                        "bbox": pelem["bbox"],
                        "page": pelem["page"],
                        "span": [0, len(text)],
                    }
                ],
            }
            doc_leg["figures"].append(figure)

            pitem = {
                "$ref": f"#/figures/{find}",
                "name": pelem["name"],
                "type": pelem["type"],
            }
            doc_leg["main-text"].append(pitem)

        elif ptype == "table":
            text = ""
            for caption in obj["captions"]:
                text += caption["text"]

                for nprov in caption["prov"]:
                    npaths = nprov["$ref"].split("/")
                    nelem = resolve_item(npaths, doc_glm)

                    if nelem is None:
                        print(f"warning: undefined caption {npaths}")
                        continue

                    span_i = nelem["span"][0]
                    span_j = nelem["span"][1]

                    text = caption["text"][span_i:span_j]

                    pitem = {
                        "text": text,
                        "name": nelem["name"],
                        "type": nelem["type"],
                        "prov": [
                            {
                                "bbox": nelem["bbox"],
                                "page": nelem["page"],
                                "span": [0, len(text)],
                            }
                        ],
                    }
                    doc_leg["main-text"].append(pitem)

            tind = len(doc_leg["tables"])

            table = {
                "#-cols": obj.get("#-cols", 0),
                "#-rows": obj.get("#-rows", 0),
                "confidence": obj.get("confidence", 0),
                "created_by": obj.get("created_by", ""),
                "type": obj.get("type", "table"),
                "cells": [],
                "data": obj["data"],
                "text": text,
                "prov": [
                    {"bbox": pelem["bbox"], "page": pelem["page"], "span": [0, 0]}
                ],
            }
            doc_leg["tables"].append(table)

            pitem = {
                "$ref": f"#/tables/{tind}",
                "name": pelem["name"],
                "type": pelem["type"],
            }
            doc_leg["main-text"].append(pitem)

        elif "text" in obj:
            text = obj["text"][span_i:span_j]

            type_label = pelem["type"]
            name_label = pelem["name"]
            if update_name_label and len(props) > 0 and type_label == "paragraph":
                prop = props[
                    (props["type"] == "semantic") & (props["subj_path"] == iref)
                ]
                if len(prop) == 1 and prop.iloc[0]["confidence"] > 0.85:
                    name_label = prop.iloc[0]["label"]

            pitem = {
                "text": text,
                "name": name_label,  # pelem["name"],
                "type": type_label,  # pelem["type"],
                "prov": [
                    {
                        "bbox": pelem["bbox"],
                        "page": pelem["page"],
                        "span": [0, len(text)],
                    }
                ],
            }
            doc_leg["main-text"].append(pitem)

        else:
            pitem = {
                "name": pelem["name"],
                "type": pelem["type"],
                "prov": [
                    {"bbox": pelem["bbox"], "page": pelem["page"], "span": [0, 0]}
                ],
            }
            doc_leg["main-text"].append(pitem)

    return doc_leg


def to_xml_format(doc_glm, normalised_pagedim: int = -1):
    result = "<document>\n"

    page_dims = pd.DataFrame()
    if "page-dimensions":
        page_dims = pd.DataFrame(doc_glm["page-dimensions"])

    for pelem in doc_glm["page-elements"]:
        ptype = pelem["type"]
        span_i = pelem["span"][0]
        span_j = pelem["span"][1]

        if "iref" not in pelem:
            # print(json.dumps(pelem, indent=2))
            continue

        iref = pelem["iref"]

        page = pelem["page"]
        bbox = pelem["bbox"]

        x0 = bbox[0]
        y0 = bbox[1]
        x1 = bbox[2]
        y1 = bbox[3]

        if normalised_pagedim > 0 and len(page_dims[page_dims["page"] == page]) > 0:
            page_width = page_dims[page_dims["page"] == page].iloc[0]["width"]
            page_height = page_dims[page_dims["page"] == page].iloc[0]["height"]

            rx0 = float(x0) / float(page_width) * normalised_pagedim
            rx1 = float(x1) / float(page_width) * normalised_pagedim

            ry0 = float(y0) / float(page_height) * normalised_pagedim
            ry1 = float(y1) / float(page_height) * normalised_pagedim

            x0 = max(0, min(normalised_pagedim, round(rx0)))
            x1 = max(0, min(normalised_pagedim, round(rx1)))

            y0 = max(0, min(normalised_pagedim, round(ry0)))
            y1 = max(0, min(normalised_pagedim, round(ry1)))

        elif normalised_pagedim > 0 and len(page_dims[page_dims["page"] == page]) == 0:
            print(f"ERROR: no page dimensions for page {page}")

        if re.match("#/figures/(\\d+)/captions/(.+)", iref):
            # print(f"skip {iref}")
            continue

        if re.match("#/tables/(\\d+)/captions/(.+)", iref):
            # print(f"skip {iref}")
            continue

        path = iref.split("/")
        obj = resolve_item(path, doc_glm)

        if obj is None:
            print(f"warning: undefined {path}")
            continue

        if ptype == "figure":
            result += f"<figure bbox=[{x0}, {y0}, {x1}, {y1}]></figure>\n"

        elif ptype == "table":
            result += f"<table bbox=[{x0}, {y0}, {x1}, {y1}]></table>\n"

        elif "text" in obj:
            text = obj["text"][span_i:span_j]
            text_type = pelem["type"]

            result += (
                f"<{text_type} bbox=[{x0}, {y0}, {x1}, {y1}]>{text}</{text_type}>\n"
            )
        else:
            continue

    result += "</document>"

    return result
