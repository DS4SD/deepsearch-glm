
import os

import json
import copy
import glob
import hashlib
import datetime
import subprocess

from tqdm import tqdm
from numerize.numerize import numerize
from dotenv import load_dotenv

import deepsearch as ds

from deepsearch.cps.client.components.elastic import ElasticDataCollectionSource
from deepsearch.cps.queries import DataQuery
from deepsearch.cps.client.components.queries import RunQueryError

def get_scratch_dir():
    
    load_dotenv()

    tmpdir = os.path.abspath(os.getenv("DEEPSEARCH_GLM_SCRATCH_DIR"))

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

    # avoid common mistakes ...
    if host=="https://deepsearch-experience.res.ibm.com/":
        verify_ssl=True
    elif host=="https://cps.foc-deepsearch.zurich.ibm.com/":
        verify_ssl=False
    
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
    #print(f"zips: ", len(zipfiles))

    for zipfile in zipfiles:
        cmd = ["unzip", zipfile, "-d", tdir]
        print(" ".join(cmd))

        subprocess.call(cmd)

    # clean up
    for i,zipfile in enumerate(zipfiles):
        print(i, "\t removing ", zipfile)
        subprocess.call(["rm", zipfile])        

    """
    cellsfiles = sorted(glob.glob(os.path.join(tdir, "*.cells")))
    for i,cellsfile in enumerate(cellsfiles):
        subprocess.call(["rm", cellsfile])            
    """
    
def convert_pdffiles(pdf_files, force=False):

    for i,pdf_file in enumerate(pdf_files):

        old_file = pdf_file
        """
        if os.path.exists(f"{old_file}"):
            print("exists ...")
        """
        
        subprocess.call(["ls", f"{old_file}"])

        if " " in pdf_file:
            new_file = pdf_file.replace(" ", "_")
            subprocess.call(["mv", f"{old_file}", f"{new_file}"])
            pdf_files[i] = new_file
    
    json_files=[]
    
    new_pdfs=[]
    for pdf_file in pdf_files:

        if force:
            new_pdfs.append(pdf_file)
        else:        
            json_file = pdf_file.replace(".pdf", ".json")
            if os.path.exists(json_file):
                json_files.append(json_file)
            else:
                new_pdfs.append(pdf_file)

    if len(new_pdfs)==0:
        return json_files
    else:
        print("found new pdf's: ", json.dumps(new_pdfs, indent=2))
        
    scratch_dir = get_scratch_dir()    
    
    old_pdfs = glob.glob(os.path.join(scratch_dir, "*.pdf"))
    for old_pdf in old_pdfs:
        subprocess.call(["rm", f"{old_pdf}"])
    
    for new_pdf in new_pdfs:
        subprocess.call(["cp", f"{new_pdf}", scratch_dir])

    ds_api, ds_proj = get_ds_api()
    
    documents = ds.convert_documents(api=ds_api, proj_key=ds_proj,
                                     source_path=scratch_dir,
                                     progress_bar=True)           

    documents.download_all(result_dir=scratch_dir)
    process_zip_files(scratch_dir)

    info = documents.generate_report(result_dir=scratch_dir)
    
    for new_pdf in new_pdfs:
        basename = os.path.basename(new_pdf).replace(".pdf", ".json")
        
        sfile = os.path.join(scratch_dir, basename)
        tfile = new_pdf.replace(".pdf", ".json")

        cmd = ["cp", f"{sfile}", f"{tfile}"]
        print(" ".join(cmd))

        subprocess.call(cmd)
        json_files.append(tfile)

    json_files = sorted(list(set(json_files)))
    return json_files

def ds_list_indices():

    api, proj_key = get_ds_api()
    
    # Fetch list of all data collections
    collections = api.elastic.list()
    collections.sort(key=lambda c: c.name.lower())    

    # Visualize summary table
    results = [
        {
            "Name": c.name,
            "Type": c.metadata.type,
            "Num entries": numerize(c.documents),
            "Date": c.metadata.created.strftime("%Y-%m-%d"),
            "Id": f"{c.source.elastic_id}",
            "Index": f"{c.source.index_key}",
        }
        for c in collections
    ]
    return results

def create_docs_dir():

    tdir = get_scratch_dir()

    now = datetime.datetime.now()
    odir = now.strftime("document-collection-%Y-%m-%d_%H-%M-%S")

    odir = os.path.join(tdir, odir)
    return odir    

def ds_index_query(index, query, odir=None, force=False, limit=-1,
                   sources=["file-info", "description",
                            "main-text",
                            "texts", "tables", "figures",                
                            "page-headers", "page-footers",
                            "footnotes"] # Which fields of documents we want to fetch
                   ):
    
    api, proj_key = get_ds_api()
    
    tdir = get_scratch_dir()

    if odir==None:
        dumpdir = hashlib.md5(f"{index}:{query}".encode()).hexdigest()
        dumpdir = os.path.join(tdir, dirname)
    else:
        dumpdir = odir
        
    if not os.path.exists(dumpdir):
        os.mkdir(dumpdir)
    elif not force:
        return dumpdir
    
    # Input query
    search_query = f"{query}" #"\"global warming potential\" AND \"etching\""
    print(f"query: {query}")
    
    data_collection = ElasticDataCollectionSource(elastic_id="default", index_key=index)
    page_size = 50

    """
        source=["file-info", "description",
                "main-text",
                "texts", "tables", "figures",                
                "page-headers", "page-footers",
                "footnotes"], 
    """
    
    # Prepare the data query
    query = DataQuery(
        search_query, # The search query to be executed
        source=sources, # Which fields of documents we want to fetch
        limit=page_size, # The size of each request page
        coordinates=data_collection # The data collection to be queries
    )

    # [Optional] Compute the number of total results matched. This can be used to monitor the pagination progress.
    count_query = copy.deepcopy(query)
    count_query.paginated_task.parameters["limit"] = 0
    count_results = api.queries.run(count_query)
       
    expected_total = count_results.outputs["data_count"]
    print(f"#-found documents: {expected_total}")

    if limit!=-1 and expected_total>limit:
        expected_total=limit    

    expected_pages = (expected_total + page_size - 1) // page_size # this is simply a ceiling formula
    
    # Iterate through all results by fetching `page_size` results at the same time
    bar_format = '{desc:<5.5}{percentage:3.0f}%|{bar:70}{r_bar}'

    count=0
    
    all_results = []
    cursor = api.queries.run_paginated_query(query)
    for result_page in tqdm(cursor, total=expected_pages, bar_format=bar_format):
        for row in result_page.outputs["data_outputs"]:
            _id = row["_id"]
            with open(f"{dumpdir}/{_id}.json", "w") as fw:
                fw.write(json.dumps(row["_source"], indent=2))

            count+=1
            if limit!=-1 and count>=limit:
                break

        if limit!=-1 and count>=limit:
            break            
                
    return dumpdir

