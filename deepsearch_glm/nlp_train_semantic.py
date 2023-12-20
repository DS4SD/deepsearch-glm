#!/usr/bin/env python
"""Module to train semantic classifier"""

import argparse
import glob
import json
import os
import random
import sys

import pandas as pd
import textColor as tc
import tqdm
from tabulate import tabulate

from deepsearch_glm.andromeda_nlp import nlp_model
from deepsearch_glm.nlp_utils import create_nlp_dir, init_nlp_model
from deepsearch_glm.utils.ds_utils import ds_index_query

# import re
# import time


# from deepsearch_glm.nlp_utils import create_nlp_dir, init_nlp_model, print_on_shell
# from deepsearch_glm.utils.ds_utils import convert_pdffiles, ds_index_query


def parse_arguments():
    """Function to parse arguments for `nlp_train_semantic`"""

    parser = argparse.ArgumentParser(
        prog="nlp_train_semantic",
        description="train classifier for semantic text classifier",
        epilog="""
examples of execution: 

1. end-to-end example on pdf documents:

    poetry run python ./deepsearch_glm/nlp_train_semantic.py -m all --input-dir '<root-dir-of-json-docs> --output-dir <models-directory>'

""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "-m",
        "--mode",
        required=True,
        default="all",
        help="mode for training semantic model",
        choices=["retrieve", "prepare", "process", "train", "eval", "refine", "all"],
    )

    parser.add_argument(
        "--input-dir",
        required=True,
        type=str,
        # default="./semantic-models/documents",
        help="input directory with documents",
    )

    parser.add_argument(
        "--output-dir",
        required=False,
        type=str,
        default=None,
        help="output directory for trained models",
    )

    args = parser.parse_args()

    idir = args.input_dir

    if not os.path.exists(idir):
        print(f"input directory {idir} does not exist")
        sys.exit(-1)

    if args.output_dir is None:
        odir = idir

    elif not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)
        odir = args.output_dir

    else:
        odir = args.output_dir

    return args.mode, idir, odir


def retrieve_data_pubmed(sdir):
    """Function to retrieve data from pubmed folder"""

    tdir = os.path.join(sdir, "pubmed")

    if not os.path.exists(sdir):
        os.mkdir(sdir)

    index = "pubmed"
    query = "description.publication_date:[2022-01-01 TO 2022-03-01]"

    odir = ds_index_query(
        index,
        query,
        tdir,
        sources=["_name", "file-info", "references", "description"],
        force=True,
        limit=1000,
    )

    return odir


def retrieve_data_arxiv(sdir):
    """Function to retrieve data from arxiv folder"""

    tdir = os.path.join(sdir, "arxiv")

    if not os.path.exists(sdir):
        os.mkdir(sdir)

    index = "arxiv"
    query = "description.publication_date:[2022-01-01 TO 2022-03-01]"

    odir = ds_index_query(
        index,
        query,
        tdir,
        sources=["_name", "file-info", "description", "main-text"],
        force=True,
        limit=50000,
    )

    return odir


def retrieve_data(sdir, index):
    """Function to retrieve data"""

    tdir = os.path.join(sdir, index)

    if not os.path.exists(sdir):
        os.mkdir(sdir)

    query = "*"

    odir = ds_index_query(
        index,
        query,
        tdir,
        sources=["_name", "file-info", "description", "main-text"],
        force=True,
        limit=50000,
    )

    return odir


def prepare_data_from_legacy_documents(doc):
    """Function to prepare data from legacy documents"""

    if "file-info" in doc:
        dhash = doc["file-info"]["document-hash"]
    else:
        dhash = -1

    text_len = len(doc["main-text"])

    title_ind = len(doc["main-text"])

    abs_beg = len(doc["main-text"])
    intro_beg = len(doc["main-text"])

    ref_beg = len(doc["main-text"])
    ref_end = len(doc["main-text"])

    data = []
    for i, item in enumerate(doc["main-text"]):
        if "text" not in item:
            continue

        label = item["type"].lower()
        text = item["text"].lower().strip()

        if "title" == label and title_ind == text_len:
            title_ind = i

        if ("title" in label) and ("abstract" in text) and abs_beg == text_len:
            abs_beg = i

        if (text.startswith("abstract")) and abs_beg == text_len:
            abs_beg = i

        if ("title" in label) and ("introduction" in text) and intro_beg == text_len:
            intro_beg = i

        if ("title" in label) and ("references" in text) and ref_beg == text_len:
            ref_beg = i

        # (("title" in label) or ("caption" in label)) and ("reference" not in text):
        if (
            ref_end == text_len
            and ref_beg < text_len
            and i > ref_beg
            and (("title" in label))
            and ("reference" not in text)
        ):
            ref_end = i

    if title_ind == text_len or abs_beg == text_len or ref_beg == text_len:
        return data

    # print(dhash)
    for i, item in enumerate(doc["main-text"]):
        if "text" not in item:
            continue

        type_ = item["type"]
        label = item["type"]
        text = item["text"]

        skip = (
            ((len(text) <= 1) or (len(text.split(" ")) == 1))
            and ("title" not in label)
            and (len(text) <= 5)
        )
        if skip:
            # print(f"skipping: {text}")
            continue

        if title_ind < i and (i < min(abs_beg, intro_beg)):
            label = "meta-data"
        elif ref_beg < i and i < ref_end:
            label = "reference"
        elif "title" in label:
            label = "header"
        else:
            label = "text"

        if "title" in label:
            print(tc.yellow(f"{label}, {type_}: {text[0:48]}"))
        elif "meta" in label:
            print(tc.green(f"\t{label}, {type_}: {text[0:48]}"))
        elif "text" in label:
            print(f"\t{label}, {type_}: {text[0:48]}")
        elif "reference" in label:
            print(tc.blue(f"\t{label}, {type_}: {text[0:48]}"))
        else:
            print(tc.red(f"\t{label}, {type_}: {text[0:48]}"))

        if random.random() < 0.9:
            training_sample = True
        else:
            training_sample = False

        data.append(
            {
                "document-hash": dhash,
                "label": label,
                "text": item["text"],
                "training-sample": training_sample,
            }
        )

    return data


def prepare_data_from_description(doc):
    """Function to prepare data from description"""

    if "file-info" in doc:
        dhash = doc["file-info"]["document-hash"]
    else:
        dhash = -1

    data = []

    if "references" in doc:
        for item in doc["references"]:
            data.append(
                {"label": "reference", "text": item["text"], "document-hash": dhash}
            )

    if "description" in doc:
        desc = doc["description"]

        if "title" in desc:
            data.append(
                {"label": "text", "text": desc["title"], "document-hash": dhash}
            )

        if "abstract" in desc:
            for item in desc["abstract"]:
                data.append({"label": "text", "text": item, "document-hash": dhash})

        affiliations = []
        if "affiliations" in desc:
            for item in desc["affiliations"]:
                affiliations.append(item["name"])
                data.append(
                    {"label": "meta-data", "text": item["name"], "document-hash": dhash}
                )

        authors = []
        if "authors" in desc:
            for item in desc["authors"]:
                authors.append(item["name"])
                data.append(
                    {"label": "meta-data", "text": item["name"], "document-hash": dhash}
                )

        if len(authors) > 1:
            data.append(
                {
                    "label": "meta-data",
                    "text": ", ".join(authors),
                    "document-hash": dhash,
                }
            )

        if len(affiliations) > 1:
            data.append(
                {
                    "label": "meta-data",
                    "text": ", ".join(affiliations),
                    "document-hash": dhash,
                }
            )

        if len(authors) >= 1 and len(affiliations) >= 1:
            for _ in authors:
                for __ in affiliations:
                    data.append(
                        {
                            "label": "meta-data",
                            "text": " ".join([_, __]),
                            "document-hash": dhash,
                        }
                    )

    return data


def prepare_data(json_files, data_file):
    """Function to prepare data"""

    num_lines = 0

    fw = open(data_file, "w", encoding="utf-8")

    for json_file in tqdm.tqdm(json_files):
        data = []

        try:
            with open(json_file, "r", encoding="utf-8") as fr:
                doc = json.load(fr)
        except:
            continue

        if "main-text" in doc:
            data = prepare_data_from_legacy_documents(doc)
            # continue
        else:
            data = prepare_data_from_description(doc)

        for item in data:
            num_lines += 1
            fw.write(json.dumps(item) + "\n")
        data = []

    fw.close()

    return num_lines


def process_data(data_file):
    """Function to process data"""

    model = init_nlp_model("semantic")

    configs = model.get_train_configs()
    # print(json.dumps(configs, indent=2))

    for config in configs:
        if config["mode"] == "train" and config["model"] == "semantic":
            config["files"]["train-file"] = data_file
            print(json.dumps(config))

            model.prepare_data_for_train(config)


# To train a FST model with HPO, one can use
#
# `./fasttext supervised -input <path-to-train.txt> -output model_name -autotune-validation <<path-to-valid.txt>> -autotune-duration 600 -autotune-modelsize 1M`
#
#  => the parameters can be found via
#
# `./fasttext dump model_cooking.bin args`
#
def train_fst(
    data_file, model_file, metrics_file, autotune=True, duration=360, modelsize="1M"
):
    """Function to train fasttext classifier"""

    model = nlp_model()

    configs = model.get_train_configs()
    # print(json.dumps(configs, indent=2))

    for config in configs:
        if config["mode"] == "train" and config["model"] == "semantic":
            config["args"] = {}
            config["hpo"]["autotune"] = autotune
            config["hpo"]["duration"] = duration
            config["hpo"]["modelsize"] = modelsize

            config["args"]["n-gram"] = 0

            config["files"]["train-file"] = data_file

            config["files"]["model-file"] = model_file
            config["files"]["metrics-file"] = metrics_file

            model.train(config)


def evaluate_model(data_file, model_file, metrics_file):
    """Function to evaluate fasttext classifier"""

    model = nlp_model()

    configs = model.get_train_configs()
    # print(json.dumps(configs, indent=2))

    for config in configs:
        if config["mode"] == "train" and config["model"] == "semantic":
            config["args"] = {}

            config["files"]["train-file"] = data_file

            config["files"]["model-file"] = model_file + ".bin"
            config["files"]["metrics-file"] = metrics_file

            print(config)
            model.evaluate_model(config)


def refine_data(data_file):
    """Function to refine data"""

    model = init_nlp_model("semantic", filters=["properties"])

    print(f"reading {data_file}")

    fr = open(data_file, "r", encoding="utf-8")
    fw = open(data_file.replace(".jsonl", ".v2.jsonl"), "w", encoding="utf-8")

    table = []

    num_lines = 0
    while True:
        try:
            line = fr.readline()
            data = json.loads(line)
        except:
            break

        num_lines += 1

        res = model.apply_on_text(data["text"])

        pred = res["properties"]["data"][-1][-2]
        conf = res["properties"]["data"][-1][-1]

        if "properties" in res:
            row = [
                data["document-hash"],
                data["training-sample"],
                data["label"],
                conf,
                pred,
                data["text"][0:48],
            ]

            table.append(row)
        else:
            print(json.dumps(res, indent=2))

        if conf > 0.9 or data["label"] == pred:
            fw.write(json.dumps(data) + "\n")

        if num_lines > 100000:
            break

    fr.close()

    print(f"num-lines: {num_lines}")
    print(tabulate(table))

    df = pd.DataFrame(
        table, columns=["document", "training", "true", "conf", "pred", "text"]
    )
    df.sort_values(["pred", "training", "conf"], ascending=[True, True, False])

    df.to_csv(f"{data_file}.csv")


def train_semantic(mode, idir, odir, autotune=True, duration=360, modelsize="1M"):
    """Function to train semantic fasttext classifier"""

    tdir = os.path.join(odir, "documents")

    data_file = os.path.join(odir, "nlp-train-semantic.data.jsonl")
    # annot_file = os.path.join(odir, "nlp-train-semantic.annot.jsonl")

    fst_model_file = os.path.join(odir, "fst_semantic")
    fst_metrics_file = os.path.join(odir, "fst_semantic.metrics.txt")

    if mode in ["all", "retrieve"]:
        retrieve_data_pubmed(tdir)
        json_files = sorted(glob.glob(os.path.join(tdir, "*.json")))

        retrieve_data_arxiv(tdir)
        json_files += sorted(glob.glob(os.path.join(tdir, "*.json")))

    if mode in ["all", "prepare"]:
        json_files = sorted(glob.glob(os.path.join(idir, "*.json")))
        json_files += sorted(glob.glob(os.path.join(idir, "*/*.json")))

        print("#-files: ", len(json_files))
        if len(json_files) == 0:
            exit(-1)

        # json_files = json_files[0:1000]

        num_lines = prepare_data(json_files, data_file)

    if mode in ["all", "process"]:
        process_data(data_file)  # , annot_file)

    if mode in ["all", "train"]:
        # train_fst(annot_file, fst_model_file, fst_metrics_file, autotune=True, duration=300, modelsize="100M")
        train_fst(
            data_file,
            fst_model_file,
            fst_metrics_file,
            autotune=True,
            duration=3600,
            modelsize="100M",
        )

    if mode in ["all", "eval"]:
        evaluate_model(data_file, fst_model_file, fst_metrics_file)

    if mode in ["all", "refine"]:
        refine_data(data_file)


if __name__ == "__main__":
    mode, idir, odir = parse_arguments()

    train_semantic(mode, idir, odir)
