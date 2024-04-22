#!/usr/bin/env python
"""Module to train CRF reference model"""

import argparse
import glob
import json
import os
import random
import subprocess
import sys
import time

import pandas as pd
import textColor as tc
import tqdm
from tabulate import tabulate

# from deepsearch_glm.andromeda_nlp import nlp_model
from deepsearch_glm.nlp_utils import (
    create_nlp_dir,
    get_max_items,
    init_nlp_model,
    train_crf,
)
from deepsearch_glm.utils.load_pretrained_models import get_resources_dir


def parse_arguments():
    """Function to parse arguments for nlp_train_reference"""

    parser = argparse.ArgumentParser(
        prog="nlp_train_reference",
        description="Prepare CRF data for CRF-reference parser",
        epilog="""
examples of execution: 

1. end-to-end example on pdf documents:

    poetry run python ./deepsearch_glm/nlp_train_semantic.py -m all --input-dir '<root-dir-of-json-docs> --output-dir <models-directory>'

2. annotate (100) references:

    poetry run python ./deepsearch_glm/nlp_train_semantic.py -m annotate --input-dir '<root-dir-of-json-docs> --output-dir <models-directory> --max-items 100'        
""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "-m",
        "--mode",
        required=True,
        default="all",
        help="mode for training semantic model",
        choices=["extract", "annotate", "train", "all"],
    )

    parser.add_argument(
        "--input-dir",
        required=True,
        type=str,
        default=None,
        help="input directory with documents",
    )

    parser.add_argument(
        "--output-dir",
        required=False,
        type=str,
        default=None,
        help="output directory for trained models",
    )

    parser.add_argument(
        "--max-items", required=False, type=int, default=-1, help="number of references"
    )

    args = parser.parse_args()

    idir = args.input_dir

    if not os.path.exists(idir):
        print(f"input directory {idir} does not exist")
        sys.exit(-1)

    if args.output_dir is None:
        odir = idir  # create_nlp_dir()

    elif not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)
        odir = args.output_dir

    else:
        odir = args.output_dir

    return args.mode, idir, odir, args.max_items


def shorten_text(text):
    """Function to shorten text"""

    ntext = text.replace("\n", "")

    return ntext.strip()


def extract_references(filenames, ofile, max_items: int = -1):
    """Function to identify and extract references"""

    nlp_model = init_nlp_model("semantic")

    fw = open(ofile, "w", encoding="utf-8")

    total = 0
    for filename in tqdm.tqdm(filenames):
        # print(f"reading {filename}")

        try:
            with open(filename, "r", encoding="utf-8") as fr:
                idoc = json.load(fr)
        except Exception as exc:
            print(f"could not read line: {str(exc)}")
            continue

        training_sample = random.random() < 0.9

        odoc = nlp_model.apply_on_doc(idoc)

        props = pd.DataFrame(
            odoc["properties"]["data"], columns=odoc["properties"]["headers"]
        )

        props_refs = props[props["label"] == "reference"]
        # print(props_refs)
        refs_hash = list(props_refs["subj_hash"])

        texts = pd.DataFrame.from_records(odoc["texts"])
        # print(texts)

        refs = pd.merge(props_refs, texts, how="inner", on=["subj_hash"])
        # print(refs[refs["confidence"]>0.95][["confidence", "text"]])

        for i, ref in refs.iterrows():
            if ref["confidence"] > 0.95 and len(ref["text"]) > 32:
                item = {"training-sample": training_sample, "text": ref["text"]}
                fw.write(json.dumps(item) + "\n")

                total += 1

        if max_items != -1 and total >= max_items:
            break

    fw.close()


def parse_with_anystyle_api(anystyle, refs):
    """Function to parse references with the anystyle API"""

    time.sleep(1)

    tmpfile = "tmp.json"

    payload = {"input": []}
    for ref in refs:
        payload["input"].append(ref["text"])

    """
    anystyle_token = "9fEhg+39p0J60Bs+WTTwTMcqqTFAUYoyjLlp8nEys4wnfgACn0IoqravX8Exsx/+2q1p4sU7636DR22xUeneLg=="
    anystyle_session = "9GFKMlFoJwbMV6W1Z37YFsG9nbXLqmGicXVzL4r5mn4SqTLcf0revMMFvAjfxcjqR8YBnj2M0fgTWBW12kK1KMFcOgZvZnwQv5lZZ3PQgPP9sait9WgoDR72BHqRpbPe0c1B6%2BNFtYE7aqpugLsTupqBuj%2B%2Fef0tbyd84wC61GkVA9Vtz2nSNC90hDliCre%2BZ2gQUc6runu6yt1M4xa0F8kM4Cxt2pN92XB8hRusqGNfsaCsw5JKdU%2FcDFtdh%2BYDSEBz6DjQFfJq81%2FTI%2F4ulku7mlv73vOC7ew%3D--o%2B2gjgNJqgCjYf4V--3mSN%2FKmNt68WTJsxBh9Bww%3D%3D"

    anystyle_agent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36"

    assert anystyle_token == anystyle["token"]
    assert anystyle_session == anystyle["session"]
    assert anystyle_agent == anystyle["agent"]
    """

    token = anystyle["token"]
    session = anystyle["session"]
    agent = anystyle["agent"]

    res = subprocess.call(
        [
            "curl",
            "https://anystyle.io/parse?format=json",
            "-H",
            "authority: anystyle.io",
            "-H",
            "accept: application/json, text/plain, */*",
            "-H",
            "accept-language: en-GB,en-US;q=0.9,en;q=0.8",
            "-H",
            "content-type: application/json;charset=UTF-8",
            "-H",
            f"cookie: _any_style_session={session}",
            "-H",
            "origin: https://anystyle.io",
            "-H",
            "referer: https://anystyle.io/",
            "-H",
            'sec-ch-ua: "Not_A Brand";v="99", "Google Chrome";v="109", "Chromium";v="109"',
            "-H",
            "sec-ch-ua-mobile: ?0",
            "-H",
            'sec-ch-ua-platform: "macOS"',
            "-H",
            "sec-fetch-dest: empty",
            "-H",
            "sec-fetch-mode: cors",
            "-H",
            "sec-fetch-site: same-origin",
            "-H",
            f"user-agent: {agent}",
            "-H",
            f"x-csrf-token: {token}",
            "--data-raw",
            json.dumps(payload),
            "--compressed",
            "-o",
            tmpfile,
            "-s",
        ]
    )

    try:
        with open(tmpfile, "r", encoding="utf-8") as fr:
            tmp = json.load(fr)

        if os.path.exists(tmpfile):
            os.remove(tmpfile)

        return tmp
    except Exception as exc:
        print(tc.red("could not call anystyle API endpoint ..."))
        print(tc.yellow(f" -> error: {str(exc)}"))

    if os.path.exists(tmpfile):
        os.remove(tmpfile)

    return []


def update_references(anystyle, refs, label_map, verbose=False):
    """Function to update references"""

    results = parse_with_anystyle_api(anystyle, refs)

    if len(results) != len(refs):
        return

    for j, item in enumerate(results):
        parts = []
        for row in item:
            parts.append(row[1])

        text = " ".join(parts)
        if text != refs[j]["text"]:
            print("WARNING: mismatch text")
            continue

        beg = 0
        for k, row in enumerate(item):
            charlen = len(row[1].encode("utf-8"))

            item[k].append(beg)
            item[k].append(beg + charlen)

            beg += charlen
            beg += 1

        refs[j]["word_tokens"]["headers"].append("true-label")

        for ri, row_i in enumerate(refs[j]["word_tokens"]["data"]):
            label = "__undef__"
            for rj, row_j in enumerate(item):
                if row_j[2] <= row_i[0] and row_i[1] <= row_j[3]:
                    label = row_j[0]
                    break

            if label in label_map:
                label = label_map[label]
            else:
                ##print(label)
                label = "null"

            refs[j]["word_tokens"]["data"][ri].append(label)

        if verbose:
            print(text)
            print(
                "\n\n",
                tabulate(
                    refs[j]["word_tokens"]["data"],
                    headers=refs[j]["word_tokens"]["headers"],
                ),
            )

        refs[j]["annotated"] = True


def annotate_references(rfile, ofile, max_items):
    """Function to annotate references"""

    label_map = {
        "author": "authors",
        "title": "title",
        "container-title": "conference",
        "journal": "journal",
        "date": "date",
        "volume": "volume",
        "pages": "pages",
        "citation-number": "reference-number",
        "note": "note",
        "url": "url",
        "doi": "doi",
        "isbn": "isbn",
        "publisher": "publisher",
    }

    resources_dir = get_resources_dir()
    with open(f"{resources_dir}/data_nlp.json", "r", encoding="utf-8") as fr:
        configs = json.load(fr)
        anystyle = configs["services"]["anystyle"]

    nlp_model = init_nlp_model("semantic", filters=["properties", "word_tokens"])

    max_items = get_max_items(rfile, max_items)

    refs = []

    fr = open(rfile, "r", encoding="utf-8")
    fw = open(ofile, "w", encoding="utf-8")

    for i in tqdm.tqdm(range(0, max_items)):
        line = fr.readline().strip()
        if line is None or len(line) == 0:
            break

        try:
            item = json.loads(line)
            ref = nlp_model.apply_on_text(item["text"])

            ref["training-sample"] = item["training-sample"]

            refs.append(ref)

        except Exception as exc:
            print(f"Could not process (error: {str(exc)}) for line: {line}")
            continue

        if len(refs) >= 16:
            update_references(anystyle, refs, label_map)

            for ref in refs:
                if "annotated" in ref and ref["annotated"]:
                    fw.write(json.dumps(ref) + "\n")

            refs = []

    fr.close()
    fw.close()

    print(f"writing annotation to {ofile}")


def create_reference_model(mode: str, idir: str, odir: str, max_items: int = -1):
    """Function to create reference model"""

    json_files = glob.glob(os.path.join(idir, "*.json"))
    print("#-docs: ", len(json_files))

    sfile = os.path.join(odir, "nlp-references.data.jsonl")
    afile = os.path.join(odir, "nlp-references.annot.jsonl")

    crf_model_file = os.path.join(odir, "crf_reference")
    crf_metrics_file = crf_model_file + ".metrics.txt"

    if mode in ["extract", "all"]:
        extract_references(json_files, sfile, max_items)

    if mode in ["annotate", "all"]:
        annotate_references(sfile, afile, max_items)

    if mode in ["train", "all"]:
        train_crf("reference", afile, crf_model_file, crf_metrics_file)


if __name__ == "__main__":
    mode, idir, odir, max_items = parse_arguments()

    create_reference_model(mode, idir, odir, max_items)
