import re
from pathlib import Path
from typing import List

import pandas as pd
from docling_core.types.doc import (
    BoundingBox,
    CoordOrigin,
    DocItemLabel,
    DoclingDocument,
    DocumentOrigin,
    GroupLabel,
    ProvenanceItem,
    Size,
    TableCell,
    TableData,
)


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
            spans_tuple = tuple(tuple(span) for span in obj["spans"])
            if spans_tuple not in seen_spans:
                seen_spans.add(spans_tuple)
                unique_objects.append(obj)

    return unique_objects


def to_docling_document(doc_glm, update_name_label=False) -> DoclingDocument:
    origin = DocumentOrigin(
        mimetype="application/pdf",
        filename=doc_glm["file-info"]["filename"],
        binary_hash=doc_glm["file-info"]["document-hash"],
    )
    doc_name = Path(origin.filename).stem

    doc: DoclingDocument = DoclingDocument(name=doc_name, origin=origin)

    if "properties" in doc_glm:
        props = pd.DataFrame(
            doc_glm["properties"]["data"], columns=doc_glm["properties"]["headers"]
        )
    else:
        props = pd.DataFrame()

    current_list = None

    for ix, pelem in enumerate(doc_glm["page-elements"]):
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
            current_list = None
            print(f"warning: undefined {path}")
            continue

        if ptype == "figure":
            current_list = None
            text = ""
            caption_refs = []
            for caption in obj["captions"]:
                text += caption["text"]

                for nprov in caption["prov"]:
                    npaths = nprov["$ref"].split("/")
                    nelem = resolve_item(npaths, doc_glm)

                    if nelem is None:
                        # print(f"warning: undefined caption {npaths}")
                        continue

                    span_i = nelem["span"][0]
                    span_j = nelem["span"][1]

                    cap_text = caption["text"][span_i:span_j]

                    # doc_glm["page-elements"].remove(nelem)

                    prov = ProvenanceItem(
                        page_no=nelem["page"],
                        charspan=tuple(nelem["span"]),
                        bbox=BoundingBox.from_tuple(
                            nelem["bbox"], origin=CoordOrigin.BOTTOMLEFT
                        ),
                    )

                    caption_obj = doc.add_text(
                        label=DocItemLabel.CAPTION, text=cap_text, prov=prov
                    )
                    caption_refs.append(caption_obj.get_ref())

            prov = ProvenanceItem(
                page_no=pelem["page"],
                charspan=(0, len(text)),
                bbox=BoundingBox.from_tuple(
                    pelem["bbox"], origin=CoordOrigin.BOTTOMLEFT
                ),
            )

            pic = doc.add_picture(prov=prov)
            pic.captions.extend(caption_refs)

        elif ptype == "table":
            current_list = None
            text = ""
            caption_refs = []
            for caption in obj["captions"]:
                text += caption["text"]

                for nprov in caption["prov"]:
                    npaths = nprov["$ref"].split("/")
                    nelem = resolve_item(npaths, doc_glm)

                    if nelem is None:
                        # print(f"warning: undefined caption {npaths}")
                        continue

                    span_i = nelem["span"][0]
                    span_j = nelem["span"][1]

                    cap_text = caption["text"][span_i:span_j]

                    # doc_glm["page-elements"].remove(nelem)

                    prov = ProvenanceItem(
                        page_no=nelem["page"],
                        charspan=tuple(nelem["span"]),
                        bbox=BoundingBox.from_tuple(
                            nelem["bbox"], origin=CoordOrigin.BOTTOMLEFT
                        ),
                    )

                    caption_obj = doc.add_text(
                        label=DocItemLabel.CAPTION, text=cap_text, prov=prov
                    )
                    caption_refs.append(caption_obj.get_ref())

            table_cells_glm = _flatten_table_grid(obj["data"])

            table_cells = []
            for tbl_cell_glm in table_cells_glm:
                if tbl_cell_glm["bbox"] is not None:
                    bbox = BoundingBox.from_tuple(
                        tbl_cell_glm["bbox"], origin=CoordOrigin.BOTTOMLEFT
                    )
                else:
                    bbox = None

                is_col_header = False
                is_row_header = False
                is_row_section = False

                if tbl_cell_glm["type"] == "col_header":
                    is_col_header = True
                elif tbl_cell_glm["type"] == "row_header":
                    is_row_header = True
                elif tbl_cell_glm["type"] == "row_section":
                    is_row_section = True

                table_cells.append(
                    TableCell(
                        row_span=tbl_cell_glm["row-span"][1]
                        - tbl_cell_glm["row-span"][0],
                        col_span=tbl_cell_glm["col-span"][1]
                        - tbl_cell_glm["col-span"][0],
                        start_row_offset_idx=tbl_cell_glm["row-span"][0],
                        end_row_offset_idx=tbl_cell_glm["row-span"][1],
                        start_col_offset_idx=tbl_cell_glm["col-span"][0],
                        end_col_offset_idx=tbl_cell_glm["col-span"][1],
                        text=tbl_cell_glm["text"],
                        bbox=bbox,
                        column_header=is_col_header,
                        row_header=is_row_header,
                        row_section=is_row_section,
                    )
                )

            tbl_data = TableData(
                num_rows=obj.get("#-rows", 0),
                num_cols=obj.get("#-cols", 0),
                table_cells=table_cells,
            )

            prov = ProvenanceItem(
                page_no=pelem["page"],
                charspan=(0, 0),
                bbox=BoundingBox.from_tuple(
                    pelem["bbox"], origin=CoordOrigin.BOTTOMLEFT
                ),
            )

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

            prov = ProvenanceItem(
                page_no=pelem["page"],
                charspan=(0, len(text)),
                bbox=BoundingBox.from_tuple(
                    pelem["bbox"], origin=CoordOrigin.BOTTOMLEFT
                ),
            )
            label = DocItemLabel(name_label)

            if label == DocItemLabel.LIST_ITEM:
                if current_list is None:
                    current_list = doc.add_group(label=GroupLabel.LIST, name="list")

                # TODO: Infer if this is a numbered or a bullet list item
                doc.add_list_item(
                    text=text, enumerated=False, prov=prov, parent=current_list
                )
            elif label == DocItemLabel.SECTION_HEADER:
                current_list = None

                doc.add_heading(text=text, prov=prov)
            else:
                current_list = None

                doc.add_text(label=DocItemLabel(name_label), text=text, prov=prov)

    for page_dim in doc_glm["page-dimensions"]:
        page_no = int(page_dim["page"])
        size = Size(width=page_dim["width"], height=page_dim["height"])

        doc.add_page(page_no=page_no, size=size)

    return doc


def to_legacy_document_format(doc_glm, doc_leg={}, update_name_label=False):
    """Convert Document object (with `body`) to its legacy format (with `main-text`)"""

    reverse_label_mapping = {
        DocItemLabel.CAPTION.value: "caption",
        DocItemLabel.FOOTNOTE.value: "footnote",
        DocItemLabel.FORMULA.value: "formula",
        DocItemLabel.LIST_ITEM.value: "list-item",
        DocItemLabel.PAGE_FOOTER.value: "page-footer",
        DocItemLabel.PAGE_HEADER.value: "page-header",
        DocItemLabel.PICTURE.value: "picture",  # low threshold adjust to capture chemical structures for examples.
        DocItemLabel.SECTION_HEADER.value: "section-header",
        DocItemLabel.TABLE.value: "table",
        DocItemLabel.TEXT.value: "text",
        DocItemLabel.TITLE.value: "title",
        DocItemLabel.DOCUMENT_INDEX.value: "document index",
        DocItemLabel.CODE.value: "code",
        DocItemLabel.CHECKBOX_SELECTED.value: "checkbox-selected",
        DocItemLabel.CHECKBOX_UNSELECTED.value: "checkbox-unselected",
        DocItemLabel.FORM.value: "form",
        DocItemLabel.KEY_VALUE_REGION.value: "key-value region",
        DocItemLabel.PARAGRAPH.value: "paragraph",
        "subtitle-level-1": "subtitle-level-1",
    }

    extra_mappings = {}
    for v in reverse_label_mapping.values():
        extra_mappings[v] = v
        # extra_mappings[v.lower()] = v
    reverse_label_mapping = {**reverse_label_mapping, **extra_mappings}

    layout_label_to_ds_type = {
        DocItemLabel.TITLE: "title",
        DocItemLabel.DOCUMENT_INDEX: "table-of-contents",
        DocItemLabel.SECTION_HEADER: "subtitle-level-1",
        DocItemLabel.CHECKBOX_SELECTED: "checkbox-selected",
        DocItemLabel.CHECKBOX_UNSELECTED: "checkbox-unselected",
        DocItemLabel.CAPTION: "caption",
        DocItemLabel.PAGE_HEADER: "page-header",
        DocItemLabel.PAGE_FOOTER: "page-footer",
        DocItemLabel.FOOTNOTE: "footnote",
        DocItemLabel.TABLE: "table",
        DocItemLabel.FORMULA: "equation",
        DocItemLabel.LIST_ITEM: "paragraph",
        DocItemLabel.CODE: "paragraph",
        DocItemLabel.PICTURE: "figure",
        DocItemLabel.TEXT: "paragraph",
        DocItemLabel.PARAGRAPH: "paragraph",
    }
    extra_mappings = {}
    for v in layout_label_to_ds_type.values():
        # extra_mappings[v[:1].upper() + v[1:]] = v # capitalize
        extra_mappings[v] = v
    layout_label_to_ds_type = {**layout_label_to_ds_type, **extra_mappings}

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
                        "name": reverse_label_mapping[nelem["name"]],
                        "type": layout_label_to_ds_type[nelem["type"]],
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
                "name": reverse_label_mapping[pelem["name"]],
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
                        "name": reverse_label_mapping[nelem["name"]],
                        "type": layout_label_to_ds_type[nelem["type"]],
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
                "name": reverse_label_mapping[pelem["name"]],
                "type": pelem["type"],
            }
            doc_leg["main-text"].append(pitem)

        elif "text" in obj:
            text = obj["text"][span_i:span_j]

            type_label = layout_label_to_ds_type[pelem["type"]]
            name_label = reverse_label_mapping[pelem["name"]]
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
                "name": reverse_label_mapping[pelem["name"]],
                "type": layout_label_to_ds_type[pelem["type"]],
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
