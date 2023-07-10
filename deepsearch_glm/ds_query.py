#!/usr/bin/env python

import os

import json
import glob

import argparse

import pandas as pd

from ds_utils import ds_list_indices, ds_index_query

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'ds_query',
        description = 'Query and download documents on the Deep Search platform',
        epilog =
"""
examples of execution: 

1. search for documents:

    poetry run python ./deepsearch_glm/ds_query.py --index esg-reports --query "\"net zero\""
    poetry run python ./deepsearch_glm/ds_query.py --index patent-uspto --query "\"global warming potential\" AND \"etching\""
    poetry run python ./deepsearch_glm/ds_query.py --index arxiv --query "\"quantum computing\""
""",
        formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--index', required=True,
                        type=str,
                        help="Deep Search document index")

    parser.add_argument('--query', required=True,
                        type=str, 
                        help="Query for document")

    args = parser.parse_args()

    return args.index, args.query

if __name__ == '__main__':

    index, query = parse_arguments()

    indices = ds_list_indices()

    found=False
    for item in indices:
        if item["Index"]==index:
            found=True

    if not found:
        print(pd.DataFrame(indices))
        print(f"index `{index}` not found")
        exit(-1)

    else:
        odir = ds_index_query(index, query, force=True)        
        print(f"files downloaded: {odir}")
