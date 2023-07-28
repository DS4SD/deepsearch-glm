#!/usr/bin/env python

import os
import json
import glob

import argparse
"""
import textwrap
wrapper = textwrap.TextWrapper(width=70)

import pandas as pd
pd.options.display.width=48

import textColor as tc


"""

import andromeda_nlp

from deepsearch_glm.nlp_utils import init_nlp_model, print_on_shell

default_text = """A team of physicists at Université Paris-Saclay has, for the first time, observed spontaneous quasi-crystal self-assembly. The observation occurred during an experiment they were conducting with tiny vibrating magnetic spheres. The team has written a paper describing their experiment and have posted it on the arXiv preprint server while they await peer review."""

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'nlp_apply_on_text',
        description = 'Apply NLP on text',
        epilog =
"""
examples of execution:

1. apply NLP on default text with default models (=`langauge`):

    poetry run python ./deepsearch_glm/apply_nlp_on_text.py

2. apply NLP on a single piece of text with default model (=`langauge`):

    poetry run python ./deepsearch_glm/apply_nlp_on_text.py --text "FeSe is a superconductor with Tc of 30 K."

3. apply NLP on default text with specific models (=`verb;term;conn;semantic;abbreviation`):

    poetry run python ./deepsearch_glm/apply_nlp_on_text.py --model-names "verb;term;conn;semantic;abbreviation"

4. apply NLP on text from prompt with specific models (=`verb;term`):

    poetry run python ./deepsearch_glm/apply_nlp_on_text.py --model-names "verb;term" --interactive True

""",
        formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('-t', '--text', required=False,
                        type=str, default=default_text,
                        help="text on which the NLP is applied.")

    parser.add_argument('-m', '--model-names', required=False,
                        type=str, default="verb;term;conn;semantic;abbreviation",
                        help="model_names to be applied")

    parser.add_argument('--interactive', required=False,
                        type=bool, default=False,
                        help="run on text interactively")
    
    args = parser.parse_args()
    
    return args.text, args.model_names, args.interactive

if __name__ == '__main__':

    text, model_names, interactive = parse_arguments()
    
    model = andromeda_nlp.nlp_model()
    print("resource-dir: ", model.get_resources_path())

    model.initialise_models(model_names)

    if interactive:

        while True:
            text = input("text: ")

            if text in ["q", "quit"]:
                break

            result = model.apply_on_text(text)
            print_on_shell(text, result)

    else:

        result = model.apply_on_text(text)
        print_on_shell(text, result)

