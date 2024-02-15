#!/usr/bin/env python
"""Module to create a CRF model (prepare data, train & evaluate)"""

import argparse
import json
import os
import random

import tqdm
from tabulate import tabulate

from deepsearch_glm.nlp_utils import (
    create_nlp_dir,
    eval_crf,
    get_max_items,
    init_nlp_model,
    train_crf,
)


def parse_arguments():
    """Function to parse arguments for `nlp_train_crf`"""

    parser = argparse.ArgumentParser(
        prog="nlp_train_crf",
        description="train CRF model",
        epilog="""
examples of execution: 

1. end-to-end example to train CRF:

    poetry run python ./deepsearch_glm/nlp_train_crf.py -m all --input-file <filename> --output-dir <models-directory>'

2. end-to-end example to train CRF with limited samples:

    poetry run python ./deepsearch_glm/nlp_train_crf.py -m all --input-file <filename> --output-dir <models-directory> --max-items 1000'
""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "-m",
        "--mode",
        required=True,
        default="all",
        help="mode for training CRF model",
        choices=["prepare", "train", "evaluate", "all"],
    )

    parser.add_argument(
        "--input-file",
        required=False,
        type=str,
        default=None,
        help="input-file with annotations in jsonl format",
    )

    parser.add_argument(
        "--output-dir",
        required=False,
        type=str,
        default="./reference-models",
        help="output directory for trained models & prepared data",
    )

    parser.add_argument(
        "--max-items", required=False, type=int, default=-1, help="number of references"
    )

    args = parser.parse_args()

    odir = None
    if args.output_dir is None:
        odir = create_nlp_dir()

    elif not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)
        odir = args.output_dir

    else:
        odir = args.output_dir

    return args.mode, args.input_file, odir, args.max_items


def annotate_item(
    atem,
    item,
    start_and_end_in_utf8=True,
    debug=False,
):
    """Function to annotate the tokens of the string-item"""

    atem["word_tokens"]["headers"].append("true-label")

    char_i_ind = atem["word_tokens"]["headers"].index("char_i")
    char_j_ind = atem["word_tokens"]["headers"].index("char_j")

    label = "null"
    for ri, row_i in enumerate(atem["word_tokens"]["data"]):
        atem["word_tokens"]["data"][ri].append(label)

    text = atem["text"]

    # print(item["text"])
    for annot in item["annotation"]:
        lbl = annot["label"].replace(" ", "_").lower().strip()
        utf8_rng = [annot["start"], annot["end"]]

        # print(item["text"][utf8_rng[0] : utf8_rng[1]])

        # beg_ustr = item["text"][0:utf8_rng[0]]
        beg_ustr = text[0 : utf8_rng[0]]
        beg_bstr = beg_ustr.encode("utf-8")

        # end_ustr = item["text"][0:utf8_rng[1]]
        end_ustr = text[0 : utf8_rng[1]]
        end_bstr = end_ustr.encode("utf-8")

        byte_rng = [len(beg_bstr), len(end_bstr)]

        if start_and_end_in_utf8:
            char_i = byte_rng[0]
            char_j = byte_rng[1]
        else:
            char_i = utf8_rng[0]
            char_j = utf8_rng[1]

        for ri, row_i in enumerate(atem["word_tokens"]["data"]):
            if char_i <= row_i[char_i_ind] and row_i[char_j_ind] <= char_j:
                atem["word_tokens"]["data"][ri][-1] = lbl

    if debug:
        print(text)
        print(
            "\n\n",
            tabulate(
                atem["word_tokens"]["data"], headers=atem["word_tokens"]["headers"]
            ),
        )

    # btext = text.encode("utf-8")
    # assert len(text)==len(btext)

    atem["annotated"] = True

    return atem


def prepare_crf(rfile: str, ofile: str, max_items: int, ratio: float = 0.9):
    """Function to prepare the data (strings to token-array) with labels"""

    nlp_model = init_nlp_model("language", filters=["properties", "word_tokens"])

    max_items = get_max_items(rfile, max_items)

    fr = open(rfile, "r", encoding="utf-8")
    fw = open(ofile, "w", encoding="utf-8")

    for i in tqdm.tqdm(range(0, max_items)):
        line = fr.readline().strip()
        if line is None or len(line) == 0:
            break

        item = json.loads(line)
        atem = nlp_model.apply_on_text(item["text"])

        atem = annotate_item(atem, item)
        atem["training-sample"] = random.random() < ratio

        if "annotated" in atem and atem["annotated"]:
            fw.write(json.dumps(atem) + "\n")

    fr.close()
    fw.close()


def create_crf_model(mode: str, ifile: str, odir: str, max_items: int):
    """Function to create CRF model"""

    filename = os.path.basename(ifile)

    afile = os.path.join(odir, filename.replace(".jsonl", ".annot.jsonl"))

    crf_model_file = os.path.join(odir, filename.replace(".jsonl", ".crf_model.bin"))
    crf_metrics_file = crf_model_file + ".metrics.txt"

    if mode in ["prepare", "all"]:
        prepare_crf(ifile, afile, max_items, ratio=0.9)

    if mode in ["train", "all"]:
        train_crf(
            # model_name="reference",
            model_name="custom_crf",
            train_file=afile,
            model_file=crf_model_file,
            metrics_file=crf_metrics_file,
        )

    if mode in ["evaluate", "all"]:
        eval_crf(
            # model_name="reference",
            model_name="custom_crf",
            train_file=afile,
            model_file=crf_model_file,
            metrics_file=crf_metrics_file,
        )

    return afile, crf_model_file, crf_metrics_file


if __name__ == "__main__":
    mode, ifile, odir, max_items = parse_arguments()

    create_crf_model(mode, ifile, odir, max_items)
