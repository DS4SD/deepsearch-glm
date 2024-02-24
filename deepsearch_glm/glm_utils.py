#!/usr/bin/env python
"""Module providing utility functions for GLM"""

# import argparse
import datetime

# import glob
# import json
import os
import textwrap
from typing import List

import pandas as pd
from tabulate import tabulate

from deepsearch_glm.andromeda_glm import glm_model, glm_query
from deepsearch_glm.utils.ds_utils import get_scratch_dir


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


def load_glm(idir: str, loglevel: str = "WARNING"):
    """Function to load GLM"""

    config = load_glm_config(idir)

    glm = glm_model()
    glm.set_loglevel(loglevel)

    glm.load(config)

    return glm


def read_nodes_in_dataframe(node_file: str):
    """Function to read nodes from a GLM into a dataframe"""

    df = None

    if node_file.endswith(".csv") and os.path.exists(node_file):
        df = pd.read_csv(node_file)

    return df


def read_edges_in_dataframe(edge_file: str):
    """Function to read edges from a GLM into a dataframe"""

    df = None

    if edge_file.endswith(".csv") and os.path.exists(edge_file):
        df = pd.read_csv(edge_file)

    return df


def create_glm_from_config(config: dict, loglevel: str = "WARNING"):
    """Function to create config to create GLM"""

    glm = glm_model()
    glm.set_loglevel(loglevel)

    glm.create(config)

    return config["IO"]["save"]["root"], glm


def create_glm_config_from_docs(
    odir: str,
    json_files: List[str],
    nlp_models: str = "conn;verb;term;abbreviation",
    export_csv=True,
):
    """Function to create GLM configuration"""

    config = {
        "IO": {
            "load": {"root": odir},
            "save": {
                "root": odir,
                "write-CSV": export_csv,
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
    odir: str,
    json_files: List[str],
    nlp_models: str = "conn;verb;term;abbreviation",
    export_csv=False,
):
    """Function to create GLM from documents."""

    glm = glm_model()
    glm.set_loglevel("WARNING")

    config = create_glm_config_from_docs(
        odir, json_files, nlp_models, export_csv=export_csv
    )
    glm.create(config)

    return odir, glm


def create_glm_config_from_texts(
    odir: str,
    json_files: List[str],
    nlp_models: str = "conn;verb;term;abbreviation",
    export_csv=True,
):
    """Function to create GLM configuration"""

    config = {
        "IO": {
            "load": {"root": odir},
            "save": {
                "root": odir,
                "write-CSV": export_csv,
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
                "input-format": "jsonl",
                "input-max-documents": 1e9,
                "input-paths": json_files,
                "key": "text",
                "output": False,
                "output-format": "annot.jsonl",
                "output-path": os.path.join(odir, "annots"),
                "start-line": 0,
                "subject-type": "TEXT",
            },
        ],
    }

    return config


def create_glm_from_texts(
    odir: str,
    json_files: List[str],
    nlp_models: str = "conn;verb;term;abbreviation",
    export_csv=True,
):
    """Function to create GLM from texts in JSONL-files."""

    glm = glm_model()
    glm.set_loglevel("WARNING")

    config = create_glm_config_from_texts(odir, json_files, nlp_models, export_csv)
    glm.create(config)

    return odir, glm


def show_query_result(res, max_nodes=16):
    """Function to show the result of the query"""

    if ("status" not in res) or (res["status"] != "success"):
        return

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
            data[j][headers.index("text")] = "\n".join(wrapper.wrap(text))

        print(f"operation {i}: \n", tabulate(data[0:max_nodes], headers=headers), "\n")


def expand_terms(glm: glm_model, term: str):
    """Function to expand the terms"""

    # qry = andromeda_glm.glm_query()
    qry = glm_query()

    if " " not in term:
        qry.select({"nodes": [[term]]})
    else:
        qry.select({"nodes": [term.split(" ")]})

    # qry.filter_by({"mode": "node-flavor", "node-flavors": ["word_token"]})
    qry.filter_by({"mode": "node-flavor", "node-flavors": ["term"]})

    # flid = qry.get_last_flid()
    # qry.traverse({"name": "roots", "edge": "to-root"})
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
    """
    if "status" in res and res["status"] == "success":
        show_query_result(res)
    else:
        print(res)
        # print(res["status"], ": ", res["message"])
    """

    return res
