
import os

import json
import glob

import argparse
import textwrap
import datetime
import subprocess

#from tabulate import tabulate

from utils.ds_utils import get_scratch_dir

import andromeda_nlp

def load_models():

    if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
        RESOURCES_DIR = os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR")
    else:
        # FIXME: not sure about this ...
        ROOT_DIR=os.path.abspath("./")
        RESOURCES_DIR=os.path.join(ROOT_DIR, "resources")
    
    with open(f"{RESOURCES_DIR}/models.json") as fr:
        models = json.load(fr)
        
    COS_URL = models["object-store"]

    cmds=[]
    for name,files in models["trained-models"].items():
        source = os.path.join(COS_URL, files[0])
        target = os.path.join(RESOURCES_DIR, files[1])
        
        cmds.append(["curl", source, "-o", target, "-s"])
        #print(" ".join(cmds[-1]))
        
    for cmd in cmds:
        if not os.path.exists(cmd[3]):
            print(f"downloading {os.path.basename(cmd[3])} ... ", end="")
            message = subprocess.run(cmd, cwd=ROOT_DIR)    
            print("done!")
        else:
            print(f" -> already downloaded {os.path.basename(cmd[3])}")
    

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

    config = nlp_model.get_apply_configs()[0]
    config["models"] = model_names

    nlp_model.initialise(config)
    
    return nlp_model
