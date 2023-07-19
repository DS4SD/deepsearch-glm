#!/usr/bin/env python

import os
import json
import glob

import argparse
import textwrap

#import pandas as pd

#from tabulate import tabulate
#from ds_utils import convert_pdffiles

import andromeda_nlp

if __name__ == '__main__':

    mdl = andromeda_nlp.nlp_model()
    print("resource-dir: ", mdl.get_resources_path())
