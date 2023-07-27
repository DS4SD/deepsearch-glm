#!/usr/bin/env python

import os
import re

import time
import json
import glob

import random
import argparse

import textColor as tc

from deepsearch_glm.utils.ds_utils import convert_pdffiles, ds_index_query
from deepsearch_glm.nlp_utils import create_nlp_dir, init_nlp_model, print_on_shell

import andromeda_nlp

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = "nlp_train_semantic",
        description = 'train classifier for semantic text classifier',
        epilog =
"""
examples of execution: 

1. end-to-end example on pdf documents:

    poetry run python ./deepsearch_glm/nlp_train_semantic.py -m all --json '<root-dir-of-json-docs>'

""",
        formatter_class=argparse.RawTextHelpFormatter)
        
    parser.add_argument('-m', '--mode', required=False, default="all",
                        help="parse: [retrieve,prepare,train,eval,all]")

    parser.add_argument('--output-dir', required=False,
                        type=str, default=None,
                        help="output root directory for trained models")
    
    args = parser.parse_args()

    """
    if args.json!=None:        
        json_files=sorted(glob.glob(args.json))    
    else:
        json_files=None
    """
    if args.output_dir==None:
        odir = create_nlp_dir()
    
    elif not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)
        odir = args.output_dir

    else:
        odir = args.output_dir
        
    return args.mode, odir

def retrieve_data_pubmed(sdir):

    tdir = os.path.join(sdir, "pubmed")

    if not os.path.exists(sdir):
        os.mkdir(sdir)
    
    index="pubmed"
    query="*"
        
    odir = ds_index_query(index, query, tdir, sources=["_name", "file-info", "references", "description"],
                          force=True, limit=1000)        
    
    return odir

def retrieve_data(sdir, index):

    tdir = os.path.join(sdir, index)

    if not os.path.exists(sdir):
        os.mkdir(sdir)
    
    query="*"
        
    odir = ds_index_query(index, query, tdir, sources=["_name", "file-info", "description", "main-text"],
                          force=True, limit=1000)        
    
    return odir

def prepare_data(json_files, data_file):

    fw = open(data_file, "w")
    
    for json_file in json_files:

        data=[]
        
        with open(json_file, "r") as fr:
            doc = json.load(fr)

        #_name = doc["_name"]
        if ("description" not in doc) or \
           ("languages" not in doc["description"]) or \
           ("en" not in doc["description"]["languages"]):
            print(f"skipping ...")
            continue
            
        if "file-info" in doc:
            dhash = doc["file-info"]["document-hash"]
        else:
            #print(doc)
            dhash = -1

        """
        if "references" in doc:
            for item in doc["references"]:
                data.append({"label":"reference", "text":item["text"], "document-hash":dhash})

        if "description" in doc:

            desc = doc["description"]

            if "title" in desc:
                #data.append({"label":"title", "text":desc["title"], "document-hash":dhash})
                data.append({"label":"text", "text":desc["title"], "document-hash":dhash})
            
            if "abstract" in desc:
                for item in desc["abstract"]:
                    data.append({"label":"text", "text":item, "document-hash":dhash})

            affiliations=[]
            if "affiliations" in desc:                    
                for item in desc["affiliations"]:
                    affiliations.append(item["name"])
                    #data.append({"label":"affiliation", "text":item["name"], "document-hash":dhash})
                    data.append({"label":"meta-data", "text":item["name"], "document-hash":dhash})

            authors=[]                    
            if "authors" in desc:                                        
                for item in desc["authors"]:
                    authors.append(item["name"])
                    #data.append({"label":"person_name", "text":item["name"], "document-hash":dhash})
                    data.append({"label":"meta-data", "text":item["name"], "document-hash":dhash})

            if len(authors)>1:
                data.append({"label":"meta-data", "text": ", ".join(authors), "document-hash":dhash})
                data.append({"label":"meta-data", "text": "; ".join(authors), "document-hash":dhash})

            if len(affiliations)>1:
                data.append({"label":"meta-data", "text": ", ".join(affiliations), "document-hash":dhash})
                data.append({"label":"meta-data", "text": "; ".join(affiliations), "document-hash":dhash})                
                
            if len(authors)>=1 and len(affiliations)>=1:

                for _ in authors:
                    for __ in affiliations:
                        data.append({"label":"meta-data", "text": " ".join([_, __]), "document-hash":dhash})
        """
        
        if "main-text" in doc:

            N = len(doc["main-text"])

            title_ind=len(doc["main-text"])
            abs_beg=len(doc["main-text"])
            
            ref_beg=len(doc["main-text"])
            ref_end=len(doc["main-text"])
            
            for i,item in enumerate(doc["main-text"]):

                if "text" not in item:
                    continue
                
                label = item["type"].lower()
                text = item["text"].lower().strip()

                if "title" == label:
                    title_ind=i
                
                if ("subtitle" in label) and ("abstract" in text):
                    abs_beg=i

                if ("subtitle" in label) and ("introduction" in text):
                    intro_beg=i                    
                    
                if ("title" in label) and ("reference" in text):
                    ref_beg=i

                if ref_end==N and ref_beg<N and i>ref_beg and ("title" in label) and ("reference" not in text):
                    ref_end=i

            if title_ind==N or abs_beg==N or ref_beg==N:
                print(f"skipping: {dhash}")
                continue
                    
            for i,item in enumerate(doc["main-text"]):

                if "text" not in item:
                    continue

                type_ = item["type"]
                label = item["type"]
                text = item["text"]

                if title_ind<i and i<abs_beg:
                    label = "meta-data"
                elif ref_beg<i and i<ref_end:
                    label = "reference"
                else:
                    label = "text"

                """
                if "title" in label:
                    print(tc.yellow(f"{label}, {type_}: {text[0:48]}"))
                elif "person" in label or "affi" in label:
                    print(tc.green(f"\t{label}, {type_}: {text[0:48]}"))                    
                elif "text" in label:
                    print(f"\t{label}, {type_}: {text[0:48]}")
                elif "caption" in label:
                    print(tc.red(f"\t{label}, {type_}: {text[0:48]}"))
                elif "reference" in label:
                    print(tc.blue(f"\t{label}, {type_}: {text[0:48]}"))
                else:
                    print(tc.red(f"\t{label}, {type_}: {text[0:48]}"))
                """
                
                data.append({"label":label, "text":item["text"], "document-hash":dhash})

        for item in data:
            fw.write(json.dumps(item)+"\n")
        data=[]

    fw.close()
        
def process_data(dfile, afile):

    """
    config = {
        "mode" : "apply",
        "order" : True,
        "models": "numval,link,language"
    }
    """
    
    fr = open(dfile, "r")
    fw = open(afile, "w")
    
    #model = andromeda_nlp.nlp_model()
    #model.initialise(config)

    model = init_nlp_model("numval,link,language")
    
    total=0
    
    while True:

        try:
            line = fr.readline()
            item = json.loads(line)
        except:
            #print(f"could not parse {line}")
            break

        text = item["text"]
        label = item["label"]
        dhash = item["document-hash"]

        if len(text)<8:
            #print(tc.yellow(f"skipping text {label}: {text}"))
            continue
        
        nlpres = model.apply_on_text(item["text"])

        nlpres["document-hash"] = dhash
        
        tind = nlpres["properties"]["headers"].index("type")
        lind = nlpres["properties"]["headers"].index("label")
        cind = nlpres["properties"]["headers"].index("confidence")

        """
        for i,row in enumerate(nlpres["properties"]["data"]):
            if row[tind]=="language" and row[lind]!="en":
                print(f"skipping text of {row[lind]}: {text[0:48]}")
                continue
        """
        
        found=False
        for i,row in enumerate(nlpres["properties"]["data"]):
            if row[tind]=="semantic":
                row[lind] = label
                row[cind] = 1.0
                
                found = True

        if not found:
            nlpres["properties"]["data"].append(["semantic", label, 1.0])
            found = True

        if random.random()<0.9:
            nlpres["training-sample"] = True
        else:
            nlpres["training-sample"] = False

        if found:                    
            fw.write(json.dumps(nlpres)+"\n")
            total += 1
            
    fr.close()
    fw.close()

    print(tc.green(f"found {total} training-items"))
    
# To train a FST model with HPO, one can use
# 
# `./fasttext supervised -input <path-to-train.txt> -output model_name -autotune-validation <<path-to-valid.txt>> -autotune-duration 600 -autotune-modelsize 1M`
#
#  => the parameters can be found via
#
# `./fasttext dump model_cooking.bin args`
#
def train_fst(train_file, model_file, metrics_file):

    model = andromeda_nlp.nlp_model()
                
    configs = model.get_train_configs()    
    print(json.dumps(configs, indent=2))

    for config in configs:
        if config["mode"]=="train" and \
           config["model"]=="semantic":

            config["args"] = {}
            config["hpo"]["autotune"] = True
            config["hpo"]["duration"] = 360
            config["hpo"]["modelsize"] = "1M"

            config["args"]["n-gram"] = 0
            
            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file 
            config["files"]["metrics-file"] = metrics_file

            model.train(config)
    
if __name__ == '__main__':

    mode, root_dir = parse_arguments()

    sdir = os.path.join(root_dir, "documents")

    data_file = os.path.join(root_dir, "nlp-train-semantic.data.jsonl")
    annot_file = os.path.join(root_dir, "nlp-train-semantic.annot.jsonl")

    fst_model_file = os.path.join(root_dir, "fst_semantic")
    fst_metrics_file = os.path.join(root_dir, "fst_semantic.metrics.txt")
    
    if mode=="all" or mode=="retrieve":
        tdir = retrieve_data_pubmed(sdir)
        json_files = sorted(glob.glob(os.path.join(tdir, "*.json")))
                
        tdir = retrieve_data(sdir, "acl")
        json_files += sorted(glob.glob(os.path.join(tdir, "*.json")))

        tdir = retrieve_data(sdir, "arxiv")
        json_files += sorted(glob.glob(os.path.join(tdir, "*.json")))        
        
        print(f"results saved in {sdir}: ", len(json_files))
        
    if mode=="all" or mode=="prepare":

        json_files = sorted(glob.glob(os.path.join(sdir, "pubmed/*.json")))
        json_files += sorted(glob.glob(os.path.join(sdir, "acl/*.json")))
        json_files += sorted(glob.glob(os.path.join(sdir, "arxiv/*.json")))

        print("#-files: ", len(json_files))
        
        prepare_data(json_files, data_file)
        process_data(data_file, annot_file)

    if mode=="all" or mode=="train":

        train_fst(annot_file, fst_model_file, fst_metrics_file)    
