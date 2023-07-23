#!/usr/bin/env python

import os

import json
import glob

import argparse
import textwrap
import datetime

from tabulate import tabulate

from deepsearch_glm.glm_utils import load_glm

import andromeda_nlp
import andromeda_glm

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'explore_glm',
        description = 'Explore GLM from Deep Search documents',
        epilog =
"""
examples of execution: 

1 Explore the GLM
    poetry run python ./deepsearch_glm/explore_glm.py --glm <glm-root-dir>

""",
        formatter_class=argparse.RawTextHelpFormatter)
        
    parser.add_argument('--glm', required=True,
                        type=str,  help="root directory of GLM")
    
    args = parser.parse_args()

    if not os.path.exists(args.glm):
        exit(-1)
        
    return args.glm

def show_query_result(res, max_nodes=16):

    wrapper = textwrap.TextWrapper(width=50)
    
    print("overview: \n", tabulate(res["overview"]["data"],
                                   headers=res["overview"]["headers"]), "\n")

    for i,item in enumerate(res["result"]):
        
        headers = item["nodes"]["headers"]
        data = item["nodes"]["data"]

        for j,row in enumerate(data):
            text = row[headers.index("text")]
            #print("text: ", text)
            
            data[j][headers.index("text")] = "\n".join(wrapper.wrap(text))
                    
        print(f"operation {i}: \n", tabulate(data[0:max_nodes], headers=headers), "\n")

def expand_terms(terms, glm_model):

    for term in terms:
        print(term)
        
        qry = andromeda_glm.glm_query()
        qry.select({"nodes":[[term]]})
        qry.filter_by({"mode": "node-flavor", "node-flavors":["token"]})
        #qry.filter_by({"mode": "node-flavor", "node-flavors":["term"]})

        #flid = qry.get_last_flid()
        qry.traverse({"name": "roots", "edge":"to-root"})
        qry.traverse({"name": "tax-up", "edge":"tax-up"})
        #qry.traverse({"edge":"from-root"})
        #qry.traverse({"edge":"from-token"})
        #qry.filter_by({"mode": "node-flavor", "node-flavors":["term"]})

        #qry.filter_by({"mode": "contains", "contains-flid":flid})        
        #qry.traverse({"edge":"to-sent"})

        qry.filter_by({"mode": "node-regex", "node-regex":[f".*{term}.*"]})
        
        config = qry.to_config()    
        #print("query: ", json.dumps(config, indent=2))    
        
        res = glm_model.query(config)
        if "status" in res and res["status"]=="success":
            show_query_result(res)
        else:
            print(res)
            #print(res["status"], ": ", res["message"])

def explore(glm_model):
    
    while True:
        
        prompt = input("question: ")
        #prompt = "What is the income of IBM in 2022?"
        #prompt = "net-zero"
        if(prompt=="q"):
            break
        
        #terms = analyse_prompt(prompt, nlp_model)
        terms = prompt.split(" ")
        
        expand_terms(terms, glm_model)            

if __name__ == '__main__':

    idir = parse_arguments()

    glm = load_glm(idir)
    
    explore(glm)
