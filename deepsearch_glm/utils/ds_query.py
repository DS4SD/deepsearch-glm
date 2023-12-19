#!/usr/bin/env python
"""Module to query Deep Search"""

import argparse
import sys

import pandas as pd

from deepsearch_glm.utils.ds_utils import (
    create_docs_dir,
    ds_index_query,
    ds_list_indices,
)


def parse_arguments():
    """Parsing arguments for querying Deep Search indices"""

    parser = argparse.ArgumentParser(
        prog="ds_query",
        description="Query and download documents on the Deep Search platform",
        epilog="""
examples of execution: 

1. search for documents:

    poetry run python ./deepsearch_glm/ds_query.py --index esg-reports --query "\\\"net zero\\\""
    poetry run python ./deepsearch_glm/ds_query.py --index patent-uspto --query "\\\"global warming potential\\\" AND \\\"etching\\\""
    poetry run python ./deepsearch_glm/ds_query.py --index arxiv --query "\\\"quantum computing\\\""


""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "--index", required=True, type=str, help="Deep Search document index"
    )

    parser.add_argument("--query", required=True, type=str, help="Query for document")

    parser.add_argument(
        "--output-dir",
        required=False,
        type=bool,
        default=create_docs_dir(),
        help="output directory for documents",
    )

    args = parser.parse_args()

    return args.index, args.query, args.output_dir


if __name__ == "__main__":
    index, query, odir = parse_arguments()

    indices = ds_list_indices()

    index_found = False
    for item in indices:
        if item["Index"] == index:
            index_found = True

    if not index_found:
        print(pd.DataFrame(indices))
        print(f"index `{index}` not found")
        sys.exit(-1)

    else:
        odir = ds_index_query(index, query, odir, force=True)
        print(f"files downloaded: {odir}")
