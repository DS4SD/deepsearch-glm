#!/usr/bin/env python

import os
import json
import glob

import argparse
import textwrap

import textColor as tc
import pandas as pd

from tabulate import tabulate
from nlp_utils import init_nlp_model

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'nlp_prompt',
        description = 'Apply NLP models via prompt',
        epilog =
"""
examples of execution: 

1. run interactive on pieces of text via prompt:

     poetry run python ./deepsearch_glm/prompt.py

2. run interactive on pieces of text via prompt with specific models:

     poetry run python ./deepsearch_glm/prompt.py --models "term;sentence"
""",
        formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--models', required=False,                        
                        type=str, default="language,reference,abbreviation",
                        help="set NLP models (e.g. `language` or `term;sentence`)")

    args = parser.parse_args()

    return args.models


if __name__ == '__main__':

    wrapper = textwrap.TextWrapper(width=70)
    
    model_names = parse_arguments()

    model = init_nlp_model(model_names)

    while True:

        text = input("text: ")

        if text=="quit":
            break

        result = model.apply_on_text(text)

        #print(result.keys())
        
        if len(result["properties"]["data"])>0:
            print(tc.yellow("properties:\n"))
            print(tabulate(result["properties"]["data"],
                           headers=result["properties"]["headers"]), "\n")

        headers=["type", "subtype", "char_i", "char_j", "original"]

        data=[]
        for row in result["instances"]["data"]:

            _=[]
            for h in headers:
                item = str(row[result["instances"]["headers"].index(h)])
                _.append("\n".join(wrapper.wrap(item)))
                
            data.append(_)

        if len(data)>0:
            print(tc.yellow("instances:\n"))            
            print(tabulate(data, headers=headers), "\n")

        if len(result["relations"]["data"])>0:
            print(tc.yellow("relations:\n"))
            print(tabulate(result["relations"]["data"],
                           headers=result["relations"]["headers"]), "\n")        
        

