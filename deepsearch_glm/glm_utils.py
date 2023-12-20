#!/usr/bin/env python
"""Module providing utility functions for GLM"""

# import argparse
import datetime

# import glob
# import json
import os
import textwrap
from typing import List

from tabulate import tabulate

from deepsearch_glm.andromeda_glm import glm_model, glm_query
from deepsearch_glm.utils.ds_utils import get_scratch_dir

# import andromeda_nlp
# import andromeda_glm


def create_glm_dir():
    """Function creating directory to write GLM-files."""

    tdir = get_scratch_dir()

    now = datetime.datetime.now()
    glmdir = now.strftime("GLM-model-%Y-%m-%d_%H-%M-%S")

    odir = os.path.join(tdir, glmdir)
    return odir


def load_glm_config(idir: str):
    """Function to create config for loading GLM."""

    config = {
        "IO": {
            "load": {"root": idir},
        },
    }

    return config


def load_glm(idir: str):
    """Function to load GLM"""

    config = load_glm_config(idir)

    glm = glm_model()
    glm.load(config)

    return glm


def create_glm_config_from_docs(
    odir: str, json_files: List[str], nlp_models: str = "conn;verb;term;abbreviation"
):
    """Function to create GLM."""

    config = {
        "IO": {
            "load": {"root": odir},
            "save": {
                "root": odir,
                "write-CSV": False,
                "write-JSON": False,
                "write-path-text": False,
            },
        },
        "create": {
            "enforce-max-size": False,
            "model": {"max-edges": 1e8, "max-nodes": 1e7},
            "number-of-threads": 4,
            "worker": {
                "local-reading-break": True,
                "local-reading-range": [256, 2560],
                "max-edges": 1e7,
                "max-nodes": 1e6,
            },
            "write-nlp-output": False,
        },
        "mode": "create",
        "parameters": {
            "glm-padding": 1,
            "glm-paths": {
                "keep-concatenation": True,
                "keep-connectors": True,
                "keep-terms": True,
                "keep-verbs": True,
                "keep-sentences": True,
                "keep-tables": True,
                "keep-texts": True,
                "keep-docs": True,
            },
            "nlp-models": nlp_models,
        },
        "producers": [
            {
                "input-format": "json",
                "input-paths": json_files,
                "keep-figures": True,
                "keep-tables": True,
                "keep-text": True,
                "order-text": True,
                "output": False,
                "output-format": "nlp.json",
                "subject-type": "DOCUMENT",
            }
        ],
    }

    return config


def create_glm_from_docs(
    odir: str, json_files: List[str], nlp_models: str = "conn;verb;term;abbreviation"
):
    """Function to create GLM from documents."""

    config = create_glm_config_from_docs(odir, json_files, nlp_models)

    glm = glm_model()
    glm.create(config)

    return odir, glm


def show_query_result(res, max_nodes=16):
    """Function to show the result of the query"""

    wrapper = textwrap.TextWrapper(width=50)

    print(
        "overview: \n",
        tabulate(res["overview"]["data"], headers=res["overview"]["headers"]),
        "\n",
    )

    for i, item in enumerate(res["result"]):
        headers = item["nodes"]["headers"]
        data = item["nodes"]["data"]

        for j, row in enumerate(data):
            text = row[headers.index("text")]
            print("text: ", text)

            data[j][headers.index("text")] = "\n".join(wrapper.wrap(text))

        print(f"operation {i}: \n", tabulate(data[0:max_nodes], headers=headers), "\n")


def expand_terms(terms, glm):
    """Function to expand the terms"""

    for term in terms:
        print(term)

        # qry = andromeda_glm.glm_query()
        qry = glm_query()

        qry.select({"nodes": [[term]]})
        qry.filter_by({"mode": "node-flavor", "node-flavors": ["token"]})
        # qry.filter_by({"mode": "node-flavor", "node-flavors":["term"]})

        # flid = qry.get_last_flid()
        qry.traverse({"name": "roots", "edge": "to-root"})
        qry.traverse({"name": "tax-up", "edge": "tax-up"})
        # qry.traverse({"edge":"from-root"})
        # qry.traverse({"edge":"from-token"})
        # qry.filter_by({"mode": "node-flavor", "node-flavors":["term"]})

        # qry.filter_by({"mode": "contains", "contains-flid":flid})
        # qry.traverse({"edge":"to-sent"})

        qry.filter_by({"mode": "node-regex", "node-regex": [f".*{term}.*"]})

        config = qry.to_config()
        # print("query: ", json.dumps(config, indent=2))

        res = glm.query(config)
        if "status" in res and res["status"] == "success":
            show_query_result(res)
        else:
            print(res)
            # print(res["status"], ": ", res["message"])
