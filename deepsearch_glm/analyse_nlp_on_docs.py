#!/usr/bin/env python

import os
import json
import glob

import argparse
import textwrap

import pandas as pd

#from tabulate import tabulate
#from ds_utils import convert_pdffiles

#import andromeda_nlp

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'analyse_nlp_on_doc',
        description = 'Analyse NLP on `Deep Search` documents ()',
        epilog =
"""
examples of execution:

1.a run on single document (pdf or json) with default model (=`langauge`):
    poetry run python ./deepsearch_glm/apply_nlp_on_docs.py --json ./data/documents/articles/2305.02334.nlp.json
""",
        formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--json', required=True,
                        type=str, default=None,
                        help="filename(s) of json document")

    args = parser.parse_args()
    
    json_files=sorted(glob.glob(args.json))

    return json_files

"""
def resolve_item(doc, parts):

    if parts[0]=="#"
"""

def extract_meta(doc):
    return

def extract_text(doc):

    page_items = doc["page-items"]
    texts = doc["texts"]
    
    wrapper = wrapper = textwrap.TextWrapper(width=70)
    
    for item in doc["texts"]:
        """
        print("type: ", item.keys())
        print("text: ", "\n".join(wrapper.wrap(text=item["text"])))
        print("")
        """

        for line in wrapper.wrap(text=item["text"]):
            print(f"\t{line}")

        print("")
    return

def extract_sentences(doc):

    """
    for row in doc["instances"]:
        print(len(row), "\t", row)
    """

    df = pd.DataFrame(doc["instances"]["data"],
                      columns=doc["instances"]["headers"])
    #print(df)

    entity_types = df["type"].value_counts()
    print(entity_types)
    
    sents = df[df["type"]=="sentence"]
    print(sents)
    
    return

def extract_tables(doc):

    wrapper = wrapper = textwrap.TextWrapper(width=70)
    
    for i,item in enumerate(doc["tables"]):

        print(f"#/tables/{i}: ", len(item["captions"]))
        if len(item["captions"])>0:
            for line in wrapper.wrap(text=item["captions"][0]["text"]):
                print(f"\t{line}")
    
    return

def extract_figures(doc):

    wrapper = wrapper = textwrap.TextWrapper(width=70)
    
    for i,item in enumerate(doc["figures"]):

        print(f"#/figures/{i}", len(item["captions"]))
        if len(item["captions"])>0:
            for line in wrapper.wrap(text=item["captions"][0]["text"]):
                print(f"\t{line}")

                #print(item["captions"][0]["text"])
    
    return

def extract_references(doc):
    return

if __name__ == '__main__':

    json_files = parse_arguments()
    
    for json_file in json_files:

        print(f" --> reading {json_file}")
        with open(json_file, "r") as fr:
            doc = json.load(fr)

        extract_text(doc)

        extract_sentences(doc)
        
        extract_tables(doc)
        
        extract_figures(doc)
