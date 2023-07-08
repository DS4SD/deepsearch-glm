#!/usr/bin/env python

import os

import json
import glob

import argparse
import textwrap

from tabulate import tabulate

from ds_utils import get_scratch_dir, ds_index_query

import andromeda_nlp
import andromeda_glm

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'glm_docqa',
        description = 'Do Q&A on pdf document',
        epilog = 'Text at the bottom of help')

    parser.add_argument('--index', required=True,
                        type=str,
                        help="Deep Search index")

    parser.add_argument('--query', required=True,
                        type=bool, default=False,
                        help="Query for document")

    parser.add_argument('--models', required=False,
                        type=str, default="name;verb;term;abbreviation",
                        help="set NLP models (e.g. `term;sentence`)")
    
    parser.add_argument('--force', required=False,
                        type=bool, default=False,
                        help="force pdf conversion")

    args = parser.parse_args()

    return args.index, args.query, args.models, args.force

#################################################
#
#
# configs = glm.get_configurations()
# print(json.dumps(configs, indent=2))
#
#################################################

def create_glm_config(jsondir):
    
    scratch_dir = get_scratch_dir()
    
    config = {
        "IO": {
            "load": {
                "root": f"{jsondir}-glm-model"
            },
            "save": {
                "root": f"{jsondir}-glm-model",
                "write-CSV": True,
                "write-JSON": True,
                "write-path-text": True
            }
        },
        "create": {
            "enforce-max-size": False,
            "model": {
                "max-edges": 1e8,
                "max-nodes": 1e7
            },
            "number-of-threads": 4,
            "worker": {
                "local-reading-break": True,
                "local-reading-range": [
                    256,
                    2560
                ],
                "max-edges": 1e7,
                "max-nodes": 1e6
            },
            "write-nlp-output": False
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
            "nlp-models": "conn;verb;term;abbreviation"
        },
        "producers": [
            {
                "input-format": "json",
                "input-paths": [
                    jsondir
                ],
                "keep-figures": True,
                "keep-tables": True,
                "keep-text": True,

                "order-text": True,

                "output": False,
                "output-format": "nlp.json",

                "subject-type": "DOCUMENT"
            }
        ]
    }

    return config

def create_glm(jsondir):

    config = create_glm_config(jsondir)
    
    glm = andromeda_glm.glm_model()
    glm.create(config)

    return glm
    
if __name__ == '__main__':

    index, query, models, force = parse_arguments()

    dumpdir = ds_index_query(index, query, force)

    glm = create_glm(dumpdir)
