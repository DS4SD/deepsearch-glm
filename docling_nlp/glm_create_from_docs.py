#!/usr/bin/env python
"""Module to create GLM from JSON documents"""

import argparse

# import datetime
import glob

# import json
# import os
import sys

from docling_nlp.glm_utils import create_glm_dir, create_glm_from_docs

# import textwrap

# from tabulate import tabulate


def parse_arguments():
    """Function to parse arguments for `create_glm_from_docs`"""

    parser = argparse.ArgumentParser(
        prog="create_glm_from_docs",
        description="Create GLM from JSON documents",
        epilog="""
examples of execution: 

1. run on single JSON document with default NLP models (=`term`):
    uv run python ./docling_nlp/create_glm_from_docs.py --json-docs './data/documents/articles/2305.02334.json'

2. run on multiple JSON documents:
    uv run python ./docling_nlp/create_glm_from_docs.py --json-docs './data/documents/articles/*.json'

3. run on multiple documents with non-default models:
    uv run python ./docling_nlp/create_glm_from_docs.py --json-docs './data/documents/articles/2305.*.json' --models 'language;term;abbreviation'

""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "--json-docs",
        required=False,
        type=str,
        default=None,
        help="filename(s) of json document",
    )

    parser.add_argument(
        "--models",
        required=False,
        type=str,
        default="name;verb;term;abbreviation",
        help="set NLP models (e.g. `term;sentence`)",
    )

    parser.add_argument(
        "--output-dir",
        required=False,
        type=str,
        default=None,
        help="output root directory for GLM",
    )

    args = parser.parse_args()

    json_docs = args.json_docs

    if json_docs is None:
        sys.exit(-1)

    json_files = sorted(glob.glob(json_docs))

    if len(json_files) == 0:
        sys.exit(-1)

    output_dir = args.output_dir
    if output_dir is None:
        output_dir = create_glm_dir()

    return json_files, args.models, output_dir


if __name__ == "__main__":
    json_files, model_names, odir = parse_arguments()

    json_files = sorted(list(set(json_files)))

    glm = create_glm_from_docs(odir, json_files, model_names)
    print(f" --> GLM saved to: {odir}")
