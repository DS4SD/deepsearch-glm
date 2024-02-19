#!/usr/bin/env python
"""Module to query the GLM"""

import argparse
import json

import pandas as pd
from tabulate import tabulate

from deepsearch_glm.andromeda_glm import glm_query
from deepsearch_glm.glm_utils import load_glm
from deepsearch_glm.nlp_utils import init_nlp_model


def parse_arguments():
    """Function to parse arguments for `glm_docqa`"""

    parser = argparse.ArgumentParser(
        prog="glm_docqa",
        description="Do Q&A on pdf document",
        epilog="Text at the bottom of help",
    )

    parser.add_argument(
        "--glm-dir", required=True, type=str, help="directory of GLM model"
    )

    args = parser.parse_args()

    return args.glm_dir


def execute_query(glm, query):
    """Exexcute query on GLM"""

    config = query.to_config()
    # print("query: ", json.dumps(config, indent=2))

    out = glm.query(config)
    # print(json.dumps(out, indent=2))

    return out


def display_result(out):
    """Display the GLM-query output"""

    if out["status"] == "success":
        """
        docs = pd.DataFrame(
            out["result"][1]["nodes"]["data"],
            columns=out["result"][1]["nodes"]["headers"],
        )
        print(docs)
        """
        data = out["result"][1]["nodes"]["data"]
        headers = out["result"][1]["nodes"]["headers"]
        print(tabulate(data, headers=headers))


def run(glm):
    """interactively query GLM"""

    while True:
        line = input("prompt: ").strip()
        parts = line.split(" ")

        qry = glm_query()
        # qry.select({"nodes": [["This"]]})
        qry.select({"nodes": [parts]})
        qry.traverse({"edge": "tax-up"})

        out = execute_query(glm, qry)

        display_result(out)

        # break


if __name__ == "__main__":
    glm_dir = parse_arguments()

    glm_mdl = load_glm(glm_dir)

    run(glm_mdl)
