#!/usr/bin/env python

import os
import json

from deepsearch_glm.nlp_utils import list_nlp_model_configs, init_nlp_model, \
    extract_references_from_doc
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models

from deepsearch_glm.nlp_train_semantic import train_semantic

GENERATE=False

def test_01_load_nlp_models():
    models = load_pretrained_nlp_models()
    #print(f"models: {models}")

    assert "language" in models
    assert "semantic" in models
    assert "name" in models
    assert "reference" in models

def check_dimensions(item):

    assert "headers" in item
    assert "data" in item    

    headers = item["headers"]
    for row in item["data"]:
        assert len(row)==len(headers)

def test_02A_run_nlp_models_on_text():

    source = "./tests/data/texts/test_02A_text_01.jsonl"
    target = source #"./tests/data/texts/test_02A_text_01.nlp.jsonl"
    
    model = init_nlp_model("sentence;language;term")
    sres = model.apply_on_text("FeSe is a material.")

    if GENERATE: # generate the test-data

        fw = open(source, "w")
        fw.write(json.dumps(sres)+"\n")            
        fw.close()        

        assert True

    else:

        with open(target) as fr:
            tres = json.load(fr)
        
        for label in ["properties", "instances"]:
            check_dimensions(sres[label])
            assert label in sres
                      
        for label in ["relations"]:
            assert label not in sres

        assert tres==sres
            
def test_02B_run_nlp_models_on_text():

    source = "./tests/data/texts/test_02B_text_02.jsonl"
    target = source #"./tests/data/texts/test_02B_text_02.nlp.jsonl"
    
    filters = ["properties"]
    
    model = init_nlp_model("sentence;language;term", filters)
    sres = model.apply_on_text("FeSe is a material.")

    if GENERATE: # generate the test-data

        fw = open(source, "w")
        fw.write(json.dumps(sres)+"\n")            
        fw.close()        

        assert True

    else:

        with open(target) as fr:
            tres = json.load(fr)    

        for label in ["text", "properties"]:
            assert label in sres

        for label in ["instances", "relations"]:
            assert label not in sres

        assert tres==sres            

def test_03A_run_nlp_models_on_document():

    with open("./tests/data/docs/1806.02284.json") as fr:
        doc = json.load(fr)
    
    model = init_nlp_model("sentence;language;term;reference;abbreviation")
    res = model.apply_on_doc(doc)
    #print(res.keys())

    for label in ["description", "body", "meta",
                  "page-elements", "texts", "tables", "figures",
                  "properties", "instances", "relations"]:
        assert label in res

    check_dimensions(res["properties"])
    check_dimensions(res["instances"])
    check_dimensions(res["relations"])
    
def test_03B_run_nlp_models_on_document():

    with open("./tests/data/docs/1806.02284.json") as fr:
        doc = json.load(fr)

    filters = ["applied-models", "properties"]
        
    model = init_nlp_model("sentence;language;term;reference", filters)
    res = model.apply_on_doc(doc)
    #print(res.keys())

    for label in ["dloc", "applied-models",
                  "description", "body", "meta",
                  "page-elements", "texts", "tables", "figures",
                  "properties"]: 
        assert label in res

    for label in ["instances", "relations"]:
        assert label not in res
                  
    check_dimensions(res["properties"])

def test_03C_run_nlp_models_on_document():

    model = init_nlp_model("language;semantic;sentence;term;verb;conn;geoloc;reference")

    source = "./tests/data/docs/1806.02284.json"
    target = "./tests/data/docs/1806.02284.nlp.json"
    
    if GENERATE: # generate the test-data
        with open(source) as fr:
            doc = json.load(fr)

        res = model.apply_on_doc(doc)
        extract_references_from_doc(res)
        
        fw = open(target, "w")
        fw.write(json.dumps(res)+"\n")            
        fw.close()
        
        assert True
        
    else:
        with open(source) as fr:
            sdoc = json.load(fr)

        res = model.apply_on_doc(sdoc)        

        with open(target) as fr:
            tdoc = json.load(fr)
        
        assert res==tdoc

def test_04A_terms():

    source = "./tests/data/texts/terms.jsonl"
    target = "./tests/data/texts/terms.nlp.jsonl"
    
    model = init_nlp_model("language;semantic;sentence;term;verb;conn;geoloc")

    if GENERATE: # generate the test-data
        with open(source) as fr:
            lines = fr.readlines()

        fw = open(target, "w")
    
        for line in lines:
            data = json.loads(line)
            res = model.apply_on_text(data["text"])

            fw.write(json.dumps(res)+"\n")
            
        fw.close()

    else:
        with open(target) as fr:
            lines = fr.readlines()
            
        for line in lines:
            data = json.loads(line)
            res = model.apply_on_text(data["text"])

            for i,row_i in enumerate(res["properties"]["data"]):
                row_j = data["properties"]["data"][i]
                #print(i, "\t", row_i)
                #print(i, "\t", row_j)
                assert row_i==row_j

            for i,row_i in enumerate(res["instances"]["data"]):
                row_j = data["instances"]["data"][i]
                #print(i, "\t", row_i)
                #print(i, "\t", row_j)
                assert row_i==row_j
                
            assert res==data
       
    assert True

def test_04B_semantic():

    model = init_nlp_model("semantic")

    source = "./tests/data/texts/semantics.jsonl"
    target = "./tests/data/texts/semantics.nlp.jsonl"
    
    if GENERATE: # generate the test-data
        
        with open(source) as fr:
            lines = fr.readlines()

        fw = open(target, "w")
    
        for line in lines:
            data = json.loads(line)
            res = model.apply_on_text(data["text"])
            
            fw.write(json.dumps(res)+"\n")
                
        fw.close()
        assert True
        
    else:
        with open(target) as fr:
            lines = fr.readlines()
            
        for line in lines:
            data = json.loads(line)
            res = model.apply_on_text(data["text"])

            for i,row_i in enumerate(res["properties"]["data"]):
                row_j = data["properties"]["data"][i]
                assert row_i==row_j

            assert res==data

def test_04C_references():

    model = init_nlp_model("reference")

    source = "./tests/data/texts/references.jsonl"
    target = "./tests/data/texts/references.nlp.jsonl"
    
    if GENERATE: # generate the test-data
        
        with open(source) as fr:
            lines = fr.readlines()

        fw = open(target, "w")
    
        for line in lines:
            data = json.loads(line)
            res = model.apply_on_text(data["text"])
            
            fw.write(json.dumps(res)+"\n")
                
        fw.close()
        assert True
        
    else:
        with open(target) as fr:
            lines = fr.readlines()
            
        for line in lines:
            data = json.loads(line)
            res = model.apply_on_text(data["text"])

            assert res==data

"""
def test_05A_train_semantic():

    train_semantic("prepare", "./tests/data/train/semantic", autotune=True, duration=60, modelsize="1M")

    train_semantic("train", "./tests/data/train/semantic", autotune=True, duration=60, modelsize="1M")
"""
