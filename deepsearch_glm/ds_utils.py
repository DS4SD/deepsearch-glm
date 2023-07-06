
import os

import json
import copy
import glob
import hashlib


from tqdm import tqdm

import subprocess

from dotenv import load_dotenv

import deepsearch as ds

from deepsearch.cps.client.components.elastic import ElasticDataCollectionSource
from deepsearch.cps.queries import DataQuery
from deepsearch.cps.client.components.queries import RunQueryError

def get_scratch_dir():

    load_dotenv()

    tmpdir = os.path.abspath(os.getenv("DEEPSEARCH_TMPDIR"))

    if not os.path.exists(tmpdir):
        os.mkdir(tmpdir)

    return tmpdir
    
def load_vars():

    load_dotenv()
    
    host = os.getenv("DEEPSEARCH_HOST")
    proj = os.getenv("DEEPSEARCH_PROJ")
    
    username = os.getenv("DEEPSEARCH_USERNAME")
    apikey = os.getenv("DEEPSEARCH_APIKEY")

    verify_ssl = os.getenv("DEEPSEARCH_VERIFYSSL")
        
    return host, proj, username, apikey, verify_ssl

def get_ds_api():

    tdir = get_scratch_dir()
    
    host, proj, username, apikey, verify_ssl = load_vars()
    
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

    return api, proj

def process_zip_files(tdir):

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

        tdir = get_scratch_dir()

        subprocess.call(["cp", pdffile, f"{tdir}/{base_pdffile}"])
        
        ds_api, proj_key = get_ds_api()
        
        docs = ds.convert_documents(api=ds_api, proj_key=proj_key,
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

def ds_index_query(index, query, force=False):

    api, proj_key = get_ds_api()
    
    # Fetch list of all data collections
    #collections = api.elastic.list()
    #collections.sort(key=lambda c: c.name.lower())
    
    tdir = get_scratch_dir()
    dirname = hashlib.md5(f"{index}:{query}".encode()).hexdigest()

    dumpdir = os.path.join(tdir, dirname)

    if not os.path.exists(dumpdir):
        os.mkdir(dumpdir)
    elif not force:
        return dumpdir
    
    # Input query
    search_query = f"\"{query}\"" #"\"global warming potential\" AND \"etching\""
    data_collection = ElasticDataCollectionSource(elastic_id="default", index_key=index)
    page_size = 50

    # Prepare the data query
    query = DataQuery(
        search_query, # The search query to be executed
        source=["description", "main-text", "texts", "tables", "figures"], # Which fields of documents we want to fetch
        limit=page_size, # The size of each request page
        coordinates=data_collection # The data collection to be queries
    )

    # [Optional] Compute the number of total results matched. This can be used to monitor the pagination progress.
    count_query = copy.deepcopy(query)
    count_query.paginated_task.parameters["limit"] = 0
    count_results = api.queries.run(count_query)
    expected_total = count_results.outputs["data_count"]
    expected_pages = (expected_total + page_size - 1) // page_size # this is simply a ceiling formula
    
    # Iterate through all results by fetching `page_size` results at the same time
    all_results = []
    cursor = api.queries.run_paginated_query(query)
    for result_page in tqdm(cursor, total=expected_pages, bar_format='{desc:<5.5}{percentage:3.0f}%|{bar:70}{r_bar}'):
        for row in result_page.outputs["data_outputs"]:
            _id = row["_id"]
            with open(f"{dumpdir}/{_id}.json", "w") as fw:
                fw.write(json.dumps(row["_source"], indent=2))
    
    return dumpdir
