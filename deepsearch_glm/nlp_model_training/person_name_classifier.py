#!/usr/bin/env python
"""Module to train name-versus-expr classifier"""

import argparse
import glob
import json
import os
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

    parser.add_argument(
        "-c",
        "--max-count",
        required=True,
        type=int,
        default=-1,
        help="mode",
    )
    args = parser.parse_args()

    return args.mode, int(args.max_count)


def extract(idir: str, ifile: str, max_count: int = -1):
    """Function to extract training data from meta-data of documents"""

    model = init_nlp_model("name")

    fnames = sorted(glob.glob(os.path.join(idir, "*.json")))

    items = []

    for fname in tqdm(fnames, total=len(fnames), ncols=140):
        with open(fname, "r") as fr:
            doc = json.load(fr)

        desc = doc["description"]

        title = desc.get("title", "").strip()

        subjects = desc.get("subjects", [])

        authors = desc.get("authors", [])
        affls = desc.get("affiliations", [])

        # topics = {}

        if False and len(title) > 0:
            res = model.apply_on_text(title)

            insts = pd.DataFrame(
                res["instances"]["data"], columns=res["instances"]["headers"]
            )

            for i, inst in insts.iterrows():
                if inst["type"] == "term":
                    val = random.random()
                    items.append(
                        {
                            "label": "term",
                            "training-sample": (val < 0.9),
                            "text": inst["original"],
                        }
                    )

        for _ in authors:
            name = _["name"]

            label = "person-name"

            val = random.random()
            items.append({"label": label, "training-sample": (val < 0.9), "text": name})

        for _ in affls:
            text = _["name"]

            res = model.apply_on_text(text)

            val = random.random()
            items.append(
                {
                    "label": "term",  # inst["subtype"],
                    "training-sample": (val < 0.9),
                    "text": text,
                }
            )

            if "instances" in res:
                insts = pd.DataFrame(
                    res["instances"]["data"], columns=res["instances"]["headers"]
                )

                for i, inst in insts.iterrows():
                    if inst["type"] == "name":  # and inst["subtype"]!="person-name":
                        val = random.random()
                        items.append(
                            {
                                "label": "term",  # inst["subtype"],
                                "training-sample": (val < 0.9),
                                "text": inst["original"],
                            }
                        )

        if max_count > 0 and len(items) > max_count:
            break

    # print(json.dumps(topics, indent=2))

    print("#-items: ", len(items))

    random.shuffle(items)

    fout = open(ifile, "w")
    for item in tqdm(items, total=len(items), ncols=140):
        fout.write(json.dumps(item) + "\n")
    fout.close()


def prepare(fname):
    """Prepare and normalise data to train classifier"""

    prepare_data_for_fst_training(data_file=fname, loglevel="INFO")


def train(ifile: str):
    """Train FST classifier"""

    train_fst(
        data_file=ifile,
        model_file=f"{ifile}.v2.bin",
        metrics_file=f"{ifile}.v2.metrics.txt",
        ngram=0,
        duration=3600,
        modelsize="10M",
        loglevel="INFO",
    )


def evaluate(ifile: str):
    """Evaluate FST classifier"""

    eval_fst(
        data_file=ifile,
        model_file=f"{ifile}.v2.bin",
        metrics_file=f"{ifile}.v2.metrics.txt",
        loglevel="INFO",
    )


def apply_model(ifile):
    """Apply FST model"""

    model = init_nlp_model(f"custom_fst(name:{ifile}.v2.bin)")

    nodes = pd.read_csv("./data_names/data_abstracts_expr.csv")
    # nodes = pd.read_csv("./data_names/data_abstracts_person-name.csv")
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
            # if "." in text:
            if label != "term":
                print(f"{i}\t{conf}\t{label}\t{text}")
            # print(props)
            # if i>10000:
            #    break
        except:
            print(f"SKIPPING: {text}")
            continue


def test(ifile):
    """Test FST model"""

    model = init_nlp_model(f"custom_fst(name:{ifile}.v2.bin)")

    # nodes = pd.read_csv("./data_names/data_abstracts_person-name.csv")

    while True:
        text = input("text: ")

        res = model.apply_on_text(text)

        props = pd.DataFrame(
            res["properties"]["data"], columns=res["properties"]["headers"]
        )

        print("\n", props, "\n")


if __name__ == "__main__":
    idir = "./data_personnames"
    ifile = f"./{idir}/training_data.jsonl"

    mode, max_count = parse_arguments()

    if mode == "extract" or mode == "all":
        extract(idir=f"./{idir}/pubmed_meta/", ifile=ifile, max_count=max_count)

    if mode == "prepare" or mode == "all":
        prepare(ifile)

    if mode == "train" or mode == "all":
        train(ifile)

    if mode == "evaluate" or mode == "all":
        evaluate(ifile)

    if mode == "apply":
        apply_model(ifile)

    if mode == "test":
        test(ifile)
