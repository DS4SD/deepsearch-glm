#!/usr/bin/env python

import os

import json
import glob

import argparse
import textwrap

from tabulate import tabulate

from ds_convert import convert_pdffile

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

def analyse_prompt(prompt, nlp_model):

    res = nlp_model.apply_on_text(prompt)

    print(res.keys())
    print(tabulate(res["word-tokens"]["data"], headers=res["word-tokens"]["headers"]))
    print(tabulate(res["entities"]["data"], headers=res["entities"]["headers"]))

    type_ind = res["entities"]["headers"].index("type")
    name_ind = res["entities"]["headers"].index("name")
    
    terms=[]
    for i,row in enumerate(res["entities"]["data"]):
        if(row[type_ind]=="term"):
            terms.append([row[name_ind]])

    return terms

def show_query_result(res, max_nodes=16):

    wrapper = textwrap.TextWrapper(width=50)
    
    print("overview: \n", tabulate(res["overview"]["data"],
                                   headers=res["overview"]["headers"]), "\n")

    for i,item in enumerate(res["result"]):
        
        headers = item["nodes"]["headers"]
        data = item["nodes"]["data"]

        for j,row in enumerate(data):
            text = row[headers.index("text")]
            print("text: ", text)
            
            data[j][headers.index("text")] = "\n".join(wrapper.wrap(text))
                    
        print(f"operation {i}: \n", tabulate(data[0:max_nodes], headers=headers), "\n")

def expand_terms(terms):

    for term in terms:
        print(term)
        
        qry = andromeda_glm.glm_query()
        qry.select({"nodes":[[term]]})
        qry.filter_by({"mode": "node-flavor", "node-flavors":["token"]})
        flid = qry.get_last_flid()
        qry.traverse({"edge":"to-root"})
        qry.traverse({"edge":"from-root"})
        qry.filter_by({"mode": "node-flavor", "node-flavors":["term"]})

        qry.filter_by({"mode": "contains", "contains-flid":flid})        
        qry.traverse({"edge":"to-sent"})
        
        config = qry.to_config()    
        #print("query: ", json.dumps(config, indent=2))    
        
        res = glm_model.query(config)
        if "status" in res and res["status"]=="success":
            show_query_result(res)
        else:
            print(res)
            #print(res["status"], ": ", res["message"])
            
def do_qa(nlp_model, glm_model):

    while True:
        
        prompt = input("question: ")
        #prompt = "What is the income of IBM in 2022?"
        #prompt = "net-zero"
        if(prompt=="q"):
            break
        
        #terms = analyse_prompt(prompt, nlp_model)
        terms = prompt.split(" ")
        
        expand_terms(terms)
        
        """
        qry = andromeda_glm.glm_query()
        qry.select({"nodes":search_terms})
        qry.filter_by({"mode": "node-flavor", "node-flavors":["term"]})
        qry.traverse({"edge":"to-table"})
        
        config = qry.to_config()    
        print("query: ", json.dumps(config, indent=2))    

        res = glm_model.query(config)
        show_query_result(res)
        """

        #break
        
if __name__ == '__main__':

    pdffile, force, models = parse_arguments()

    success, jsonfile = convert_pdffile(pdffile, force=force)

    if not success:
        return -1
    
    print(f"json-file: {jsonfile}")
    nlp_model = load_nlp(models)    

    with open(jsonfile, "r") as fr:        
        doc_i = json.load(fr)
        doc_j = nlp_model.apply_on_doc(doc_i)

    #for item in doc_j["main-text"]:
    #    print(item)
    
    #glm_model = load_glm("../build/glm-model-reports")
    #do_qa(nlp_model, glm_model)
