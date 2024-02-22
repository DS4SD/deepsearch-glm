"""Module to provide basic functionality of the Deep Search platform"""

import copy
import datetime
import glob
import hashlib
import json
import os
import re
import subprocess

import deepsearch as ds
from deepsearch.cps.client.components.elastic import ElasticDataCollectionSource

# from deepsearch.cps.client.components.queries import RunQueryError
from deepsearch.cps.queries import DataQuery
from dotenv import load_dotenv
from numerize.numerize import numerize
from tqdm import tqdm


def get_scratch_dir():
    """Get scratch directory from environment variable `DEEPSEARCH_GLM_SCRATCH_DIR` (defined in .env)"""

    load_dotenv()

    tmpdir = os.path.abspath(os.getenv("DEEPSEARCH_GLM_SCRATCH_DIR"))

    if not os.path.exists(tmpdir):
        os.mkdir(tmpdir)

    return tmpdir


def load_vars():
    """Load variables `DEEPSEARCH_HOST`, `DEEPSEARCH_PROJ`, `DEEPSEARCH_USERNAME`, `DEEPSEARCH_APIKEY` and `DEEPSEARCH_VERIFYSSL` (defined in .env)"""

    load_dotenv()

    host = os.getenv("DEEPSEARCH_HOST")
    proj = os.getenv("DEEPSEARCH_PROJ")

    username = os.getenv("DEEPSEARCH_USERNAME")
    apikey = os.getenv("DEEPSEARCH_APIKEY")

    verify_ssl = os.getenv("DEEPSEARCH_VERIFYSSL")

    # avoid common mistakes ...
    if host == "https://deepsearch-experience.res.ibm.com/":
        verify_ssl = True
    elif host == "https://cps.foc-deepsearch.zurich.ibm.com/":
        verify_ssl = False

    return host, proj, username, apikey, verify_ssl


def get_ds_api():
    """Obtain Deep Search API object"""

    tdir = get_scratch_dir()

    host, proj, username, apikey, verify_ssl = load_vars()

    config_ = {
        "host": host,
        "auth": {"username": username, "api_key": apikey},
        "verify_ssl": verify_ssl,
    }

    config_file = f"{tdir}/ds_config.json"
    with open(config_file, "w", encoding="utf-8") as fw:
        fw.write(json.dumps(config_))

    config = ds.DeepSearchConfig.parse_file(config_file)
    client = ds.CpsApiClient(config)

    api = ds.CpsApi(client)

    return api, proj


def process_zip_files(tdir):
    """Unzip all files obtained from Deep Search"""

    zipfiles = sorted(glob.glob(os.path.join(tdir, "*.zip")))
    # print(f"zips: ", len(zipfiles))

    for zipfile in zipfiles:
        cmd = ["unzip", zipfile, "-d", tdir]
        print(" ".join(cmd))

        subprocess.call(cmd)

    # clean up
    for i, zipfile in enumerate(zipfiles):
        print(i, "\t removing ", zipfile)
        subprocess.call(["rm", zipfile])

    """
    cellsfiles = sorted(glob.glob(os.path.join(tdir, "*.cells")))
    for i,cellsfile in enumerate(cellsfiles):
        subprocess.call(["rm", cellsfile])            
    """


def convert_pdffiles(pdf_files, force=False):
    """Convert PDF files to JSON"""

    for i, pdf_file in enumerate(pdf_files):
        old_file = pdf_file

        subprocess.call(["ls", f"{old_file}"])

        if " " in pdf_file:
            new_file = pdf_file.replace(" ", "_")
            subprocess.call(["mv", f"{old_file}", f"{new_file}"])
            pdf_files[i] = new_file

    json_files = []

    new_pdfs = []
    for pdf_file in pdf_files:
        if force:
            new_pdfs.append(pdf_file)
        else:
            json_file = pdf_file.replace(".pdf", ".json")
            if os.path.exists(json_file):
                json_files.append(json_file)
            else:
                new_pdfs.append(pdf_file)

    if len(new_pdfs) == 0:
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

    documents = ds.convert_documents(
        api=ds_api, proj_key=ds_proj, source_path=scratch_dir, progress_bar=True
    )

    documents.download_all(result_dir=scratch_dir)
    process_zip_files(scratch_dir)

    info = documents.generate_report(result_dir=scratch_dir)

    for new_pdf in new_pdfs:
        basename = os.path.basename(new_pdf).replace(".pdf", ".json")

        sfile = os.path.join(scratch_dir, basename)
        tfile = new_pdf.replace(".pdf", ".json")

        cmd = ["cp", f"{sfile}", f"{tfile}"]
        print(" ".join(cmd))

        # subprocess.call(cmd)
        subprocess.run(cmd, check=True)
        json_files.append(tfile)

    json_files = sorted(list(set(json_files)))
    return json_files


def ds_list_indices():
    """List all public Deep Search data-indices"""

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
    """Create a new documents directory"""

    tdir = get_scratch_dir()

    now = datetime.datetime.now()
    odir = now.strftime("document-collection-%Y-%m-%d_%H-%M-%S")

    odir = os.path.join(tdir, odir)
    return odir


def ds_index_query(
    index,
    query,
    odir=None,
    force=False,
    limit=-1,
    sources=[
        "file-info",
        "description",
        "main-text",
        "texts",
        "tables",
        "figures",
        "page-headers",
        "page-footers",
        "footnotes",
    ],  # Which fields of documents we want to fetch
):
    """Query Deep Search document index"""

    api, proj_key = get_ds_api()

    tdir = get_scratch_dir()

    if odir is None:
        dumpdir = hashlib.md5(f"{index}:{query}".encode()).hexdigest()
        dumpdir = os.path.join(tdir, dumpdir)
    else:
        dumpdir = odir

    if not os.path.exists(dumpdir):
        os.mkdir(dumpdir)
    elif not force:
        return dumpdir

    # Input query
    search_query = f"{query}"  # "\"global warming potential\" AND \"etching\""
    print(f"query: {query}")

    data_collection = ElasticDataCollectionSource(elastic_id="default", index_key=index)
    page_size = 50

    # Prepare the data query
    query = DataQuery(
        search_query,  # The search query to be executed
        source=sources,  # Which fields of documents we want to fetch
        limit=page_size,  # The size of each request page
        coordinates=data_collection,  # The data collection to be queries
    )

    # [Optional] Compute the number of total results matched. This can be used to monitor the pagination progress.
    count_query = copy.deepcopy(query)
    count_query.paginated_task.parameters["limit"] = 0
    count_results = api.queries.run(count_query)

    expected_total = count_results.outputs["data_count"]
    print(f"#-found documents: {expected_total}")

    if limit != -1 and expected_total > limit:
        expected_total = limit

    expected_pages = (
        expected_total + page_size - 1
    ) // page_size  # this is simply a ceiling formula

    # Iterate through all results by fetching `page_size` results at the same time
    bar_format = "{desc:<5.5}{percentage:3.0f}%|{bar:70}{r_bar}"

    count = 0

    all_results = []
    cursor = api.queries.run_paginated_query(query)
    for result_page in tqdm(cursor, total=expected_pages, bar_format=bar_format):
        for row in result_page.outputs["data_outputs"]:
            _id = row["_id"]
            with open(f"{dumpdir}/{_id}.json", "w", encoding="utf-8") as fw:
                fw.write(json.dumps(row["_source"], indent=2))

            count += 1
            if limit != -1 and count >= limit:
                break

        if limit != -1 and count >= limit:
            break

    return dumpdir


def resolve_item(paths, obj):
    """Find item in document from a reference path"""

    if len(paths) == 0:
        return obj

    if paths[0] == "#":
        return resolve_item(paths[1:], obj)

    try:
        key = int(paths[0])
    except:
        key = paths[0]

    if len(paths) == 1:
        if isinstance(key, str) and key in obj:
            return obj[key]
        elif isinstance(key, int) and key < len(obj):
            return obj[key]
        else:
            return None

    elif len(paths) > 1:
        if isinstance(key, str) and key in obj:
            return resolve_item(paths[1:], obj[key])
        elif isinstance(key, int) and key < len(obj):
            return resolve_item(paths[1:], obj[key])
        else:
            return None

    else:
        return None


def to_legacy_document_format(doc_glm, doc_leg={}):
    """Convert Document object (with `body`) to its legacy format (with `main-text`)"""

    doc_leg["main-text"] = []
    doc_leg["figures"] = []
    doc_leg["tables"] = []
    doc_leg["page-headers"] = []
    doc_leg["page-footers"] = []
    doc_leg["footnotes"] = []
    doc_leg["equations"] = []

    for pelem in doc_glm["page-elements"]:
        ptype = pelem["type"]
        span_i = pelem["span"][0]
        span_j = pelem["span"][1]

        if "iref" not in pelem:
            # print(json.dumps(pelem, indent=2))
            continue

        iref = pelem["iref"]

        if re.match("#/figures/(\\d+)/captions/(.+)", iref):
            # print(f"skip {iref}")
            continue

        if re.match("#/tables/(\\d+)/captions/(.+)", iref):
            # print(f"skip {iref}")
            continue

        path = iref.split("/")
        obj = resolve_item(path, doc_glm)

        if obj is None:
            print(f"warning: undefined {path}")
            continue

        if ptype == "figure":
            text = ""
            for caption in obj["captions"]:
                text += caption["text"]

                for nprov in caption["prov"]:
                    npaths = nprov["$ref"].split("/")
                    nelem = resolve_item(npaths, doc_glm)

                    if nelem is None:
                        print(f"warning: undefined caption {npaths}")
                        continue

                    span_i = nelem["span"][0]
                    span_j = nelem["span"][1]

                    text = caption["text"][span_i:span_j]

                    pitem = {
                        "text": text,
                        "name": nelem["name"],
                        "type": nelem["type"],
                        "prov": [
                            {
                                "bbox": nelem["bbox"],
                                "page": nelem["page"],
                                "span": [0, len(text)],
                            }
                        ],
                    }
                    doc_leg["main-text"].append(pitem)

            find = len(doc_leg["figures"])

            figure = {
                "confidence": obj.get("confidence", 0),
                "created_by": obj.get("created_by", ""),
                "type": obj.get("type", "figure"),
                "cells": [],
                "data": [],
                "text": text,
                "prov": [
                    {
                        "bbox": pelem["bbox"],
                        "page": pelem["page"],
                        "span": [0, len(text)],
                    }
                ],
            }
            doc_leg["figures"].append(figure)

            pitem = {
                "$ref": f"#/figures/{find}",
                "name": pelem["name"],
                "type": pelem["type"],
            }
            doc_leg["main-text"].append(pitem)

        elif ptype == "table":
            text = ""
            for caption in obj["captions"]:
                text += caption["text"]

                for nprov in caption["prov"]:
                    npaths = nprov["$ref"].split("/")
                    nelem = resolve_item(npaths, doc_glm)

                    if nelem is None:
                        print(f"warning: undefined caption {npaths}")
                        continue

                    span_i = nelem["span"][0]
                    span_j = nelem["span"][1]

                    text = caption["text"][span_i:span_j]

                    pitem = {
                        "text": text,
                        "name": nelem["name"],
                        "type": nelem["type"],
                        "prov": [
                            {
                                "bbox": nelem["bbox"],
                                "page": nelem["page"],
                                "span": [0, len(text)],
                            }
                        ],
                    }
                    doc_leg["main-text"].append(pitem)

            tind = len(doc_leg["tables"])

            table = {
                "#-cols": obj.get("#-cols", 0),
                "#-rows": obj.get("#-rows", 0),
                "confidence": obj.get("confidence", 0),
                "created_by": obj.get("created_by", ""),
                "type": obj.get("type", "table"),
                "cells": [],
                "data": obj["data"],
                "text": text,
                "prov": [
                    {"bbox": pelem["bbox"], "page": pelem["page"], "span": [0, 0]}
                ],
            }
            doc_leg["tables"].append(table)

            pitem = {
                "$ref": f"#/tables/{tind}",
                "name": pelem["name"],
                "type": pelem["type"],
            }
            doc_leg["main-text"].append(pitem)

        elif "text" in obj:
            text = obj["text"][span_i:span_j]

            pitem = {
                "text": text,
                "name": pelem["name"],
                "type": pelem["type"],
                "prov": [
                    {
                        "bbox": pelem["bbox"],
                        "page": pelem["page"],
                        "span": [0, len(text)],
                    }
                ],
            }
            doc_leg["main-text"].append(pitem)

        else:
            pitem = {
                "name": pelem["name"],
                "type": pelem["type"],
                "prov": [
                    {"bbox": pelem["bbox"], "page": pelem["page"], "span": [0, 0]}
                ],
            }
            doc_leg["main-text"].append(pitem)

    return doc_leg
