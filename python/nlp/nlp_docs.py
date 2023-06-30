#!/usr/bin/env python

import os

import json
import glob

import argparse

from tabulate import tabulate
from PIL import Image, ImageDraw

import subprocess

import pandas as pd
import textwrap as tw
import textColor as tc

import andromeda_nlp

import deepsearch as ds

deepsearch_host = "https://deepsearch-experience.res.ibm.com"
deepsearch_proj = "1234567890abcdefghijklmnopqrstvwyz123456"

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'nlp_docs',
        description = 'Apply Andromeda-NLP on `Deep Search` documents',
        epilog = 'Text at the bottom of help')

    parser.add_argument('-m', '--mode', required=False, default="show",
                        help="mode [convert;show;run-doc;run-docs]")
    parser.add_argument('-i', '--input-data', required=False,
                        help="input directory or filename",
                        default="../data/documents/reports")

    parser.add_argument('-v', '--vpage', required=False, help="visualise reading order on page-number",
                        default=None)

    parser.add_argument('--models', required=False, help="set NLP models (e.g. `term;sentence`)",
                        default="term;abbreviation")
    
    parser.add_argument('-u', '--username', required=False, default="<email>",
                        help="username or email from DS host")
    parser.add_argument('-p', '--password', required=False, default="<API_KEY>",
                        help="API-key from DS host")
    
    args = parser.parse_args()

    return args.mode, args.input_data, args.models, args.vpage, args.username, args.password

def convert(sdirectory, username, password):

    pdfs_files=glob.glob(os.path.join(sdirectory, "*.pdf"))
    json_files=glob.glob(os.path.join(sdirectory, "*.json"))

    new_pdfs=[]
    
    found_new_pdfs=False
    for pdf_file in pdfs_files:

        json_file = pdf_file.replace(".pdf", ".json")
        if json_file not in json_files:
            new_pdfs.append(pdf_file)
            found_new_pdfs = True

    print("found new pdf's: ", found_new_pdfs)
            
    if not found_new_pdfs:
        return found_new_pdfs
    
    config_ = {
        "host": deepsearch_host,
        "auth": {
            "username": username,
            "api_key": password,
        },
        "verify_ssl": True
    }

    config_file = "ds_config.json"
    with open(config_file, "w") as fw:
        fw.write(json.dumps(config_))
    
    config = ds.DeepSearchConfig.parse_file(config_file)
    
    client = ds.CpsApiClient(config)
    api = ds.CpsApi(client)

    documents = ds.convert_documents(api=api, proj_key=deepsearch_proj,
                                     source_path=sdirectory, progress_bar=True)           
    documents.download_all(result_dir=sdirectory)

    info = documents.generate_report(result_dir=sdirectory)
    return found_new_pdfs

def process_zip_files(sdir):

    jsonfiles = sorted(glob.glob(os.path.join(sdir, "*.json")))
    for i,jsonfile in enumerate(jsonfiles):
        subprocess.call(["rm", jsonfile])

    cellsfiles = sorted(glob.glob(os.path.join(sdir, "*.cells")))
    for i,cellsfile in enumerate(cellsfiles):
        subprocess.call(["rm", cellsfile])            
    
    zipfiles = sorted(glob.glob(os.path.join(sdir, "*.zip")))
    print(f"zips: ", len(zipfiles))

    for zipfile in zipfiles:
        subprocess.call(["unzip", zipfile, "-d", sdir])    

    for i,zipfile in enumerate(zipfiles):
        print(i, "\t removing ", zipfile)
        subprocess.call(["rm", zipfile])        

    cellsfiles = sorted(glob.glob(os.path.join(sdir, "*.cells")))
    for i,cellsfile in enumerate(cellsfiles):
        subprocess.call(["rm", cellsfile])            

def resolve_item(item, doc):

    if "$ref" in item:

        path = item["$ref"].split("/")
        return doc[path[1]][int(path[2])]

    elif "__ref" in item:

        path = item["__ref"].split("/")
        return doc[path[1]][int(path[2])]

    else:
        return item

def viz_docs(doc_i, doc_j, page=1):

    for dim in doc_i["page-dimensions"]:

        pn = dim["page"]

        if pn!=page:
            continue
        
        ih = int(dim["height"])
        iw = int(dim["width"])
    
        rects_i=[]
        for item in doc_i["main-text"]:

            ritem = resolve_item(item, doc_i)

            for prov in ritem["prov"]:
                if prov["page"]==pn:
                    rects_i.append(prov["bbox"])    

        rects_j=[]
        for item in doc_j["page-items"]:

            if item["page"]==pn:
                rects_j.append(item["bbox"])    

        rects_k=[]                
        for item in doc_j["main-text"]:
                
            ritem = resolve_item(item, doc_j)
            #print(ritem)
            
            for prov in ritem["prov"]:
                if prov["page"]==pn:
                    rects_k.append(prov["bbox"])    
            
        img = Image.new("RGB", (2*iw, ih))
        drw = ImageDraw.Draw(img)

        drw.text((1*iw/2, 1), "ORIGINAL", fill=(255, 255, 255))
        drw.text((3*iw/2, 1), "ORDERED", fill=(255, 255, 255))

        drw.rectangle((( 0, 0), (1*iw-1, ih-1)), outline="white")
        drw.rectangle(((iw, 0), (2*iw-1, ih-1)), outline="white")

        orig=(0,0)
        p0=orig        
        for ind,rect in enumerate(rects_i):

            shape = ((orig[0]+rect[0], ih-rect[3]), (orig[0]+rect[2], ih-rect[1]))
            drw.rectangle(shape, outline ="red")

            p1=(orig[0]+(rect[0]+rect[2])/2, (2*ih-rect[3]-rect[1])/2)
            drw.line((p0, p1), fill="blue")
            drw.text(p1, f"{ind}", fill=(255, 255, 255))

            p0=p1

        orig=(iw,0)
        p0=orig        
        for ind,rect in enumerate(rects_j):

            shape = ((orig[0]+rect[0], ih-rect[3]), (orig[0]+rect[2], ih-rect[1]))
            drw.rectangle(shape, outline ="red")

            p1=(orig[0]+(rect[0]+rect[2])/2, (2*ih-rect[3]-rect[1])/2)
            drw.line((p0, p1), fill="blue")
            drw.text(p1, f"{ind}", fill=(255, 255, 255))

            p0=p1
            
        img.show()
        
def get_label(item, model_name):

    if "properties" not in item:
        return None, None
    
    #print(tabulate(item["properties"]["data"], headers=item["properties"]["headers"]))
    
    mind = item["properties"]["headers"].index("type")
    lind = item["properties"]["headers"].index("label")
    cind = item["properties"]["headers"].index("confidence")
    
    for row in item["properties"]["data"]:
        if model_name==row[mind]:
            return row[lind], row[cind]

    return None, None

def get_ents(item, model_name):

    #print(item["entities"]["headers"])
    
    text = item["text"]
    
    mind = item["entities"]["headers"].index("type")
    sind = item["entities"]["headers"].index("subj_path")
    oind = item["entities"]["headers"].index("original")

    result=[]
    
    for row in item["entities"]["data"]:
        if model_name==row[mind]:
            result.append([row[mind], row[sind], "\n".join(tw.wrap(row[oind]))])

    return result, ["type", "path", "sentence"]
        
def display_sentences(doc_j):

    sentences=[]
    
    for i,item in enumerate(doc_j["main-text"]):

        #print(item["name"])
        
        if item["type"]!="paragraph":
            continue

        page = item["prov"][0]["page"]
        
        ents, headers = get_ents(item, "sentence")

        for ent in ents:
            sentences.append([page]+ent)


    print("\n\n")            
    print("sentences: ", len(sentences))            
    print(tabulate(sentences))
            
def display_maintext(doc_j):

    mtext=[]
    
    print("\n\n")            
    print("main-text: ", len(doc_j["main-text"]))
    for i,item in enumerate(doc_j["main-text"]):

        ritem = resolve_item(item, doc_j)
        #print(item, " => ", len(ritem["prov"]))
        #print(" -> ", ritem)
        
        name_ = item["name"]
        type_ = item["type"]

        #print(" -> ", ritem["prov"][0])        
        page = ritem["prov"][0]["page"]
        
        lanlabel, lconf = get_label(ritem, "language")
        semlabel, sconf = get_label(ritem, "semantic")

        text = " ... "
        if "text" in ritem:# and ("type" in item) and (item["type"] in ["paragraph", "subtitle-level-1"])):
            
            text = ritem["text"]
            if len(text)>64:
                text = ritem["text"][0:32] + " ... " + ritem["text"][len(text)-27:len(text)]
                
        mtext.append([i, page, type_, name_, semlabel, sconf, lanlabel, text])

    headers=["index", "page", "type", "name", "semantic", "confidence", "language", "text"]
    print(tabulate(mtext, headers=headers))
    
def display_tables(doc_j):

    print("\n\n")            
    print("tables: ", len(doc_j["tables"]))
    for i,table in enumerate(doc_j["tables"]):
        if "captions" in table and len(table["captions"])>0:

            text = None
            if "captions" in table and len(table["captions"])>0:
                text = table["captions"][0]["text"]

            if len(text)>64:
                text = text[0:32] + " ... " + text[len(text)-27:len(text)]
            
            print(i, "\tpage: ", table["prov"][0]["page"], "\t", text)
        else:
            print(i, "\tpage: ", table["prov"][0]["page"], "\t", None)

def display_figures(doc_j):            

    print("\n\n")            
    print("figures: ", len(doc_j["figures"]))
    for i,figure in enumerate(doc_j["figures"]):
        if "captions" in figure and len(figure["captions"])>0:

            text = None
            if "captions" in figure and len(figure["captions"])>0:
                text = figure["captions"][0]["text"]

            if len(text)>64:
                text = text[0:32] + " ... " + text[len(text)-27:len(text)]
            
            print(i, "\tpage: ", figure["prov"][0]["page"], "\t", text)
        else:
            print(i, "\tpage: ", figure["prov"][0]["page"], "\t", None)
    
def run_nlp_on_doc(filename, vpage):

    print(f" --> writing {filename}")
    
    model = andromeda_nlp.nlp_model()

    config = model.get_apply_configs()[0]
    config["models"] = "name;term;language;reference"
    
    model.initialise(config)
    
    fr = open(filename, "r")
    doc_i = json.load(fr)
    fr.close()
    
    doc_j = model.apply_on_doc(doc_i)

    table=[]
    for i,item in enumerate(doc_j["page-items"]):
        table.append([i, item["__ref"], item["page"], item["type"]])

    print("# page-items: ", len(table))
    print(tabulate(table, headers=["index", "ref", "page", "type"]))
    
    table=[]
    for i,item in enumerate(doc_j["main-text"]):
        table.append([i, item["__ref"], item["type"]])

    print("# main-items: ", len(table))
    print(tabulate(table, headers=["index", "ref", "type"]))
        
    #df = pd.DataFrame(doc_j["entities"]["data"],
    #                  columns=doc_j["entities"]["headers"])
    #print(df)
    
    mtext=[]

    if vpage!=None:
        viz_docs(doc_i, doc_j, page=int(vpage))

    #display_sentences(doc_j)

    display_maintext(doc_j)

    display_tables(doc_j)
          
    display_figures(doc_j)            

    filename_j = filename.replace(".json", ".nlp.json")
    print(f" --> writing {filename_j}")
    
    fw = open(filename_j, "w")        
    fw.write(json.dumps(doc_j, indent=2))
    fw.close()        

def run_nlp_on_docs(sdir):

    filenames = glob.glob(os.path.join(sdir, "*.json"))
    print("filenames: ", json.dumps(filenames, indent=2))

    model = andromeda_nlp.nlp_model()

    config = model.get_apply_configs()[0]
    config["models"] = "name;term;language;reference"
    
    model.initialise(config)

    page_num=1
    
    for filename in filenames:

        if(filename.endswith(".nlp.json")):
            continue

        print(f"reading {filename}")
        run_nlp_on_doc(filename, vpage=-1)
    
if __name__ == '__main__':

    mode, sdir, models, vpage, uname, pword = parse_arguments()

    found_new_pdfs=False
    if uname!="<email>" and pword!="<API_KEY>":
        found_new_pdfs = convert(sdir, uname, pword)
    
    if found_new_pdfs:
        process_zip_files(sdir)

    if mode=="convert" and found_new_pdfs:
        print(tc.blue(f"converted files located in {sdir}"))
    #elif mode=="show":
    #    show_nlp_on_docs(sdir)
    elif mode=="run-doc":
        run_nlp_on_doc(sdir, vpage)                
    #elif mode=="run-docs":
    #    run_nlp_on_docs(sdir)
    else:
        print(tc.red(f"unknown {mode}"))
        
