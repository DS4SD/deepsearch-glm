#!/usr/bin/env python

import os

import json
import glob

import argparse
import textwrap

from tabulate import tabulate

#from ds_convert import convert_pdffile

import pandas as pd
import matplotlib.pyplot as plt

import andromeda_nlp
import andromeda_glm

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'glm_docqa',
        description = 'Do Q&A on pdf document',
        epilog = 'Text at the bottom of help')

    parser.add_argument('--glm-dir', required=True,
                        type=str,
                        help="directory of GLM model")

    parser.add_argument('--qa-pairs', required=False, 
                        type=str, default="prompt",
                        help="CSV file with QA pairs or `prompt`")
    
    parser.add_argument('--models', required=False,                        
                        type=str, default="name;verb;term;abbreviation",
                        help="set NLP models (e.g. `term;sentence`)")
    
    args = parser.parse_args()

    return args.glm_dir, args.qa_pairs, args.models
    
def load_nlp(models:str="name;conn;verb;term;language;reference;abbreviation"):

    nlp_model = andromeda_nlp.nlp_model()

    config = nlp_model.get_apply_configs()[0]
    config["models"] = models
    
    nlp_model.initialise(config)    

    return nlp_model

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

def apply_nlp_on_doc(doc_i, nlp_model):

    doc_j = nlp_model.apply_on_doc(doc_i)

def apply_nlp_on_text(text, nlp_model):

    res = nlp_model.apply_on_text(text)    

    return res
    
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

def compute_topk_on_documents(df, nlp_model, glm_model):

    topk = {0:0}
    for ind in range(1, 10):
        topk[ind] = 0
        
    for i,row in df.iterrows():
        print(i, row["question"])

        doc_hash = row["doc_hash"]
        question = row["question"]

        context = row["text"]
        
        qres = apply_nlp_on_text(question, nlp_model)
        cres = apply_nlp_on_text(context, nlp_model)
        
        """
        print(json.dumps(res, indent=2))
        print(tabulate(res["instances"]["data"],
                       headers=res["instances"]["headers"]))
        """

        data = cres["instances"]["data"]
        headers = cres["instances"]["headers"]

        insts=[]
        for j,row in enumerate(data):
            insts.append([row[headers.index("type")],
                          row[headers.index("subtype")],
                          row[headers.index("name")]])

        """
        print(f"context: {context}\n")
            
        print("instances: ")
        print(tabulate(insts, headers=["type", "subtype", "name"]), "\n")
        """
        
        data = qres["instances"]["data"]
        headers = qres["instances"]["headers"]
        
        insts=[]
        for j,row in enumerate(data):
            insts.append([row[headers.index("type")],
                          row[headers.index("subtype")],
                          row[headers.index("name")]])
        """
        print(f"question: {question}\n")
            
        print("instances: ")
        print(tabulate(insts, headers=["type", "subtype", "name"]), "\n")
        """
        
        terms=[]
        for j,row in enumerate(insts):
            if "term"==row[0]:
                term=row[2].split()
                terms.append(term)
                
        qry = andromeda_glm.glm_query()
        qry.select({"nodes":terms})
        qry.traverse({"edge":"to-doc"})

        config = qry.to_config()    
        #print("query: ", json.dumps(config, indent=2))            

        out = glm_model.query(config)
        #print(json.dumps(out, indent=2))
                
        if out["status"]=="success":

            docs = pd.DataFrame(out["result"][1]["nodes"]["data"],
                                columns=out["result"][1]["nodes"]["headers"])

            doc_hashes = list(docs["text"])
            
            for k,v in topk.items():
                if k==0:
                    topk[k] += 1
                elif doc_hash in doc_hashes[0:k]:
                    topk[k] += 1

    print(json.dumps(topk, indent=2))

    x=[]
    y=[]
    for i in range(1,10):
        x.append(i)
        y.append(topk[i]/topk[0])
    
    plt.figure(1)
    plt.plot(x,y, "r.-", label="doc-topk")
    plt.ylim(0,1.05)
    plt.legend(loc="lower right")
    plt.show()

def compute_topk_on_element(df, nlp_model, glm_model):

    topk = {0:0}
    for ind in range(1, 10):
        topk[ind] = 0
        
    for i,row in df.iterrows():
        print(i, row["question"])

        doc_hash = row["doc_hash"]
        question = row["question"]

        context = row["text"]
        
        qres = apply_nlp_on_text(question, nlp_model)
        cres = apply_nlp_on_text(context, nlp_model)    

        data = qres["instances"]["data"]
        headers = qres["instances"]["headers"]
        
        insts=[]
        for j,row in enumerate(data):
            insts.append([row[headers.index("type")],
                          row[headers.index("subtype")],
                          row[headers.index("name")]])
        """
        print(f"question: {question}\n")
            
        print("instances: ")
        print(tabulate(insts, headers=["type", "subtype", "name"]), "\n")
        """
        
        terms=[]
        for j,row in enumerate(insts):
            if "term"==row[0]:
                term=row[2].split()
                terms.append(term)
                
        qry = andromeda_glm.glm_query()
        qry.select({"nodes":terms})
        qry.traverse({"edge":"to-text"})

        config = qry.to_config()    
        #print("query: ", json.dumps(config, indent=2))            

        out = glm_model.query(config)
        #print(json.dumps(out, indent=2))
                
        if out["status"]=="success":

            docs = pd.DataFrame(out["result"][1]["nodes"]["data"],
                                columns=out["result"][1]["nodes"]["headers"])

            doc_hashes = list(docs["text"])
            
            for k,v in topk.items():
                if k==0:
                    topk[k] += 1
                elif doc_hash in doc_hashes[0:k]:
                    topk[k] += 1

    print(json.dumps(topk, indent=2))

    
if __name__ == '__main__':

    glm_dir, qa_pairs_file, models = parse_arguments()

    glm_model = load_glm(glm_dir)    
    nlp_model = load_nlp(models)    

    df = pd.read_csv(qa_pairs_file)

    compute_topk_on_documents(df, nlp_model, glm_model)
    #compute_topk_on_element(df, nlp_model, glm_model)
