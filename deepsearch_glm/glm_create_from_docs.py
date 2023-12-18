#!/usr/bin/env python
"""Module to create GLM from PDF/JSON documets"""

import argparse

# import datetime
import glob

# import json
# import os
import sys

from deepsearch_glm.glm_utils import create_glm_dir, create_glm_from_docs
from deepsearch_glm.utils.ds_utils import convert_pdffiles

# import textwrap

# from tabulate import tabulate


def parse_arguments():
    """Function to parse arguments for `create_glm_from_docs`"""

    parser = argparse.ArgumentParser(
        prog="create_glm_from_docs",
        description="Create GLM from Deep Search documents",
        epilog="""
examples of execution: 

1.a run on single document (pdf or json) with default NLP models (=`term`):
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --pdf-docs './data/documents/articles/2305.02334.pdf'
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --json-docs './data/documents/articles/2305.02334.json'

2. run on multiple documents:
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --pdf-docs './data/documents/articles/*.pdf'
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --json-docs './data/documents/articles/*.json'

3. run on multiple documents with non-default models:
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --pdf-docs './data/documents/articles/2305.*.pdf' --models 'language;term;abbreviation'

""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "--pdf-docs",
        required=False,
        type=str,
        default=None,
        help="filename(s) of pdf document",
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
        "--force-convert",
        required=False,
        type=bool,
        default=False,
        help="force pdf conversion",
    )

    parser.add_argument(
        "--output-dir",
        required=False,
        type=str,
        default=create_glm_dir(),
        help="output root directory for GLM",
    )

    args = parser.parse_args()

    pdf_docs = args.pdf_docs
    json_docs = args.json_docs

    if pdf_docs is None and json_docs is None:
        sys.exit(-1)

    pdf_files = []
    if pdf_docs is not None:
        pdf_files = sorted(glob.glob(pdf_docs))

    json_files = []
    if json_docs is not None:
        json_files = sorted(glob.glob(json_docs))

    if len(pdf_files) == 0 and len(json_files) == 0:
        sys.exit(-1)

    return pdf_files, json_files, args.models, args.force_convert, args.output_dir


if __name__ == "__main__":
    pdf_files, json_files, model_names, force_convert, odir = parse_arguments()

    if len(pdf_files) > 0:
        new_json_files = convert_pdffiles(pdf_files, force=force_convert)

        for _ in new_json_files:
            json_files.append(_)

    json_files = sorted(list(set(json_files)))

    glm = create_glm_from_docs(odir, json_files, model_names)
    print(f" --> GLM saved to: {odir}")
