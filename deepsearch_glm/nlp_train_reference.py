#!/usr/bin/env python

import os
import re

import time
import json
import glob
import argparse

import random

import subprocess

import numpy as np

import pandas as pd
import matplotlib.pyplot as plt

#import fasttext
import textColor as tc

#import deepsearch as ds
#from tabulate import tabulate

import andromeda_nlp

from deepsearch_glm.utils.ds_utils import convert_pdffiles
from deepsearch_glm.nlp_utils import create_nlp_dir



def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = "nlp_train_reference",
        description = 'Prepare CRF data for CRF-reference parser',
        epilog =
"""
examples of execution: 

1. end-to-end example on pdf documents:

    poetry run python ./deepsearch_glm/nlp_train_reference.py -m all --pdf './data/documents/articles/*.pdf'

""",
        formatter_class=argparse.RawTextHelpFormatter)
        
    parser.add_argument('-m', '--mode', required=False, default="all",
                        help="parse: [convert,extract,annotate,classify,pure-classify,crf,pure-crf,all]")

    parser.add_argument('--pdf', required=True,
                        type=str, default=None,
                        help="filename(s) of pdf document")

    parser.add_argument('--json', required=False,
                        type=str, default=None,
                        help="filename(s) of json document")
    
    parser.add_argument('--output-dir', required=False,
                        type=str, default=create_nlp_dir(),
                        help="output root directory for trained models")
    
    """
    parser.add_argument('', '--source-directory', required=False, default="./data/documents/articles",
                        help="directory with pdfs")
    parser.add_argument('-t', '--target-directory', required=False, default="./data/models/",
                        help="directory for target files")
    """
    
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
    
    if not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)
    
    return args.mode, pdf_files, json_files, args.output_dir

def shorten_text(text):
    
    ntext = text.replace("\n", "")
    
    return ntext.strip()
        
def extract_references(filenames, sfile, rfile):

    print(f"extract references for filenames: ", len(filenames))
    
    config = {
        "mode" : "apply",
        "order" : True,
        "models": "numval,link"
    }
    
    model = andromeda_nlp.nlp_model()
    model.initialise(config)
    
    MINLEN = 5

    fws = open(sfile, "w")
    fwr = open(rfile, "w")
    
    for filename in filenames:

        if filename.endswith("references.json"):
            continue
        
        print(f"reading {filename}")

        try:
            with open(filename, "r") as fr:
                data = json.load(fr)
        except:
            continue
        
        with open(filename, "w") as fw:
            fw.write(json.dumps(data, indent=2))             

        is_ref=False
        cnt_ref=0
        
        for item in data["main-text"]:

            if "text" in item:
                text = item["text"].strip()
            else:
                continue

            if "type" in item:
                label = item["type"]
            else:
                continue

            label = (label.split("-"))[0]            

            content = text.lower().strip().replace(" ", "")
            
            if content.endswith("references"):
                is_ref = True
            elif is_ref and label=="subtitle":
                is_ref = False

            if is_ref and (not content.endswith("references")) and len(text)>=MINLEN:                
                label = "reference"
            elif re.match("^(\d+|\[\d+\])(.*)\((19|20)\d{2}\)\.?$", text):
                label = "reference"
            elif re.match("^(\[\d+\])(\s+[A-Z]\.)+.*", text):
                label = "reference"                
            elif re.match("^(\[\])(.*)\((19|20)\d{2}\)\.?$", text):
                label = "reference"
            elif re.match("^(Table|Figure)(\s+\d+(\.\d+)?)(.*)", text):
                label = "caption"                
            elif len(text.strip())<MINLEN:
                label = "paragraph"
            elif label=="title":
                label = "subtitle"

            nlpres = model.apply_on_text(text)
            nlpres["filename"] = os.path.basename(filename.replace(".json", ".pdf"))

            if(label=="subtitle"):            
                print(nlpres["filename"], "\t", label, "\t", tc.yellow(text[0:96]))                
            elif(label=="reference"):            
                print(nlpres["filename"], "\t", label, "\t\t", tc.blue(text[0:96]))
            elif(label=="caption"):            
                print(nlpres["filename"], "\t", label, "\t\t", tc.red(text[0:96]))                                
            else:
                print(nlpres["filename"], "\t", label, "\t\t", text[0:96])
                
            if label=="reference":
                cnt_ref += 1
                fwr.write(json.dumps(nlpres)+"\n")

            if True:

                """
                print(tabulate(nlpres["word-tokens"]["data"],
                               headers=nlpres["word-tokens"]["headers"]))
                """

                """
                print(tabulate(nlpres["properties"]["data"],
                               headers=nlpres["properties"]["headers"]))
                """
                
                tind = nlpres["properties"]["headers"].index("type")
                lind = nlpres["properties"]["headers"].index("label")
                cind = nlpres["properties"]["headers"].index("confidence")

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
                    fws.write(json.dumps(nlpres)+"\n")
                
        if cnt_ref>0:
            print(tc.green(f"{filename}: {cnt_ref}"))
        else:
            print(tc.yellow(f"{filename}: {cnt_ref}"))
                
    fws.close()
    fwr.close()

    print(f"semantic-classification dumped in {sfile}")
    print(f"references dumped in {rfile}")

def parse_with_anystyle_api(tlines):

    time.sleep(1)

    tmpfile = "tmp.json"
    
    payload = { "input": [] }
    for tline in tlines:
        payload["input"].append(tline[1])

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

def update_references(refs, tlines):

    results = parse_with_anystyle_api(tlines)
    
    for j,item in enumerate(results):
                
        parts=[]
        for row in item:
            parts.append(row[1])
            
        text = " ".join(parts)
        if text!=tlines[j][1]:
            continue

        beg=0
        for k,row in enumerate(item):
            charlen = len(row[1].encode('utf-8'))
            
            item[k].append(beg)
            item[k].append(beg+charlen)

            beg += charlen
            beg += 1

        ind = tlines[j][0]
        
        refs[ind]["word-tokens"]["headers"].append("true-label")
                
        for ri,row_i in enumerate(refs[ind]["word-tokens"]["data"]):

            label="__undef__"
            for rj,row_j in enumerate(item):
                if row_j[2]<=row_i[0] and row_i[1]<=row_j[3]:
                    label = row_j[0]
                    break

            refs[ind]["word-tokens"]["data"][ri].append(label)

        """
        print(tabulate(refs[ind]["word-tokens"]["data"],
                       headers=refs[ind]["word-tokens"]["headers"]))
        """
        
        refs[ind]["annotated"]=True
            
    tlines=[]

def annotate(rfile, ofile):

    refs=[]

    fr = open(rfile, "r")

    while True:

        line = fr.readline().strip()
        if line==None or len(line)==0:
            break

        try:
            item = json.loads(line)
            refs.append(item)
        except:
            continue

    fr.close()

    print("#-refs: ", len(refs))
    
    tlines=[]
    for ind,ref in enumerate(refs):

        print(f"\rreferennce-annotation: {ind}/{len(refs)}", end="")
        
        refs[ind]["annotated"]=False
        tlines.append([ind, ref["text"]])
        
        if len(tlines)>0 and len(tlines)%16==0:
            update_references(refs, tlines)
            tlines=[]

    print(" --> done")
            
    if len(tlines)>0:
        update_references(refs, tlines)
            
    fw = open(ofile, "w")    

    for ref in refs:
        if "annotated" in ref and ref["annotated"]:
            fw.write(json.dumps(ref)+"\n")

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

        wt = item["word-tokens"]

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

    model = andromeda_nlp.nlp_model()
                
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

    model = andromeda_nlp.nlp_model()
                
    configs = model.get_train_configs()    
    print(configs)
    
    for config in configs:
        if config["mode"]=="train" and \
           config["model"]=="semantic":

            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file 
            config["files"]["metrics-file"] = metrics_file
            
            model.train(config)
            
if __name__ == '__main__':

    mode, pdf_files, json_files, tdir = parse_arguments()
    
    if len(pdf_files)>0:
        new_json_files = convert_pdffiles(pdf_files, force=False)

        for _ in new_json_files:
            json_files.append(_)

    json_files = sorted(list(set(json_files)))        
    
    sfile = os.path.join(tdir, "nlp-train-semantic-classification.annot.jsonl")
    rfile = os.path.join(tdir, "nlp-train-references-crf.jsonl")
    afile = os.path.join(tdir, "nlp-train-references-crf.annot.jsonl")
    
    crf_model_file = os.path.join(tdir, "crf_reference")
    fst_model_file = os.path.join(tdir, "fst_sematic")

    if mode=="extract" or mode=="all":
        extract_references(json_files, sfile, rfile)

    if mode=="annotate" or mode=="all":
        annotate(rfile, afile)

    if "classify" in mode or mode=="all":

        if mode=="classify" or mode==all:
            extract_references(sdir, sfile, rfile)
            
        train_fst(sfile, fst_model_file, fst_model_file+".metrics.txt")

    if "crf" in mode or mode=="all":

        if mode=="crf" or mode=="all":
            prepare_for_crf(afile)
        
        train_crf(afile, crf_model_file, crf_model_file+".metrics.txt")
        
