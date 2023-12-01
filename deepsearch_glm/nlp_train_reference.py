#!/usr/bin/env python

import os
import re

import time
import json
import glob
import tqdm
import argparse

import random

import subprocess

import numpy as np

import pandas as pd
import matplotlib.pyplot as plt

import textColor as tc

from tabulate import tabulate
from deepsearch_glm.andromeda_nlp import nlp_model

from deepsearch_glm.utils.ds_utils import convert_pdffiles
from deepsearch_glm.nlp_utils import create_nlp_dir, init_nlp_model

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = "nlp_train_reference",
        description = 'Prepare CRF data for CRF-reference parser',
        epilog =
"""
examples of execution: 

1. end-to-end example on pdf documents:

    poetry run python ./deepsearch_glm/nlp_train_semantic.py -m all --input-dir '<root-dir-of-json-docs> --output-dir <models-directory>'
""",
        formatter_class=argparse.RawTextHelpFormatter)
        
    parser.add_argument('-m', '--mode', required=True, default="all",
                        help="mode for training semantic model",
                        choices=["extract","annotate","train","all"])

    parser.add_argument('--input-dir', required=False,
                        type=str, default=None,
                        help="input directory with documents")
    
    parser.add_argument('--output-dir', required=False,
                        type=str, default="./reference-models",
                        help="output directory for trained models")

    parser.add_argument('--max-items', required=False,
                        type=int, default=-1,
                        help="number of references")
    
    args = parser.parse_args()

    idir = args.input_dir
    
    if args.output_dir==None:
        odir = create_nlp_dir()
    
    elif not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)
        odir = args.output_dir

    else:
        odir = args.output_dir
        
    return args.mode, args.input_dir, odir, args.max_items    

def shorten_text(text):
    
    ntext = text.replace("\n", "")
    
    return ntext.strip()

def extract_references(filenames, ofile):

    nlp_model = init_nlp_model("semantic")

    fw = open(ofile, "w")

    total=0
    for filename in tqdm.tqdm(filenames):        
        #print(f"reading {filename}")

        try:
            with open(filename, "r") as fr:
                idoc = json.load(fr)
        except:
            continue

        if random.random()<0.9:
            training_sample = True
        else:
            training_sample = False
        
        odoc = nlp_model.apply_on_doc(idoc)

        props = pd.DataFrame(odoc["properties"]["data"],
                             columns=odoc["properties"]["headers"])

        props_refs = props[props["label"]=="reference"]
        #print(props_refs)
        refs_hash = list(props_refs["subj_hash"])

        texts = pd.DataFrame.from_records(odoc["texts"])
        #print(texts)

        refs = pd.merge(props_refs, texts, how='inner', on=['subj_hash'])
        #print(refs[refs["confidence"]>0.95][["confidence", "text"]])

        for i,ref in refs.iterrows():

            if ref["confidence"]>0.95 and len(ref["text"])>32:
                item = {"training-sample": training_sample, "text": ref["text"]}
                fw.write(json.dumps(item)+"\n")

        #input("continue ...")
            
        """
        for item in odoc["texts"]:

            if "properties" not in item:
                continue
            

            if (df[df["type"]=="semantic"]["label"]=="reference").bool():
                #print(item["text"])

                total += 1

                if random.random()<0.9:
                    training_sample = True
                else:
                    training_sample = False
                
                item = {"training-sample": training_sample, "text": item["text"]}
                fw.write(json.dumps(item)+"\n")
        """
        
    fw.close()

    print("#-items: ", total)

def parse_with_anystyle_api(refs):

    time.sleep(1)

    tmpfile = "tmp.json"
    
    payload = { "input": [] }
    for ref in refs:
        payload["input"].append(ref["text"])

    anystyle_token = '9fEhg+39p0J60Bs+WTTwTMcqqTFAUYoyjLlp8nEys4wnfgACn0IoqravX8Exsx/+2q1p4sU7636DR22xUeneLg=='
    anystyle_session = '9GFKMlFoJwbMV6W1Z37YFsG9nbXLqmGicXVzL4r5mn4SqTLcf0revMMFvAjfxcjqR8YBnj2M0fgTWBW12kK1KMFcOgZvZnwQv5lZZ3PQgPP9sait9WgoDR72BHqRpbPe0c1B6%2BNFtYE7aqpugLsTupqBuj%2B%2Fef0tbyd84wC61GkVA9Vtz2nSNC90hDliCre%2BZ2gQUc6runu6yt1M4xa0F8kM4Cxt2pN92XB8hRusqGNfsaCsw5JKdU%2FcDFtdh%2BYDSEBz6DjQFfJq81%2FTI%2F4ulku7mlv73vOC7ew%3D--o%2B2gjgNJqgCjYf4V--3mSN%2FKmNt68WTJsxBh9Bww%3D%3D'
        
    res = subprocess.call(['curl', 'https://anystyle.io/parse?format=json',
                           '-H', 'authority: anystyle.io'  ,
                           '-H', 'accept: application/json, text/plain, */*'  ,
                           '-H', 'accept-language: en-GB,en-US;q=0.9,en;q=0.8'  ,
                           '-H', 'content-type: application/json;charset=UTF-8'  ,
                           '-H', f'cookie: _any_style_session={anystyle_session}',
                           '-H', 'origin: https://anystyle.io'  ,
                           '-H', 'referer: https://anystyle.io/'  ,
                           '-H', 'sec-ch-ua: "Not_A Brand";v="99", "Google Chrome";v="109", "Chromium";v="109"'  ,
                           '-H', 'sec-ch-ua-mobile: ?0'  ,
                           '-H', 'sec-ch-ua-platform: "macOS"'  ,
                           '-H', 'sec-fetch-dest: empty'  ,
                           '-H', 'sec-fetch-mode: cors'  ,
                           '-H', 'sec-fetch-site: same-origin'  ,
                           '-H', 'user-agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36'  ,
                           '-H', f'x-csrf-token: {anystyle_token}',
                           '--data-raw', json.dumps(payload),
                           '--compressed', '-o', tmpfile, '-s'])

    try:
        with open(tmpfile, "r") as fr:
            tmp = json.load(fr)

        if os.path.exists(tmpfile):
            os.remove(tmpfile)
            
        return tmp
    except:
        print(tc.red("could not call anystyle API endpoint ..."))

    if os.path.exists(tmpfile):
        os.remove(tmpfile)
        
    return []

def update_references(refs, label_map):

    results = parse_with_anystyle_api(refs)

    if len(results)!=len(refs):
        return
    
    for j,item in enumerate(results):
                
        parts=[]
        for row in item:
            parts.append(row[1])
            
        text = " ".join(parts)
        if text!=refs[j]["text"]:
            print("WARNING: mismatch text")
            continue

        beg=0
        for k,row in enumerate(item):
            charlen = len(row[1].encode('utf-8'))
            
            item[k].append(beg)
            item[k].append(beg+charlen)

            beg += charlen
            beg += 1

        refs[j]["word_tokens"]["headers"].append("true-label")
                
        for ri,row_i in enumerate(refs[j]["word_tokens"]["data"]):

            label="__undef__"
            for rj,row_j in enumerate(item):
                if row_j[2]<=row_i[0] and row_i[1]<=row_j[3]:
                    label = row_j[0]
                    break

            if label in label_map:
                label = label_map[label]
            else:
                ##print(label)
                label = "null"
                
            refs[j]["word_tokens"]["data"][ri].append(label)

        """
        print(text)
        print("\n\n", tabulate(refs[j]["word_tokens"]["data"],
                               headers=refs[j]["word_tokens"]["headers"]))
        """
        
        refs[j]["annotated"]=True

def annotate(rfile, ofile, max_items):

    label_map = {
        "author": "authors",
        "title": "title",
        "container-title": "conference",
        "journal": "journal",
        "date": "date",
        "volume": "volume",
        "pages": "pages",
        "citation-number": "reference-number",
        "note": "note",
        "url": "url",
        "doi": "doi",
        "isbn": "isbn",
        "publisher": "publisher"
    }
    
    nlp_model = init_nlp_model("semantic", filters=["properties", "word_tokens"])

    num_lines = sum(1 for _ in open(rfile))

    if max_items!=-1:
        max_items = min(max_items, num_lines)
    else:
        max_items = num_lines
    
    refs=[]

    fr = open(rfile, "r")
    fw = open(ofile, "w")

    cnt = 0
    
    for i in tqdm.tqdm(range(0,max_items)):

        line = fr.readline().strip()
        if line==None or len(line)==0:
            break

        try:
            item = json.loads(line)
            ref = nlp_model.apply_on_text(item["text"])

            ref["training-sample"] = item["training-sample"]
            
            refs.append(ref)
            cnt += 1
        except:
            continue

        if len(refs)>=16:

            #print(f"\rreference-annotation: {cnt}/{num_lines}", end="")
            update_references(refs, label_map)            

            for ref in refs:
                if "annotated" in ref and ref["annotated"]:
                    fw.write(json.dumps(ref)+"\n")

            refs=[]

        #if max_items!=-1 and cnt>max_items:
        #    break
        
    print(" --> done")

    fr.close()
    fw.close()
        
    print(f"writing annotation to {ofile}")

def prepare_for_crf(afile):

    labels={}
    
    refs=[]
    
    fr = open(afile, "r")

    while True:

        line = fr.readline().strip()
        #print(line)
        
        if line==None or len(line)==0:
            break

        try:
            item = json.loads(line)            
        except:
            continue

        wt = item["word_tokens"]

        if item["annotated"]:

            tind = wt["headers"].index("word")
            lind = wt["headers"].index("true-label")

            for i,row in enumerate(wt["data"]):

                if row[tind] in ["(", "[", "{"]:
                    row[lind] = "obracket"

                if row[tind] in [")", "]", "}"]:
                    row[lind] = "cbracket"

                if row[tind] in [",", ".", "'", "\"", ";", ":"]:                    
                    row[lind] = row[tind]

                if row[tind] in ["and", "&"] and row[lind]=="author":                    
                    row[lind] = "author_and"                                        

                if row[tind] in ["et", "al"] and row[lind]=="author":                    
                    row[lind] = "author_etal"                                        

            tokens = []
            for i,row in enumerate(wt["data"]):

                text = wt["data"][i][tind]
                text = text.replace(" ", "")

                wt["data"][i][tind] = text
                tokens.append(wt["data"][i][tind])
                
            line = " ".join(tokens)

            if True: # update true-labels for authors 
                for i,row in enumerate(wt["data"]):
                
                    if i>0 and \
                       wt["data"][i-1][lind]=="author" and \
                       wt["data"][i][lind] in ["."]:
                        wt["data"][i][lind] = "author"

                ## example:
                ##  - `Wright, N. J., Drake, J. J., Mamajek, E. E., & Henry, G. W. 2011, The Astrophysical Journal, 743, 48, doi: 10.1088/0004-637x/743/1/48`,
                ## `- 5 . Bender , C . M . ; Orszag , S . A . : Advanced Mathematical Methods for Scientists and Engineers : Asymptotic Methods and Perturbation Theory Springer - Verlag New - York Inc , 1999`
                if((" . , " in line) or (" . ; " in line) or (" . : " in line)):

                    for i,row in enumerate(wt["data"]):
                        
                        if i>0 and i<len(wt["data"])-1 and \
                           wt["data"][i-1][tind]!="." and \
                               wt["data"][i-1][lind]=="author" and \
                               wt["data"][i+1][lind]=="author" and \
                               wt["data"][i][lind] in [","]:
                            wt["data"][i][lind] = "author"

            if True: # update true-labels for title, journal, editor, container-title, location, date
                
                for i,row in enumerate(wt["data"]):

                    if 0<i and i<len(wt["data"])-1 and \
                       wt["data"][i-1][lind] in ["title", "journal", "editor", "container-title", "volume", "pages", "location", "date", "url"] and \
                       wt["data"][i-1][lind]==wt["data"][i+1][lind] and \
                       len(wt["data"][i][lind])==1:
                        wt["data"][i][lind] = wt["data"][i-1][lind]

                for i,row in enumerate(wt["data"]):

                    if 0<i and i<len(wt["data"])-1 and \
                       wt["data"][i-1][lind] in ["title", "volume", "pages"] and \
                       wt["data"][i-0][lind]== "cbracket":
                        wt["data"][i][lind] = wt["data"][i-1][lind]                        
                    
            for i,row in enumerate(wt["data"]):

                if " " in wt["data"][i][tind]:
                    has_space=True
                
                label = wt["data"][i][lind]
                if label in labels:
                    labels[label] += 1
                else:
                    labels[label] = 1
            
            if item["filename"]=="2302.05256.pdf":
                print("text: ", item["text"])
                print(tabulate(wt["data"], headers=wt["headers"]), "\n")

            refs.append(item)

    fr.close()

    print(json.dumps(labels, indent=2))
    
    doc_to_inds={}
    
    for i,ref in enumerate(refs):

        refs[i]["training-sample"] = True

        fname = ref["filename"]
        if fname in doc_to_inds:
            doc_to_inds[fname].append(i)
        else:
            doc_to_inds[fname] = [i]

    # pick 10% random reference from each paper for validation
    for key,val in doc_to_inds.items():

        num = max(1, int(0.1*len(val)))
        print(key, " -> ", num, " / ", len(val))
        
        for l in range(0, num):
            k = int(random.random()*len(val))
            refs[val[k]]["training-sample"] = False

    fw = open(afile, "w")
    for i,ref in enumerate(refs):
        fw.write(json.dumps(ref)+"\n")
    fw.close()

    print(tc.green(f"training file has been written to {afile}"))

def train_crf(train_file, model_file, metrics_file):

    #model = andromeda_nlp.nlp_model()
    model = nlp_model()
                
    configs = model.get_train_configs()

    for config in configs:
        if config["mode"]=="train" and \
           config["model"]=="reference":

            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file
            config["files"]["metrics-file"] = metrics_file 
    
            model.train(config)


# To train a FST model with HPO, one can use
# 
# `./fasttext supervised -input <path-to-train.txt> -output model_name -autotune-validation <<path-to-valid.txt>> -autotune-duration 600 -autotune-modelsize 1M`
#
#  => the parameters can be found via
#
# `./fasttext dump model_cooking.bin args`
#
def train_fst(train_file, model_file, metrics_file):

    #model = andromeda_nlp.nlp_model()
    model = nlp_model()
                
    configs = model.get_train_configs()    
    print(configs)
    
    for config in configs:
        if config["mode"]=="train" and \
           config["model"]=="semantic":

            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file 
            config["files"]["metrics-file"] = metrics_file
            
            model.train(config)
            
def create_reference_model(mode:str, idir:str, odir:str, max_items:int=-1):

    json_files = glob.glob(os.path.join(idir, "*.json"))
    print("#-docs: ", len(json_files))
    
    sfile = os.path.join(odir, "nlp-references.data.jsonl")    
    afile = os.path.join(odir, "nlp-references.annot.jsonl")

    crf_model_file = os.path.join(odir, "crf_reference")
    crf_metrics_file = crf_model_file+".metrics.txt"
    
    """
    rfile = os.path.join(tdir, "nlp-train-references-crf.jsonl")

    fst_model_file = os.path.join(tdir, "fst_sematic")
    """
    
    if mode=="extract" or mode=="all":
        extract_references(json_files, sfile, max_items)

    if mode=="annotate" or mode=="all":
        annotate(sfile, afile, max_items)

    if mode=="train" or mode=="all":
        train_crf(afile, crf_model_file, crf_metrics_file)

    """        
    if "classify" in mode or mode=="all":

        if mode=="classify" or mode==all:
            extract_references(sdir, sfile, rfile)
            
        train_fst(sfile, fst_model_file, fst_model_file+".metrics.txt")

    if "crf" in mode or mode=="all":

        if mode=="crf" or mode=="all":
            prepare_for_crf(afile)
        
        train_crf(afile, crf_model_file, crf_model_file+".metrics.txt")
    """
    
if __name__ == '__main__':

    mode, idir, odir, max_items = parse_arguments()

    create_reference_model(mode, idir, odir, max_items)
