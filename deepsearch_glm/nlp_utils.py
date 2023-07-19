
import os

import json
import glob

import argparse
import textwrap
import datetime
import subprocess

from utils.ds_utils import get_scratch_dir

import andromeda_nlp

def create_nlp_dir():

    tdir = get_scratch_dir()

    now = datetime.datetime.now()
    nlpdir = now.strftime("NLP-model-%Y-%m-%d_%H-%M-%S")

    odir = os.path.join(tdir, nlpdir)
    return odir

def list_nlp_model_configs():

    configs = []

    configs += nlp_model.get_apply_configs()
    configs += nlp_model.get_train_configs()

    return configs

def init_nlp_model(model_names:str="language;term"):

    nlp_model = andromeda_nlp.nlp_model()

    configs = nlp_model.get_apply_configs()
    print(json.dumps(configs, indent=2))
    
    config = nlp_model.get_apply_configs()[0]
    config["models"] = model_names

    nlp_model.initialise(config)
    
    return nlp_model
