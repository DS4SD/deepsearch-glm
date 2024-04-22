#!/usr/bin/env python
"""Module to train name-versus-expr classifier"""

import argparse
import json
import random
import re

import pandas as pd
from tabulate import tabulate
from tqdm import tqdm

from deepsearch_glm.glm_utils import read_edges_in_dataframe, read_nodes_in_dataframe
from deepsearch_glm.nlp_utils import (
    eval_fst,
    init_nlp_model,
    prepare_data_for_fst_training,
    train_fst,
)


def parse_arguments():
    """Function to parse arguments for `nlp_train_semantic`"""

    parser = argparse.ArgumentParser(
        prog="name_classifier", description="script to train name-classifier"
    )

    parser.add_argument(
        "-m",
        "--mode",
        required=True,
        type=str,
        choices=["extract", "prepare", "train", "evaluate", "apply", "test", "all"],
        help="mode",
    )
    args = parser.parse_args()

    return args.mode


def extract(mode, nodes_file, edges_file):
    """Function to extract training data from GLM data"""

    nodes = read_nodes_in_dataframe(nodes_file)  # pd.read_csv(nodes_file)
    edges = read_edges_in_dataframe(edges_file)  # pd.read_csv(edges_file)

    print("nodes: ", nodes_file)
    print(nodes)

    print("edges: ", edges_file)
    print(edges)

    hist = nodes["name"].value_counts()
    # print(hist)

    hist = nodes[nodes["name"] == "label"]["text"].value_counts()
    # print(hist)

    for subtype in [
        "expr",
        "person-name",
        "person-name-v2",
        "person-group",
        "vau",
        "specialised-name",
    ]:
        print(f"label {subtype}")

        label_node = nodes[(nodes["name"] == "label") & (nodes["text"] == subtype)]
        # print(label_node)

        from_label = edges[
            (edges["name"] == "from-label") & (edges["hash_i"].isin(label_node["hash"]))
        ]
        # print(from_label)

        exprs = nodes[
            (nodes["name"] == "inst") & (nodes["hash"].isin(from_label["hash_j"]))
        ]
        # print(exprs)

        exprs.to_csv(f"data_names/data_{mode}_{subtype}.csv", index=False)


def prepare(max_items=None):
    """Prepare and normalise data to train classifier"""

    lines = []

    words = [
        "univers",
        "institu",
        "group",
        "depart",
        "research",
        "labora",
        "centre",
        "center",
        "authority",
        "software",
        "collabo",
        "synchrotron",
        "state",
        "observator",
        "telescope",
        "accelerator",
        "division",
        "association",
        "alliance",
        "club",
        "college",
        "association",
        "hospital",
        "clinic",
        "faculty",
        "museum",
        "corporation",
        "telescope",
    ]

    if True:
        fname = "./data_names/data_authors_person-name.csv"
        df = pd.read_csv(fname)

        for i, row in df.iterrows():
            text = row["text"]
            text_ = text.lower()

            label = "person-name"

            text_ = text.lower()
            for _ in words:
                if _ in text_:
                    label = "expr"

            val = random.random()
            lines.append({"label": label, "training-sample": (val < 0.9), "text": text})

    if True:
        fname = "./data_names/data_authors_specialised-name.csv"
        df = pd.read_csv(fname)

        for i, row in df.iterrows():
            text = row["text"]
            text_ = text.lower()

            label = "expr"

            val = random.random()
            lines.append({"label": label, "training-sample": (val < 0.9), "text": text})

    if True:
        fname = "./data_names/data_abstracts_expr.csv"
        df = pd.read_csv(fname)

        for i, row in df.iterrows():
            text = row["text"]
            text_ = text.lower()

            label = "expr"

            val = random.random()
            lines.append({"label": label, "training-sample": (val < 0.9), "text": text})

    if True:
        fname = "./data_names/data_abstracts_person-name.csv"
        df = pd.read_csv(fname)

        for i, row in df.iterrows():
            text = row["text"]
            text_ = text.lower()

            label = "expr"
            if re.match("^[A-Z]\.\s[A-Z].+", text):
                label = "person-name"

            text_ = text.lower()
            for _ in words:
                if _ in text_:
                    label = "expr"

            val = random.random()
            lines.append(
                {"label": label, "training-sample": bool(val < 0.9), "text": text}
            )

    if True:
        lines.append({"text": "IBM Research", "label": "expr", "training-sample": True})

    print(f"#-lines: {len(lines)}")

    random.shuffle(lines)

    fname = "./data_names/fst_data-v4.jsonl"

    fdata = open(fname, "w")
    for line in lines:
        cand = False
        for _ in ["math", "phys", "mater", "combi", "theor", "science", "comput"]:
            if _ in line["text"].lower():
                cand = True

        if re.match("J\.?\s[A-Z].*", line["text"]) and cand:
            print(line)
            line["label"] = "person-name"

    fdata = open(fname, "w")
    for line in lines:
        fdata.write(json.dumps(line) + "\n")
    fdata.close()

    print(f"finished writing {fname}")

    prepare_data_for_fst_training(data_file=fname, loglevel="INFO")

    print(f"prepared {fname}")


def train():
    """Train FST classifier"""

    train_fst(
        data_file="./data_names/fst_data-v4.jsonl",
        model_file="./data_names/model_names-v4.bin",
        metrics_file="./data_names/model_names.metrics.txt",
        ngram=0,
        duration=3600,
        modelsize="1M",
        loglevel="INFO",
    )


def evaluate():
    """Evaluate FST classifier"""

    eval_fst(
        data_file="./data_names/fst_data-v4.jsonl",
        model_file="./data_names/model_names-v4.bin",
        metrics_file="./data_names/model_names.metrics.txt",
        loglevel="INFO",
    )


def apply_model():
    """Apply FST model"""

    model = init_nlp_model("custom_fst(name:./data_names/model_names-v4.bin)")

    nodes = pd.read_csv("./data_names/data_abstracts_person-name.csv")
    # nodes = pd.read_csv("./data_names/data_authors_person-name.csv")

    for i, row in nodes.iterrows():
        text = row["text"]

        try:
            res = model.apply_on_text(text)
            # print(res)

            props = pd.DataFrame(
                res["properties"]["data"], columns=res["properties"]["headers"]
            )

            label = props.loc[0, "label"]
            conf = int(100 * props.loc[0, "confidence"])

            # if label!="expr" or conf<=90:
            if "." in text:
                print(f"{i}\t{conf}\t{label}\t{text}")
            # print(props)

            # if i>10000:
            #    break
        except:
            print(f"SKIPPING: {text}")
            continue


def test():
    """Test FST model"""

    model = init_nlp_model("custom_fst(name:./data_names/model_names-v4.bin)")

    nodes = pd.read_csv("./data_names/data_abstracts_person-name.csv")

    while True:
        text = input("text: ")

        res = model.apply_on_text(text)

        props = pd.DataFrame(
            res["properties"]["data"], columns=res["properties"]["headers"]
        )

        print("\n", props, "\n")


if __name__ == "__main__":
    mode = parse_arguments()

    if mode == "extract" or mode == "all":
        extract(
            mode="abstracts",
            nodes_file="/Users/taa/Documents/projects/deepsearch-glm/build/glm_arxiv_v2_names_abstracts/nodes.csv",
            edges_file="/Users/taa/Documents/projects/deepsearch-glm/build/glm_arxiv_v2_names_abstracts/edges.csv",
        )

        extract(
            mode="authors",
            nodes_file="/Users/taa/Documents/projects/deepsearch-glm/build/glm_arxiv_v2_names_authors/nodes.csv",
            edges_file="/Users/taa/Documents/projects/deepsearch-glm/build/glm_arxiv_v2_names_authors/edges.csv",
        )

    if mode == "prepare" or mode == "all":
        prepare()

    if mode == "train" or mode == "all":
        train()

    if mode == "evaluate" or mode == "all":
        evaluate()

    if mode == "apply":
        apply_model()

    if mode == "test":
        test()
