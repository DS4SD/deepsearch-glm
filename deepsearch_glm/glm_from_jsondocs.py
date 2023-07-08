#!/usr/bin/env python

import os

import json
#import glob

import argparse
#import textwrap

#from tabulate import tabulate

from glm_utils import create_glm

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'glm_docqa',
        description = 'Do Q&A on pdf document',
        epilog = 'Text at the bottom of help')

    parser.add_argument('--directory', required=True,
                        type=str,
                        help="directory with Deep Search documents in JSON format")

    parser.add_argument('--models', required=False,
                        type=str, default="name;verb;term;abbreviation",
                        help="set NLP models (e.g. `term;sentence`)")
    
    args = parser.parse_args()

    return args.directory, args.models

if __name__ == '__main__':

    jsondir, models = parse_arguments()

    if not os.path.exists(jsondir):
        exit(-1)
    
    glm = create_glm(jsondir, models)

