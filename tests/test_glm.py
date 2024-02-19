#!/usr/bin/env python

GENERATE = False

import glob
import json
import os

from deepsearch_glm import andromeda_glm
from deepsearch_glm.glm_utils import (
    create_glm_config_from_docs,
    create_glm_dir,
    create_glm_from_docs,
    expand_terms,
    load_glm,
    read_edges_in_dataframe,
    read_nodes_in_dataframe,
    show_query_result,
)
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models


def get_dirs():
    sdir = "./tests/data/glm/test_01A"

    if GENERATE:
        rdir = os.path.join(sdir, "glm_ref")
        odir = os.path.join(sdir, "glm_ref")
    else:
        rdir = os.path.join(sdir, "glm_ref")
        odir = os.path.join(sdir, "glm_out")

    return sdir, rdir, odir


def test_01_load_nlp_models():
    """Tests to determine if NLP models are available"""

    models = load_pretrained_nlp_models()
    print(f"models: {models}")

    assert "language" in models
    assert "semantic" in models
    assert "name" in models
    assert "reference" in models


def test_02A_create_glm_from_doc():
    """Tests to determine if GLM creation work"""

    sdir, rdir, odir = get_dirs()

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


def test_02B_load_glm():
    """Tests to determine if GLM loading work"""

    # idir = "./tests/data/glm/test_01A/glm_out"

    sdir, rdir, odir = get_dirs()

    glm = load_glm(odir)
    out_topo = glm.get_topology()

    with open(os.path.join(odir, "topology.json")) as fr:
        ref_topo = json.load(fr)

    for i, row_i in enumerate(ref_topo["node-count"]["data"]):
        row_j = out_topo["node-count"]["data"][i]
        assert row_i == row_j

    for i, row_i in enumerate(ref_topo["edge-count"]["data"]):
        row_j = out_topo["edge-count"]["data"][i]
        assert row_i == row_j

    assert ref_topo == out_topo


def test_03A_query_glm():
    """Tests to determine if GLM queries work"""

    # idir = "./tests/data/glm/test_01A/glm_out"

    sdir, rdir, odir = get_dirs()

    nodes = read_nodes_in_dataframe(os.path.join(odir, "nodes.csv"))
    # edges = read_edges_in_dataframe(os.path.join(odir, "edges.csv"))

    """
    subw_nodes = nodes[ nodes["name"]=="subw_token"]
    print(subw_nodes)
    
    next_edge = edges[ (edges["hash_i"].isin(subw_nodes["hash"])) & (edges["hash_j"].isin(subw_nodes["hash"])) ]
    print(next_edge)
    #prev_edge = edges[ (edges["name"]=="prev") and (edges["hash_i"] in subw_nodes["hash"]) and (edges["hash_j"] in subw_nodes["hash"]) ]
    """

    glm = load_glm(odir)
    out_topo = glm.get_topology()

    terms = nodes[nodes["name"] == "term"]
    # print("terms: \n", terms)

    cnt = 0
    for i, row in terms.iterrows():
        res = expand_terms(glm, row["nodes-text"])
        show_query_result(res)

        cnt += 1
        if cnt >= 100:
            break
