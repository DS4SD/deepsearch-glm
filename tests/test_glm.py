#!/usr/bin/env python

GENERATE=False

import os

import json
import glob

from deepsearch_glm import andromeda_glm

from deepsearch_glm.glm_utils import create_glm_dir, create_glm_from_docs

def test_01A_create_glm_from_doc():

    sdir = "./tests/data/glm/test_01A"
    
    if GENERATE:
        rdir = os.path.join(sdir, "glm_ref")
        odir = os.path.join(sdir, "glm_ref")
    else:
        rdir = os.path.join(sdir, "glm_ref")
        odir = os.path.join(sdir, "glm_out")
        
    model_names = "semantic;name;verb;term;abbreviation"
    
    json_files = glob.glob(os.path.join(sdir, "docs/*.json"))
    
    glm = create_glm_from_docs(odir, json_files, model_names)

    with open(os.path.join(rdir, "topology.json")) as fr:
        ref_topo = json.load(fr)

    with open(os.path.join(odir, "topology.json")) as fr:
        out_topo = json.load(fr)        

    for i,row_i in enumerate(ref_topo["node-count"]["data"]):
        row_j = out_topo["node-count"]["data"][i]
        assert row_i==row_j

    for i,row_i in enumerate(ref_topo["edge-count"]["data"]):
        row_j = out_topo["edge-count"]["data"][i]
        assert row_i==row_j        
        
    #assert ref_topo==out_topo

"""    
def test_01A_load_glm():

    config = {
        "IO": {
	    "load": {
                "root": path
            }
        }
    }
    
    glm_model = andromeda_glm.glm_model()
    glm_model.load(config)

    assert True
"""
