#!/usr/bin/env python
"""Module to create a CRF model (prepare data, train & evaluate)"""

import argparse
import json
import os
import random

import tqdm
from tabulate import tabulate

from deepsearch_glm.nlp_utils import (
    train_tok
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
        "-t",
        "--type",
        required=True,
        default="unigram",
        help="model-type for training tokenizer model",
        choices=["unigram", "bpe", "word", "char"],
    )

    parser.add_argument(
        "-n",
        "--name",
        required=True,
        type=str,
        default="./trained-tokenizer",
        help="name of the output-model"
    )
    
    parser.add_argument(
        "--input-file",
        required=True,
        type=str,
        default=None,
        help="input-file with text (1 sentence per line)",
    )

    args = parser.parse_args()

    return args.model_type, args.model_name, args.input_file


def create_tok_model(model_type: str, model_name: str, ifile: str):

    train_tok(model_type, model_name, ifile)
    

if __name__ == "__main__":
    mtype, mname, ifile = parse_arguments()

    create_crf_model(mode, ifile, odir, max_items)
