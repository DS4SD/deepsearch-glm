#!/usr/bin/env python
"""Module providing utility functions for GLM"""

# import argparse
import datetime

# import glob
# import json
import os

# import textwrap
from typing import List

from deepsearch_glm.andromeda_glm import glm_model
from deepsearch_glm.utils.ds_utils import get_scratch_dir

# from tabulate import tabulate


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
            "nlp-models": "conn;verb;term;abbreviation",
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
