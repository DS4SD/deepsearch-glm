#!/usr/bin/env python

import os
import copy
import json
import glob

import argparse
import textwrap

import numpy as np
import pandas as pd

from tabulate import tabulate
from PIL import Image, ImageDraw

import andromeda_nlp

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'nlp_analyse_docs',
        description = 'Analyse NLP on `Deep Search` documents ()',
        epilog =
"""
examples of execution:

1.a run on single document (pdf or json) with default model (=`langauge`):

    poetry run python ./deepsearch_glm/nlp_analyse_docs.py --json ./data/documents/articles/2305.02334.nlp.json

""",
        formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--json', required=True,
                        type=str, default=None,
                        help="filename(s) of json document")
    
    parser.add_argument('--mode', required=False,
                        type=str, default="all",
                        help="selection of what to show (default=all, example='page=0;refereces')")

    args = parser.parse_args()
    
    json_files=sorted(glob.glob(args.json))

    return json_files, args.mode

def extract_meta(doc):
    return

def show_page(doc, page_num=1, show_orig=True):

    print(doc.keys())
    
    page_dims = doc["page-dimensions"]
    df_dims = pd.read_json(json.dumps(page_dims), orient='records')
    print(df_dims)
    
    page_items = doc["page-items"]
    print(page_items[0])

    # FIXME: ineffecient but works ...
    df = pd.read_json(json.dumps(page_items), orient='records')
    print(df)

    df_page = df[df["page"]==page_num]

    ph = int(df_dims[df_dims["page"]==page_num]["height"][0])
    pw = int(df_dims[df_dims["page"]==page_num]["width"][0])

    print("height: ", ph)
    print("width: ", pw)
    
    df_page["bbox"] = df_page["bbox"].apply(lambda bbox: [bbox[0], ph-bbox[3], bbox[2], ph-bbox[1]])
    #df_page = df_page.apply(lambda row: [row["bbox"][0], ph-row["bbox"][3], row["bbox"][2], ph-row["bbox"][1]], axis=1)
    
    factor = 2 if show_orig else 1 
    
    # Create a new image with a white background
    image = Image.new("RGB", (factor*pw, ph), "white")

    # Create an ImageDraw object to draw on the image
    orig_order=[]
    points=[]

    draw = ImageDraw.Draw(image)
    for i,row in df_page.iterrows():

        bbox = row["bbox"]
        draw.rectangle(bbox, outline="red", width=2)

        orig_order.append(row["orig-order"])
        points.append((int((bbox[0]+bbox[2])/2.0),
                       int((bbox[1]+bbox[3])/2.0)))

    draw.line(points, fill="blue", width=2)

    for i,point in enumerate(points):
        draw.text(point, str(i), fill="black")
    
    if show_orig:
        for i,row in df_page.iterrows():
            bbox = row["bbox"]
            bbox[0] += pw
            bbox[2] += pw
            draw.rectangle(bbox, outline="red", width=2)

        opoints = copy.deepcopy(points)
        for i,j in enumerate(orig_order):
            x,y = points[i]
            opoints[j] = (x+pw,y)
        
        draw.line(opoints, fill="orange", width=2)    

        for i,point in enumerate(opoints):
            draw.text(point, str(i), fill="black")
        
    image.show()        
    
def extract_text(doc):

    page_items = doc["page-items"]
    texts = doc["texts"]

    print("\n\t TEXT: \n")
    
    wrapper = textwrap.TextWrapper(width=70)
    
    for item in doc["texts"]:

        labels=[]
        for row in item["properties"]["data"]:
            labels.append(row[item["properties"]["headers"].index("label")])
        
        type_ = item["prov"][0]["type"]
        print(f"text: {type_}, labels: ", ", ".join(labels))
        for line in wrapper.wrap(text=item["text"]):
            print(f"\t{line}")

        print("")
    return

def extract_sentences(doc):

    df = pd.DataFrame(doc["instances"]["data"],
                      columns=doc["instances"]["headers"])

    entity_types = df["type"].value_counts()
    print(entity_types)
    
    sents = df[df["type"]=="sentence"]
    print(sents)
    
    return

def extract_tables(doc):
    
    wrapper = wrapper = textwrap.TextWrapper(width=70)

    print("\n\t TABLES: \n")
    
    for i,item in enumerate(doc["tables"]):

        print(f"#/tables/{i}: ", len(item["captions"]))
        if len(item["captions"])>0:
            for line in wrapper.wrap(text=item["captions"][0]["text"]):
                print(f"\t{line}")
    
    return

def extract_figures(doc):

    print("\n\t FIGURES: \n")
    
    wrapper = wrapper = textwrap.TextWrapper(width=70)
    
    for i,item in enumerate(doc["figures"]):

        print(f"#/figures/{i}", len(item["captions"]))
        if len(item["captions"])>0:
            for line in wrapper.wrap(text=item["captions"][0]["text"]):
                print(f"\t{line}")
    
    return

def extract_references(doc):

    print("\n\t REFERENCES: \n")
    
    page_items = doc["page-items"]
    texts = doc["texts"]

    df = pd.DataFrame(doc["instances"]["data"],
                      columns=doc["instances"]["headers"])
    
    wrapper = wrapper = textwrap.TextWrapper(width=70)
    
    for i,item in enumerate(doc["texts"]):

        path = f"#/texts/{i}"
        
        labels=[]
        for row in item["properties"]["data"]:
            labels.append(row[item["properties"]["headers"].index("label")])

        if "reference" in labels:

            print(f"text: ") #{type_}, labels: ", ",".join(labels))
            for line in wrapper.wrap(text=item["text"]):
                print(f"\t{line}")            
                
            refs = df[df["subj_path"]==path][["subj_path", "type", "subtype", "original"]]
            print(refs)            
                
    return

if __name__ == '__main__':

    json_files, mode = parse_arguments()

    for json_file in json_files:

        print(f" --> reading {json_file}")
        with open(json_file, "r") as fr:
            doc = json.load(fr)

        if "page" in mode or mode=="all":
            show_page(doc, page_num=1)

        if "text" in mode or mode=="all":
            extract_text(doc)

        if "sentence" in mode or mode=="all":            
            extract_sentences(doc)

        if "table" in mode or mode=="all":            
            extract_tables(doc)

        if "figure" in mode or mode=="all":            
            extract_figures(doc)

        if "reference" in mode or mode=="all":            
            extract_references(doc)
