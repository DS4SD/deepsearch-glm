#!/usr/bin/env python

import os

import json
import glob

import argparse
import textwrap
import datetime

from tabulate import tabulate

from deepsearch_glm.glm_utils import create_glm_dir, create_glm_from_docs

import andromeda_glm

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'create_glm_from_docs',
        description = 'Create GLM from Deep Search documents',
        epilog =
"""
examples of execution: 

1.a run on single document (pdf or json) with default NLP models (=`term`):
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --pdf './data/documents/articles/2305.02334.pdf'
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --json './data/documents/articles/2305.02334.json'

2. run on multiple documents:
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --pdf './data/documents/articles/*.pdf'
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --json './data/documents/articles/*.json'

3. run on multiple documents with non-default models:
    poetry run python ./deepsearch_glm/create_glm_from_docs.py --pdf './data/documents/articles/2305.*.pdf' --models 'language;term;abbreviation'

""",
        formatter_class=argparse.RawTextHelpFormatter)
        
    parser.add_argument('--pdf', required=False,
                        type=str, default=None,
                        help="filename(s) of pdf document")
    
    parser.add_argument('--json', required=False,
                        type=str, default=None,
                        help="filename(s) of json document")

    parser.add_argument('--models', required=False,
                        type=str, default="name;verb;term;abbreviation",
                        help="set NLP models (e.g. `term;sentence`)")
    
    parser.add_argument('--force-convert', required=False,
                        type=bool, default=False,
                        help="force pdf conversion")

    parser.add_argument('--output-dir', required=False,
                        type=str, default=create_glm_dir(),
                        help="output root directory for GLM")    

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
        
    return pdf_files, json_files, args.models, args.force_convert, args.output_dir
    
if __name__ == '__main__':

    pdf_files, json_files, model_names, force_convert, odir = parse_arguments()

    if len(pdf_files)>0:
        new_json_files = convert_pdffiles(pdf_files, force=force_convert)

        for _ in new_json_files:
            json_files.append(_)

    json_files = sorted(list(set(json_files)))        

    #odir = create_glm_dir()
    
    glm = create_glm_from_docs(odir, json_files, model_names)
    print(f" --> GLM saved to: {odir}")

    
