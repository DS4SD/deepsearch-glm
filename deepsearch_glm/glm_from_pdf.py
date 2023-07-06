#!/usr/bin/env python

import os

import json
import glob

import argparse
import textwrap

from tabulate import tabulate

from ds_utils import get_scratch_dir, convert_pdffile

import andromeda_nlp
import andromeda_glm

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'glm_docqa',
        description = 'Do Q&A on pdf document',
        epilog = 'Text at the bottom of help')

    parser.add_argument('--pdf', required=True,
                        type=str,
                        help="filename of pdf document")

    parser.add_argument('--force', required=False,
                        type=bool, default=False,
                        help="force pdf conversion")

    parser.add_argument('--models', required=False,
                        type=str, default="name;verb;term;abbreviation",
                        help="set NLP models (e.g. `term;sentence`)")

    args = parser.parse_args()

    return args.pdf, args.force, args.models

"""
def load_nlp(models:str="name;conn;verb;term;language;reference;abbreviation"):

    nlp_model = andromeda_nlp.nlp_model()

    config = nlp_model.get_apply_configs()[0]
    config["models"] = models

    nlp_model.initialise(config)

    return nlp_model

def apply_nlp(doc_i):

    doc_j = model.apply_on_doc(doc_i)

def load_glm(path:str):

    config = {
        "IO": {
            "load": {
                "root": path
            }
        }
    }

    glm_model = andromeda_glm.glm_model()
    glm_model.load(config)

    return glm_model
"""

#################################################
#
#
# configs = glm.get_configurations()
# print(json.dumps(configs, indent=2))
#
#################################################

def create_glm_config(jsonfile):
    
    scratch_dir = get_scratch_dir()

    bname = os.path.basename(jsonfile).replace(".json", "")
    
    config = {
        "IO": {
            "load": {
                "root": f"{scratch_dir}/glm-model-from-{bname}"
            },
            "save": {
                "root": f"{scratch_dir}/glm-model-from-{bname}",
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
                    jsonfile
                ],
                "keep-figures": True,
                "keep-tables": True,
                "keep-text": True,

                "order-text": True,

                "output": True,
                "output-format": "nlp.json",

                "subject-type": "DOCUMENT"
            }
        ]
    }

    return config

def create_glm(jsonfile):

    config = create_glm_config(jsonfile)
    
    glm = andromeda_glm.glm_model()
    glm.create(config)

    return glm
    
if __name__ == '__main__':

    pdffile, force, models = parse_arguments()

    success, jsonfile = convert_pdffile(pdffile, force=force)

    glm = create_glm(jsonfile)

    
