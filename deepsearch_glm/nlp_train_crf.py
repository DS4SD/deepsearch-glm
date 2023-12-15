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
        prog = "nlp_train_crf",
        description = 'train CRF model',
        epilog =
"""
examples of execution: 

1. end-to-end example to train CRF:

    poetry run python ./deepsearch_glm/nlp_train_crf.py -m all --input-file <filename> --output-dir <models-directory>'

2. end-to-end example to train CRF with limited samples:

    poetry run python ./deepsearch_glm/nlp_train_crf.py -m all --input-file <filename> --output-dir <models-directory> --max-items 1000'
""",
        formatter_class=argparse.RawTextHelpFormatter)
        
    parser.add_argument('-m', '--mode', required=True, default="all",
                        help="mode for training semantic model",
                        choices=["prepare","train","all"])

    parser.add_argument('--input-file', required=False,
                        type=str, default=None,
                        help="input-file with annotations in jsonl format")
    
    parser.add_argument('--output-dir', required=False,
                        type=str, default="./reference-models",
                        help="output directory for trained models & prepared data")

    parser.add_argument('--max-items', required=False,
                        type=int, default=-1,
                        help="number of references")
    
    args = parser.parse_args()
    
    if args.output_dir==None:
        odir = create_nlp_dir()
    
    elif not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)
        odir = args.output_dir

    else:
        odir = args.output_dir
        
    return args.mode, args.input_file, odir, args.max_items    

def annotate_item(atem, item, labels, is_training_sample=True, append_to_file=False, start_and_end_in_utf8=True):

    atem["word_tokens"]["headers"].append("true-label")

    char_i_ind = atem["word_tokens"]["headers"].index("char_i")
    char_j_ind = atem["word_tokens"]["headers"].index("char_j")
    
    label="null"
    for ri,row_i in enumerate(atem["word_tokens"]["data"]):
        atem["word_tokens"]["data"][ri].append(label)

    text = atem["text"]

    print(item["text"])
    for annot in item["annotation"]:

        lbl = annot["label"].replace(" ", "_").lower().strip()
        utf8_rng = [annot["start"], annot["end"]]

        print(item["text"][utf8_rng[0]:utf8_rng[1]])

        #beg_ustr = item["text"][0:utf8_rng[0]]
        beg_ustr = text[0:utf8_rng[0]]
        beg_bstr = beg_ustr.encode("utf-8")

        #end_ustr = item["text"][0:utf8_rng[1]]
        end_ustr = text[0:utf8_rng[1]]
        end_bstr = end_ustr.encode("utf-8")

        byte_rng = [len(beg_bstr), len(end_bstr)]

        if start_and_end_in_utf8:
            char_i = byte_rng[0]
            char_j = byte_rng[1]
        else:
            char_i = utf8_rng[0]
            char_j = utf8_rng[1]
            
        for ri,row_i in enumerate(atem["word_tokens"]["data"]):
            if char_i<=row_i[char_i_ind] and row_i[char_j_ind]<=char_j:
                atem["word_tokens"]["data"][ri][-1] = lbl
        
    print(text)
    print("\n\n", tabulate(atem["word_tokens"]["data"],
                           headers=atem["word_tokens"]["headers"]))
    

    btext = text.encode("utf-8")
    #assert len(text)==len(btext)
    
    atem["annotated"]=True
    
    return atem
    
def prepare_crf(rfile, ofile, max_items, ratio=0.9):

    nlp_model = init_nlp_model("language", filters=["properties", "word_tokens"])

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

        item = json.loads(line)
        atem = nlp_model.apply_on_text(item["text"])
        
        atem = annotate_item(atem, item, labels=["chemicals"])

        if random.random()<ratio:
            atem["training-sample"] = True
        else:
            atem["training-sample"] = False

        if "annotated" in atem and atem["annotated"]:
            fw.write(json.dumps(atem)+"\n")
            
        cnt += 1

def train_crf(train_file, model_file, metrics_file):

    model = nlp_model()
                
    configs = model.get_train_configs()

    for config in configs:
        if config["mode"]=="train" and \
           config["model"]=="reference":

            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file
            config["files"]["metrics-file"] = metrics_file 
    
            model.train(config)
        
def create_crf_model(mode, ifile, odir, max_items):

    filename = os.path.basename(ifile)
    
    afile = os.path.join(odir, filename.replace(".jsonl", ".annot.jsonl"))

    crf_model_file = os.path.join(odir, filename.replace(".jsonl", ".crf_model.bin"))
    crf_metrics_file = crf_model_file+".metrics.txt"
    
    if mode=="prepare" or mode=="all":
        #prepare_crf(train_file, afile, max_items, is_training_sample=True, append_to_file=False)
        #prepare_crf(test_file, afile, max_items, is_training_sample=False, append_to_file=True)
        prepare_crf(ifile, afile, max_items, ratio=0.9)
        
    if mode=="train" or mode=="all":
        train_crf(afile, crf_model_file, crf_metrics_file)

    return afile, crf_model_file, crf_metrics_file
        
if __name__ == '__main__':

    mode, ifile, odir, max_items = parse_arguments()

    create_crf_model(mode, ifile, odir, max_items)
