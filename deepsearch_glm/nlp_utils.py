
import os

import json
import glob

import argparse
import textwrap

import datetime
import subprocess

import textColor as tc
import pandas as pd

from tabulate import tabulate

from deepsearch_glm.utils.ds_utils import get_scratch_dir

from deepsearch_glm.andromeda_nlp import nlp_model
#import andromeda_nlp

def create_nlp_dir(tdir=None):

    if tdir==None:
        tdir = get_scratch_dir()

    now = datetime.datetime.now()
    nlpdir = now.strftime("NLP-model-%Y-%m-%d_%H-%M-%S")

    odir = os.path.join(tdir, nlpdir)
    return odir

def list_nlp_model_configs():

    configs = []

    configs += nlp_model.get_apply_configs()
    configs += nlp_model.get_train_configs()

    return configs

def init_nlp_model(model_names:str="language;term", filters:list[str]=[]):

    #model = andromeda_nlp.nlp_model()
    model = nlp_model()

    configs = model.get_apply_configs()
    #print(json.dumps(configs, indent=2))
    
    config = model.get_apply_configs()[0]
    config["models"] = model_names
    config["subject-filters"] = filters
    
    model.initialise(config)
    
    return model

def print_key_on_shell(key, items):

    df = pd.DataFrame(items["data"],
                      columns=items["headers"])
    
    if key in ["instances", "entities"]:

        wrapper = textwrap.TextWrapper(width=70)
        
        df = df[["type", "subtype", "subj_path", "char_i", "char_j", "original"]]

        table=[]
        for i,row in df.iterrows():
            _=[]
            for __ in row:
                if isinstance(__,str): 
                    _.append("\n".join(wrapper.wrap(__)))
                else:
                    _.append(__)

            table.append(_)

        headers = ["type", "subtype", "subj_path", "char_i", "char_j", "original"]
        print(tc.yellow(f"{key}: \n\n"), tabulate(table, headers=headers), "\n")
                         
    else:
        df = pd.DataFrame(items["data"],
                          columns=items["headers"])
        
        print(tc.yellow(f"{key}: \n\n"), df.to_string(), "\n")

def print_on_shell(text, result):

    wrapper = textwrap.TextWrapper(width=70)    
    print(tc.yellow(f"\ntext: \n\n"), "\n".join(wrapper.wrap(text)), "\n")
    
    for _ in ["properties", "word-tokens", "instances",
              "entities", "relations"]:
        if _ in result and len(result[_]["data"])>0:
            print_key_on_shell(_, result[_])
        else:
            print(tc.yellow(f"{_}:"), " null\n\n")
    
def extract_references_from_doc(doc, verbose=False):
    
    texts = pd.DataFrame.from_records(doc["texts"])

    props = pd.DataFrame(doc["properties"]["data"],
                      columns=doc["properties"]["headers"])
    
    insts = pd.DataFrame(doc["instances"]["data"],
                         columns=doc["instances"]["headers"])

    refs = props[props["label"]=="reference"]

    #print("references: \n")
    #print(refs)
    
    wrapper = textwrap.TextWrapper(width=70)

    results=[]
    
    for i,ref in refs.iterrows():

        text = texts[texts["hash"]==ref["subj_hash"]]
        refc = insts[insts["subj_hash"]==ref["subj_hash"]]

        #print(text["text"])
        #print(refc)
        
        results.append(
            {
                "text": text["text"],
                "path": text["sref"],
                "instances": refc.to_records()
            }
        )
        
    return results

