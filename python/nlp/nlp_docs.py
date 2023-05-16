#!/usr/bin/env python

import os

import json
import glob

import textwrap
import argparse

from tabulate import tabulate

import subprocess

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
                        help="mode [convert;show;run]")
    parser.add_argument('-d', '--directory', required=False,
                        default="../data/documents/reports")

    parser.add_argument('--models', required=False, default="term;abbreviation")
    
    parser.add_argument('-u', '--username', required=False, default="<email>",
                        help="username or email from DS host")
    parser.add_argument('-p', '--password', required=False, default="<API_KEY>",
                        help="API-key from DS host")
    
    args = parser.parse_args()

    return args.mode, args.directory, args.models, args.username, args.password

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
        
def show_nlp_on_docs(sdir):

    filenames = glob.glob(os.path.join(sdir, "*.json"))
    print("filenames: ", filenames)
    
    model = andromeda_nlp.nlp_model()
    model.initialise("name;term;language;reference")
    
    for filename in filenames:

        if(filename.endswith(".nlp.json")):
            continue
        
        print(filename)

        fr = open(filename, "r")
        doc = json.load(fr)
        fr.close()
        
        for i,item in enumerate(doc["main-text"]):
            
            if "text" not in item:
                continue
            
            text = item["text"]

            if len(text)<64:
                continue
            
            res = model.apply_on_text(text)

            print(tc.yellow("text:"),"\n", "\n".join(tw.wrap(text)))
            
            props = res["properties"]
            print(tc.yellow("properties:"),"\n", tabulate(props["data"], headers=props["headers"]))
            
            ents = res["entities"]

            for i,row in enumerate(ents["data"]):
                if row[ents["headers"].index("type")]=="sentence":
                    row[ents["headers"].index("name")] = row[ents["headers"].index("name")][0:16]+"..."
                    row[ents["headers"].index("original")] = row[ents["headers"].index("original")][0:16] + "..."

                elif len(row[ents["headers"].index("name")])>32:
                    row[ents["headers"].index("name")] = row[ents["headers"].index("name")][0:16] + " ... "
                    row[ents["headers"].index("original")] = row[ents["headers"].index("original")][0:16] + " ... "
                else:
                    continue

            ents["data"] = sorted(ents["data"], key=lambda x: x[ents["headers"].index("char_i")])
                
            print(tc.yellow("entities:"),"\n", tabulate(ents["data"], headers=ents["headers"]))
            
            label, conf = get_label(props["data"], header=props["headers"], key="semantic")

            
        input(" ... ")

def run_nlp_on_docs(sdir):

    filenames = glob.glob(os.path.join(sdir, "*.json"))
    print("filenames: ", filenames)
    
    model = andromeda_nlp.nlp_model()
    model.initialise("term;language;reference;abbreviation")
    
    for filename in filenames:

        if(filename.endswith(".nlp.json")):
            continue
        
        print(f"reading {filename}")

        fr = open(filename, "r")
        doc = json.load(fr)
        fr.close()

        doc_ = model.apply_on_pdfdoc(doc)
        
        filename_ = filename+".nlp.json" 

        print(f" --> writing {filename_}")
        fw = open(filename_, "w")        
        fw.write(json.dumps(doc_, indent=2))
        fw.close()
        
if __name__ == '__main__':

    mode, sdir, models, uname, pword = parse_arguments()

    found_new_pdfs=False
    if uname!="<email>" and pword!="<API_KEY>":
        found_new_pdfs = convert(sdir, uname, pword)
    
    if found_new_pdfs:
        process_zip_files(sdir)

    if mode=="convert" and found_new_pdfs:
        print(tc.blue(f"converted files located in {sdir}"))
    elif mode=="show":
        show_nlp_on_docs(sdir)
    elif mode=="run":
        run_nlp_on_docs(sdir)
    else:
        print(tc.red(f"unknown {mode}"))
        
