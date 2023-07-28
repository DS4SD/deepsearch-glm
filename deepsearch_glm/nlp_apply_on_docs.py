#!/usr/bin/env python

import os
import json
import glob
import argparse

import pandas as pd

from utils.ds_utils import convert_pdffiles

import andromeda_nlp

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'apply_nlp_on_doc',
        description = 'Apply NLP on `Deep Search` documents',
        epilog =
"""
examples of execution: 

1.a run on single document (pdf or json) with default model (=`langauge`):

     poetry run python ./deepsearch_glm/apply_nlp_on_docs.py --pdf './data/documents/articles/2305.02334.pdf'
     poetry run python ./deepsearch_glm/apply_nlp_on_docs.py --json './data/documents/articles/2305.02334.json'

1.b run on single document pdf document and enforce conversion (ignore cache):

     poetry run python ./deepsearch_glm/apply_nlp_on_docs.py --pdf './data/documents/articles/2305.02334.pdf' --force-convert True

2. run on multiple documents:

     poetry run python ./deepsearch_glm/apply_nlp_on_docs.py --pdf './data/documents/articles/*.pdf'
     poetry run python ./deepsearch_glm/apply_nlp_on_docs.py --json './data/documents/articles/*.json'

3. run on multiple documents with non-default models:

     poetry run python ./deepsearch_glm/apply_nlp_on_docs.py --pdf './data/documents/articles/2305.*.pdf' --models 'language;term'

""",
        formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--pdf', required=False,
                        type=str, default=None,
                        help="filename(s) of pdf document")
    
    parser.add_argument('--json', required=False,
                        type=str, default=None,
                        help="filename(s) of json document")

    parser.add_argument('--models', required=False,                        
                        type=str, default="language",
                        help="set NLP models (e.g. `language` or `term;sentence`)")

    parser.add_argument('--force-convert', required=False,
                        type=bool, default=False,
                        help="force pdf conversion")
    
    args = parser.parse_args()

    pdf = args.pdf
    json = args.json

    if pdf==None and json==None:
        exit(-1)
        
    if pdf!=None:
        pdf_files=sorted(glob.glob(pdf))
    else:
        pdf_files=[]
        
    if json!=None:
        json_files=sorted(glob.glob(json))
    else:
        json_files=[]
        
    return pdf_files, json_files, args.models, args.force_convert

def init_nlp_model(models:str):
    
    model = andromeda_nlp.nlp_model()

    config = model.get_apply_configs()[0]
    config["models"] = models
    
    model.initialise(config)

    return model

if __name__ == '__main__':

    pdf_files, json_files, model_names, force_convert = parse_arguments()

    if len(pdf_files)>0:
        new_json_files = convert_pdffiles(pdf_files, force=force_convert)

        for _ in new_json_files:
            json_files.append(_)

    json_files = sorted(list(set(json_files)))        

    model = init_nlp_model(model_names)

    for json_file in json_files:

        print(f"reading {json_file} ... ", end="")
        with open(json_file, "r") as fr:
            doc_i = json.load(fr)

        print(f"applying models ... ", end="")
        doc_j = model.apply_on_doc(doc_i)

        """
        props = pd.DataFrame(doc_j["properties"]["data"],
                             columns=doc_j["properties"]["headers"])
        print("properties: \n\n", props)

        ints = pd.DataFrame(doc_j["instances"]["data"], 
                            columns=doc_j["instances"]["headers"])
        print("instances: \n\n", ints)

        ents = pd.DataFrame(doc_j["entities"]["data"], 
                            columns=doc_j["entities"]["headers"])
        print(ents)
        """
        
        nlp_file = json_file.replace(".json", ".nlp.json")
        print(f"writing  models {nlp_file}")
        
        with open(nlp_file, "w") as fw:
            fw.write(json.dumps(doc_j, indent=2))


    
