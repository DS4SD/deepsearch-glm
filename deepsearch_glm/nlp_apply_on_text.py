#!/usr/bin/env python

import os
import json
import glob

import argparse
import textwrap

import pandas as pd

import textColor as tc

import andromeda_nlp

default_text = """A team of physicists at Université Paris-Saclay has, for the first time, observed spontaneous quasi-crystal self-assembly. The observation occurred during an experiment they were conducting with tiny vibrating magnetic spheres. The team has written a paper describing their experiment and have posted it on the arXiv preprint server while they await peer review."""

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'nlp_on_text',
        description = 'Apply NLP on text',
        epilog =
"""
examples of execution:

1.a run on single document (pdf or json) with default model (=`langauge`):

    poetry run python ./deepsearch_glm/nlp_analyse_docs.py --json ./data/documents/articles/2305.02334.nlp.json

""",
        formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('-t', '--text', required=False,
                        type=str, default=default_text,
                        help="text on which the NLP is applied.")

    parser.add_argument('-m', '--model-names', required=False,
                        type=str, default="verb;term;conn;semantic;abbreviation",
                        help="model_names to be applied")

    
    args = parser.parse_args()
    
    return args.text

if __name__ == '__main__':

    text, model_names = parse_arguments():
    
    mdl = andromeda_nlp.nlp_model()
    print("resource-dir: ", mdl.get_resources_path())

    mdl.initialise_models(model_names)

    result = mdl.apply_on_text(text)

    props = pd.DataFrame(result["properties"]["data"],
                         columns=result["properties"]["headers"])
    print(tc.yellow("properties: \n\n"), props, "\n")

    ints = pd.DataFrame(result["instances"]["data"], 
                        columns=result["instances"]["headers"])
    print(tc.yellow("instances: \n\n"), ints, "\n")
    
    """
    ents = pd.DataFrame(result["entities"]["data"], 
    columns=result["entities"]["headers"])
    print(ents)
    """
