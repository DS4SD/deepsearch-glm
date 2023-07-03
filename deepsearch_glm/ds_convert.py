
import os

import json
import glob

import subprocess

import deepsearch as ds

from dotenv import load_dotenv

def load_vars():

    load_dotenv()
    
    host = os.getenv("DEEPSEARCH_HOST")
    proj = os.getenv("DEEPSEARCH_PROJ")
    
    username = os.getenv("DEEPSEARCH_USERNAME")
    apikey = os.getenv("DEEPSEARCH_APIKEY")

    verify_ssl = True #os.getenv("DEEPSEARCH_VERIFYSSL")
    tmpdir = os.path.abspath(os.getenv("DEEPSEARCH_TMPDIR"))

    if not os.path.exists(tmpdir):
        os.mkdirs(tmpdir)
        
    return host, proj, username, apikey, verify_ssl, tmpdir

def get_ds_api():

    host, proj, username, apikey, verify_ssl, tdir = load_vars()
    
    config_ = {
        "host": host,
        "auth": {
            "username": username,
            "api_key": apikey
        },
        "verify_ssl": verify_ssl
    }

    config_file = f"{tdir}/ds_config.json"
    with open(config_file, "w") as fw:
        fw.write(json.dumps(config_))
    
    config = ds.DeepSearchConfig.parse_file(config_file)
    
    client = ds.CpsApiClient(config)
    api = ds.CpsApi(client)

    return api

def process_zip_files(tdir):

    """
    jsonfiles = sorted(glob.glob(os.path.join(tdir, "*.json")))
    for i,jsonfile in enumerate(jsonfiles):
        subprocess.call(["rm", jsonfile])

    cellsfiles = sorted(glob.glob(os.path.join(tdir, "*.cells")))
    for i,cellsfile in enumerate(cellsfiles):
        subprocess.call(["rm", cellsfile])            
    """
    
    zipfiles = sorted(glob.glob(os.path.join(tdir, "*.zip")))
    print(f"zips: ", len(zipfiles))

    for zipfile in zipfiles:
        subprocess.call(["unzip", zipfile, "-d", tdir])    

    for i,zipfile in enumerate(zipfiles):
        print(i, "\t removing ", zipfile)
        subprocess.call(["rm", zipfile])        

    cellsfiles = sorted(glob.glob(os.path.join(tdir, "*.cells")))
    for i,cellsfile in enumerate(cellsfiles):
        subprocess.call(["rm", cellsfile])            

def convert_pdffile(pdffile, force=False):

    pdffile = os.path.abspath(pdffile)
    jsonfile = pdffile.replace(".pdf", ".json")
    
    dir_pdffile = os.path.dirname(pdffile)
    base_pdffile = os.path.basename(pdffile)
    base_jsonfile = os.path.basename(pdffile).replace(".pdf", ".json")

    if os.path.exists(jsonfile) and (not force):
        return True, jsonfile

    elif os.path.exists(pdffile):
        
        host, proj, username, apikey, verify_ssl, tdir = load_vars()
        subprocess.call(["cp", pdffile, f"{tdir}/{base_pdffile}"])
        
        ds_api = get_ds_api()
        
        docs = ds.convert_documents(api=ds_api, proj_key=proj,
                                    source_path=tdir, progress_bar=True)           
        docs.download_all(result_dir=tdir)

        process_zip_files(tdir)        
        subprocess.call(["cp", f"{tdir}/{base_jsonfile}", jsonfile])
        
        return True, jsonfile    
    else:
        return False, ""

    return 
    
def convert_pdfdir(sdirectory):

    host, proj, username, apikey = load_vars()
    
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

