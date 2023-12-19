#!/usr/bin/env python
"""Module to Query/Explore Graph Language Models"""

import argparse

# import datetime
# import glob
# import json
import os
import sys

# from deepsearch_glm.andromeda_glm import glm_model, glm_query
# from deepsearch_glm.andromeda_nlp import nlp_model
# from deepsearch_glm.andromeda_glm import glm_query
# from deepsearch_glm.glm_utils import expand_terms, load_glm, show_query_result
from deepsearch_glm.glm_utils import expand_terms, load_glm

# import textwrap

# from tabulate import tabulate


def parse_arguments():
    """Function to parse arguments for glm_explore"""

    parser = argparse.ArgumentParser(
        prog="glm_explore",
        description="Explore GLM from Deep Search documents",
        epilog="""
examples of execution: 

1 Explore the GLM
    poetry run python ./deepsearch_glm/explore_glm.py --glm-dir <glm-root-dir>

""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "--glm-dir", required=True, type=str, help="root directory of GLM"
    )

    args = parser.parse_args()

    if not os.path.exists(args.glm_dir):
        sys.exit(-1)

    return args.glm_idr


def explore(glm):
    """Function to interactively explore GLM"""

    while True:
        prompt = input("question: ")
        # prompt = "What is the income of IBM in 2022?"
        # prompt = "net-zero"
        if prompt == "q":
            break

        # terms = analyse_prompt(prompt, nlp_model)
        terms = prompt.split(" ")

        expand_terms(terms, glm)


if __name__ == "__main__":
    idir = parse_arguments()

    glm = load_glm(idir)

    explore(glm)
