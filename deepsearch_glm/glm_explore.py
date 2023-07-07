#!/usr/bin/env python

import os

import json
#import glob

import argparse
#import textwrap

#from tabulate import tabulate

from glm_utils import load_glm, explore_glm

def parse_arguments():

    parser = argparse.ArgumentParser(
        prog = 'glm_docqa',
        description = 'Do Q&A on pdf document',
        epilog = 'Text at the bottom of help')

    parser.add_argument('--glm-dir', required=True,
                        type=str,
                        help="directory with Deep Search GLM")

    args = parser.parse_args()

    return args.directory

if __name__ == '__main__':

    jsondir = parse_arguments()

    if not os.path.exists(jsondir):
        exit(-1)
    
    load_glm(jsondir)

