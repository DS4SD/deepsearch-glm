#!/usr/bin/env python
"""Module to create a Tokenizer model (train)"""

import argparse
import json
import os
import random

import tqdm
from tabulate import tabulate

from deepsearch_glm.nlp_utils import train_tok


def parse_arguments():
    """Function to parse arguments for `nlp_train_tok`"""

    parser = argparse.ArgumentParser(
        prog="nlp_train_tok",
        description="train Tokenizer (SentencePiece) model",
        epilog="""
examples of execution: 

1. end-to-end example to train CRF:

    poetry run python ./deepsearch_glm/nlp_train_tok.py -t unigram -n <name> --input-file <filename>
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
        help="name of the output-model",
    )

    parser.add_argument(
        "-i",
        "--input-file",
        required=True,
        type=str,
        default=None,
        help="input-file with text (1 sentence per line)",
    )

    args = parser.parse_args()

    return args.model_type, args.model_name, args.input_file


def create_tok_model(model_type: str, model_name: str, ifile: str):
    """Create a new tokenizer from a text-file"""

    return train_tok(model_type, model_name, ifile)


if __name__ == "__main__":
    mtype, mname, ifile = parse_arguments()

    create_tok_model(mtype, mname, ifile)
