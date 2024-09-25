import re
from typing import List

import pandas as pd
from docling_core.types.experimental.base import BoundingBox, CoordOrigin, Size
from docling_core.types.experimental.document import DoclingDocument, FileInfo, BaseFigureData, BaseTableData, \
    TableCell, ProvenanceItem, PageItem

from docling_core.types.experimental.labels import DocItemLabel


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


def _flatten_table_grid(grid: List[List[dict]]) -> List[dict]:
    unique_objects = []
    seen_spans = set()

    for sublist in grid:
        for obj in sublist:
            # Convert the spans list to a tuple of tuples for hashing
            spans_tuple = tuple(tuple(span) for span in obj['spans'])
            if spans_tuple not in seen_spans:
                seen_spans.add(spans_tuple)
                unique_objects.append(obj)

    return unique_objects

def to_docling_document(doc_glm, update_name_label=False) -> DoclingDocument:
    doc: DoclingDocument = DoclingDocument(description={},
                                           file_info=FileInfo(document_hash=doc_glm["file-info"]["document-hash"]))

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
            caption_refs = []
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
                    doc_glm["page-elements"].remove(nelem)

                    prov = ProvenanceItem(page_no=nelem["page"], charspan=tuple(nelem["span"]), bbox=BoundingBox.from_tuple(nelem["bbox"], origin=CoordOrigin.BOTTOMLEFT))

                    caption_obj = doc.add_paragraph(label=DocItemLabel.CAPTION, text=text, prov=prov)
                    caption_refs.append(caption_obj.get_ref())

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

            prov = ProvenanceItem(page_no=pelem["page"], charspan=(0, len(text)),
                                  bbox=BoundingBox.from_tuple(pelem["bbox"], origin=CoordOrigin.BOTTOMLEFT))

            fig = doc.add_figure(data=BaseFigureData())
            fig.captions.extend(caption_refs)

        elif ptype == "table":
            text = ""
            caption_refs = []
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
                    doc_glm["page-elements"].remove(nelem)

                    prov = ProvenanceItem(page_no=pelem["page"], charspan=nelem["span"],
                                          bbox=BoundingBox.from_tuple(pelem["bbox"], origin=CoordOrigin.BOTTOMLEFT))

                    caption_obj = doc.add_paragraph(label=DocItemLabel.CAPTION, text=text, prov=prov)
                    caption_refs.append(caption_obj.get_ref())


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

            table_cells_glm = _flatten_table_grid(obj["data"])

            table_cells = []
            for tbl_cell_glm in table_cells_glm:
                table_cells.append(
                    TableCell(
                        row_span=tbl_cell_glm["row-span"][1]-tbl_cell_glm["row-span"][0],
                        col_span=tbl_cell_glm["col-span"][1]-tbl_cell_glm["col-span"][0],
                        start_row_offset_idx=tbl_cell_glm["row-span"][0],
                        end_row_offset_idx=tbl_cell_glm["row-span"][1],
                        start_col_offset_idx=tbl_cell_glm["col-span"][0],
                        end_col_offset_idx=tbl_cell_glm["col-span"][1],
                        text=tbl_cell_glm["text"],
                    ) # TODO: add "type" (col_header, row_header, body, ...)
                )
            """
                row_span: int = 1
                col_span: int = 1
                start_row_offset_idx: int
                end_row_offset_idx: int
                start_col_offset_idx: int
                end_col_offset_idx: int
                text: str
                column_header: bool = False
                row_header: bool = False
                row_section: bool = False
            """
            tbl_data = BaseTableData(num_rows=obj.get("#-rows", 0), num_cols=obj.get("#-cols", 0), table_cells=table_cells)

            prov = ProvenanceItem(page_no=pelem["page"], charspan=(0, 0),
                                  bbox=BoundingBox.from_tuple(pelem["bbox"], origin=CoordOrigin.BOTTOMLEFT))

            tbl = doc.add_table(data=tbl_data, prov=prov)
            tbl.captions.extend(caption_refs)

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
            prov = ProvenanceItem(page_no=pelem["page"], charspan=(0, len(text)),
                                  bbox=BoundingBox.from_tuple(pelem["bbox"], origin=CoordOrigin.BOTTOMLEFT))

            doc.add_paragraph(label=DocItemLabel(name_label), text=text, prov=prov)

        else:
            pitem = {
                "name": pelem["name"],
                "type": pelem["type"],
                "prov": [
                    {"bbox": pelem["bbox"], "page": pelem["page"], "span": [0, 0]}
                ],
            }
            # This branch should not be reachable.

    page_to_hash = {
        item["page"]: item["hash"]
        for item in doc_glm["file-info"]["page-hashes"]
    }

    for page_dim in doc_glm["page-dimensions"]:
        page_no = int(page_dim["page"])
        size = Size(width=page_dim["width"], height=page_dim["height"])
        hash = page_to_hash[page_no]

        pitem = doc.add_page(page_no=page_no, size=size, hash=hash)

    return doc

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
