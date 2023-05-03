#!/usr/bin/env python

import os

import json
import glob

import textwrap
import argparse
from tabulate import tabulate

import subprocess

import andromeda_nlp

import deepsearch as ds

deepsearch_host = "https://deepsearch-experience.res.ibm.com"
deepsearch_proj = "1234567890abcdefghijklmnopqrstvwyz123456"

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'nlp_docs',
        description = 'Apply Andromeda-NLP on `Deep Search` documents',
        epilog = 'Text at the bottom of help')

    parser.add_argument('-d', '--directory', required=True)

    parser.add_argument('-u', '--username', required=False, help="username or email from DS host")
    parser.add_argument('-p', '--password', required=False, help="API-key from DS host")
    
    args = parser.parse_args()

    return args.directory, args.username, args.password

def process_zip_files(sdir):

    jsonfiles = sorted(glob.glob(os.path.join(sdir, "*.json")))
    for i,jsonfile in enumerate(jsonfiles):
        print(i, "\t", jsonfile)
        subprocess.call(["rm", jsonfile])

    cellsfiles = sorted(glob.glob(os.path.join(sdir, "*.cells")))
    for i,cellsfile in enumerate(cellsfiles):
        print(i, "\t", cellsfile)
        subprocess.call(["rm", cellsfile])            
    
    zipfiles = sorted(glob.glob(os.path.join(sdir, "*.zip")))
    print(f"zips: ", len(zipfiles))

    for zipfile in zipfiles:
        subprocess.call(["unzip", zipfile, "-d", sdir])    

    jsonfiles = sorted(glob.glob(os.path.join(sdir, "*.json")))
    for i,jsonfile in enumerate(jsonfiles):
        print(i, "\t", jsonfile)

def convert(sdirectory, username, password):

    pdfs_files=glob.glob(os.path.join(sdirectory, "*.pdf"))
    json_files=glob.glob(os.path.join(sdirectory, "*.json"))

    found=True
    for pdf_file in pdfs_files:

        json_file = pdf_file.replace(".pdf", ".json")
        if json_file not in json_files:
            found = False

    if found:
        return
    
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

    info = documents.generate_report(result_dir="./converted_docs")
    print(info)
    
    process_zip_files(sdirectory)

def get_label(data, header, key):

    #print("properties: \n", tabulate(data, headers=header))
    
    i = header.index("type")
    j = header.index("label")
    k = header.index("confidence")
    
    for row in data:
        if row[i]==key:
            return row[j], row[k] 
        
    return "null", 0.0

def get_ent(ent_id, data, header, key="reference", text=""):

    #print("entities: \n", tabulate(data, headers=header))
    
    i = header.index("type")
    j = header.index("subtype")

    k = header.index("name")

    rows=[]
    for row in data:
        if row[i]==key:
            rows.append([ent_id, row[i], row[j], row[k], "\n".join(textwrap.wrap(text))])

    return rows, ["ent-id", "type", "subtype", "name", "text"]
        
def run_refs(sdir):

    filenames = glob.glob(os.path.join(sdir, "*.json"))
    print("filenames: ", filenames)
    
    model = andromeda_nlp.nlp_model()
    model.init("language;reference")
    
    for filename in filenames:
        print(filename)
        
        fr = open(filename)
        doc = json.load(fr)
        fr.close()
        
        if "main-text" not in doc:
            print(doc.keys())
            continue

        references=[]
        
        for i,item in enumerate(doc["main-text"]):
            
            if "text" not in item:
                continue
            
            text = item["text"]
            res = model.fit(text)

            props = res["properties"]
            ents = res["entities"]
            
            label, conf = get_label(props["data"], header=props["headers"], key="semantic")
            #print(label, "\t", text[0:96])

            if label=="reference" and conf>0.95:
                references.append(res)

                print("entities: \n", tabulate(ents["data"], headers=ents["headers"]))
            
            """
            print("\n".join(textwrap.wrap(text)))
            

            print("properties: \n", tabulate(props["data"], headers=props["headers"]))


            print("entities: \n", tabulate(ents["data"], headers=ents["headers"]))
            """

        """
        table=[]
        for i, reference in enumerate(references):
            print(reference.keys())
            
            ents = reference["entities"]            
            rows, headers = get_ent(i, ents["data"], header=ents["headers"], key="reference", text=reference["text"])

            table += rows

        print(tabulate(table, headers=headers))

        subtypes=[]
        for j,row in enumerate(table):
            subtypes.append(row[headers.index("subtype")])

        subtypes = list(set(subtypes))            
        print(subtypes)
        """
        
        table2=[]
        for i,reference in enumerate(references):

            hent = reference["entities"]["headers"]            
            ents = reference["entities"]["data"]            

            numb=[]
            for j,ent in enumerate(ents):
                if ent[hent.index("type")]=="reference" and \
                   ent[hent.index("subtype")]=="citation-number":
                    numb.append(ent[hent.index("name")])
                    break
                
            year=[]
            for j,ent in enumerate(ents):
                if ent[hent.index("type")]=="reference" and \
                   ent[hent.index("subtype")]=="date":
                    year.append(ent[hent.index("name")])
                    break

            authors=[]
            for j,ent in enumerate(ents):
                if ent[hent.index("type")]=="reference" and \
                   ent[hent.index("subtype")]=="author":
                    authors.append(ent[hent.index("name")])

            title=[]
            for j,ent in enumerate(ents):
                if ent[hent.index("type")]=="reference" and \
                   ent[hent.index("subtype")]=="title":
                    title.append(ent[hent.index("name")])                    
                    break
                
            journal=[]
            for j,ent in enumerate(ents):
                if ent[hent.index("type")]=="reference" and \
                   ent[hent.index("subtype")]=="journal":
                    journal.append(ent[hent.index("name")])                    
                    break

            table2.append(["; ".join(numb), "; ".join(year),
                           "; ".join(journal),
                           "\n".join(textwrap.wrap("; ".join(authors))),
                           "\n".join(textwrap.wrap("; ".join(title)))])

        print(tabulate(table2, headers=["reference-number", "year", "journal", "authors", "title"]))
            
        input("continue ...")
            
            
"""
def run_text(filenames):

    model = andromeda_nlp.nlp_model()
    model.init("term")
    
    for filename in filenames:
        print(filename)
        
        fr = open(filename)
        doc = json.load(fr)
        
        for item in doc["main-text"]:
            
            if "text" in item:
                
                text = item["text"]
                #print(f"text: {text}")
                
                res = model.fit(text)
                #print(res)

        fr.close()
"""

"""
def run_docs(filenames):

    model = andromeda_nlp.nlp_model()
    model.init("term;verb;conn")
    #model.init("reference")
    
    for filename in filenames:

        if(filename.endswith("nlp.json")):
            continue
            
        print(filename)
        
        fr = open(filename, "r")
        doc = json.load(fr)
        fr.close()

        fw = open(filename, "w")        
        fw.write(json.dumps(doc, indent=2))
        fw.close()
        
        doc_ = model.fit_pdfdoc(doc)

        filename_ = filename+".nlp.json" 

        fw = open(filename_, "w")        
        fw.write(json.dumps(doc_, indent=2))
        fw.close()
"""

if __name__ == '__main__':

    sdir, uname, pword = parse_arguments()

    convert(sdir, uname, pword)
        
    run_refs(sdir)
        
