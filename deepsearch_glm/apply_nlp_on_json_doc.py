#!/usr/bin/env python

import os
import json
import argparse

import pandas as pd

from tabulate import tabulate

import andromeda_nlp

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'nlp_docs',
        description = 'Apply Andromeda-NLP on `Deep Search` documents',
        epilog = 'Text at the bottom of help')

    parser.add_argument('--json', required=True,
                        type=str,
                        help="filename of json document")

    parser.add_argument('--models', required=False,                        
                        type=str, default="language",
                        help="set NLP models (e.g. `langauge` or `term;sentence`)")
    
    args = parser.parse_args()

    return args.json, args.models

def init_nlp_model(models:str):
    
    model = andromeda_nlp.nlp_model()

    config = model.get_apply_configs()[0]
    config["models"] = models
    
    model.initialise(config)

    return model

if __name__ == '__main__':

    jsonfile, model_names = parse_arguments()

    model = init_nlp_model(model_names)
    
    with open(jsonfile, "r") as fr:
        doc_i = json.load(fr)
    
    doc_j = model.apply_on_doc(doc_i)

    print("document description: ")
    print(json.dumps(doc_j["description"], indent=2))
    
    print("document properties: ")
    print(tabulate(doc_j["properties"]["data"],
                   headers=doc_j["properties"]["headers"]))


    """
    df = pd.DataFrame(doc_j["properties"]["data"], columns=doc_j["properties"]["headers"])
    records = json.loads(df.to_json(orient="records"))
    print(records)
    """
    
    outfile = jsonfile.replace(".json", ".nlp.json")
    with open(outfile, "w") as fw:
        fw.write(json.dumps(doc_j, indent=2))
