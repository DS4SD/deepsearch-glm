#!/usr/bin/env python
"""Module to analyse the output of the GLM on documents"""

import argparse
import copy
import glob
import json

# import numpy as np
import pandas as pd
from PIL import Image, ImageDraw

# import os
# import textwrap


# from tabulate import tabulate

# import andromeda_nlp
# from deepsearch_glm.andromeda_nlp import nlp_model


def parse_arguments():
    """Function to parse arguments for `nlp_analyse_docs`"""

    parser = argparse.ArgumentParser(
        prog="nlp_analyse_docs",
        description="Analyse NLP on `Deep Search` documents ()",
        epilog="""
examples of execution:

1.a run on single document (pdf or json) with default model (=`langauge`):

    poetry run python ./deepsearch_glm/nlp_analyse_docs.py --json ./data/documents/articles/2305.02334.nlp.json

""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "--json",
        required=True,
        type=str,
        default=None,
        help="filename(s) of json document",
    )

    parser.add_argument(
        "--page",
        required=False,
        type=int,
        default=1,
        help="page number)",
    )

    args = parser.parse_args()

    json_files = sorted(glob.glob(args.json))

    return json_files, args.page


def show_page(doc, page_num=1, show_orig=True):
    """Function to display a page from the document (default is the first page)"""

    print(doc.keys())

    page_dims = doc["page-dimensions"]
    df_dims = pd.read_json(json.dumps(page_dims), orient="records")
    print(df_dims)

    page_items = doc["page-elements"]
    print(page_items[0])

    df = pd.read_json(json.dumps(page_items), orient="records")
    # print(df)

    df_page = df[df["page"] == page_num]

    # print(df_dims[df_dims["page"] == page_num])

    ph = int(df_dims[df_dims["page"] == page_num]["height"])
    pw = int(df_dims[df_dims["page"] == page_num]["width"])

    # ph = int(df_dims[df_dims["page"]==page_num]["height"][0])
    # pw = int(df_dims[df_dims["page"]==page_num]["width"][0])

    # print("height: ", ph)
    # print("width: ", pw)

    df_page["bbox"] = df_page["bbox"].apply(
        lambda bbox: [bbox[0], ph - bbox[3], bbox[2], ph - bbox[1]]
    )

    factor = 2 if show_orig else 1

    # Create a new image with a white background
    image = Image.new("RGB", (factor * pw, ph), "white")

    # Create an ImageDraw object to draw on the image
    orig_order = []
    points = []

    draw = ImageDraw.Draw(image)
    for i, row in df_page.iterrows():
        bbox = row["bbox"]
        draw.rectangle(bbox, outline="red", width=2)

        orig_order.append(row["orig-order"])
        points.append((int((bbox[0] + bbox[2]) / 2.0), int((bbox[1] + bbox[3]) / 2.0)))

    draw.line(points, fill="blue", width=2)

    for i, point in enumerate(points):
        draw.text(point, str(i), fill="black")

    if show_orig:
        for i, row in df_page.iterrows():
            bbox = row["bbox"]
            bbox[0] += pw
            bbox[2] += pw
            draw.rectangle(bbox, outline="red", width=2)

        opoints = copy.deepcopy(points)
        for i, j in enumerate(orig_order):
            # print(i, "\t", j, "\t", len(points), "\t", len(opoints))
            x, y = points[i]
            opoints[j] = (x + pw, y)

        draw.line(opoints, fill="orange", width=2)

        for i, point in enumerate(opoints):
            draw.text(point, str(i), fill="black")

    image.show()


if __name__ == "__main__":
    json_files, page = parse_arguments()

    for json_file in json_files:
        print(f" --> reading {json_file}")
        with open(json_file, "r", encoding="utf-8") as fr:
            doc = json.load(fr)

        show_page(doc, page_num=page)
