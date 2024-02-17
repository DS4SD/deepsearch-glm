#!/usr/bin/env python

GENERATE = False

import glob
import json
import os

from deepsearch_glm import andromeda_glm
from deepsearch_glm.glm_utils import create_glm_dir, \
    create_glm_config_from_docs,\
    create_glm_from_docs, \
    load_glm
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models


def test_01_load_nlp_models():
    print("\nstarting ...")

    models = load_pretrained_nlp_models()
    print(f"models: {models}")

    assert "language" in models
    assert "semantic" in models
    assert "name" in models
    assert "reference" in models

    print("\n --> done test_01_load_nlp_models: ", end="")


def test_02A_create_glm_from_doc():
    print("\nstarting ...")

    sdir = "./tests/data/glm/test_01A"

    if GENERATE:
        rdir = os.path.join(sdir, "glm_ref")
        odir = os.path.join(sdir, "glm_ref")
    else:
        rdir = os.path.join(sdir, "glm_ref")
        odir = os.path.join(sdir, "glm_out")

    model_names = "spm;semantic;name;conn;verb;term;abbreviation"
    
    json_files = glob.glob(os.path.join(sdir, "docs/*.json"))

    config = create_glm_config_from_docs(odir, json_files, model_names)
    print(json.dumps(config, indent=2))
    
    glm = create_glm_from_docs(odir, json_files, model_names)

    with open(os.path.join(rdir, "topology.json")) as fr:
        ref_topo = json.load(fr)

    with open(os.path.join(odir, "topology.json")) as fr:
        out_topo = json.load(fr)

    for i, row_i in enumerate(ref_topo["node-count"]["data"]):
        row_j = out_topo["node-count"]["data"][i]
        # assert row_i == row_j
        if row_i != row_j:
            print(row_i, " != ", row_j)

    for i, row_i in enumerate(ref_topo["edge-count"]["data"]):
        row_j = out_topo["edge-count"]["data"][i]
        # assert row_i == row_j
        if row_i != row_j:
            print(row_i, " != ", row_j)

    assert ref_topo == out_topo

    print("\n --> done test_02A_create_glm_from_doc: ", end="")


def test_02B_load_glm():
    print("\nstarting ...")

    idir = "./tests/data/glm/test_01A/glm_out"

    glm = load_glm(idir)
    out_topo = glm.get_topology()

    with open(os.path.join(idir, "topology.json")) as fr:
        ref_topo = json.load(fr)

    for i, row_i in enumerate(ref_topo["node-count"]["data"]):
        row_j = out_topo["node-count"]["data"][i]
        assert row_i == row_j

    for i, row_i in enumerate(ref_topo["edge-count"]["data"]):
        row_j = out_topo["edge-count"]["data"][i]
        assert row_i == row_j

    assert ref_topo == out_topo

    print("\n --> done test_02B_load_glm: ", end="")
