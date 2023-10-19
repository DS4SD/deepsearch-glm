
import os
import json

from deepsearch_glm.nlp_utils import list_nlp_model_configs, init_nlp_model
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models

def test_01_load_nlp_models():
    models = load_pretrained_nlp_models()
    print(f"models: {models}")

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

    model = init_nlp_model("sentence;language;term")
    res = model.apply_on_text("FeSe is a material.")

    for label in ["text", "properties", "instances", "relations"]:
        assert label in res        
        
def test_02B_run_nlp_models_on_text():

    filters = ["properties"]
    
    model = init_nlp_model("sentence;language;term")
    res = model.apply_on_text("FeSe is a material.", filters)

    for label in ["text", "properties"]:
        assert label in res

    for label in ["instances", "relations"]:
        assert label not in res        

def test_03A_run_nlp_models_on_document():

    with open("./data/documents/articles/1806.02284.json") as fr:
        doc = json.load(fr)
    
    model = init_nlp_model("sentence;language;term;reference")
    res = model.apply_on_doc(doc)
    print(res.keys())

    for label in ["description", "body", "meta",
                  "page-elements", "texts", "tables", "figures",
                  "properties", "instances", "relations"]:
        assert label in res

    check_dimensions(res["properties"])
    check_dimensions(res["instances"])
    check_dimensions(res["relations"])
    
def test_03B_run_nlp_models_on_document():

    with open("./data/documents/articles/1806.02284.json") as fr:
        doc = json.load(fr)

    filters = ["properties"]
        
    model = init_nlp_model("sentence;language;term;reference")
    res = model.apply_on_doc(doc, filters)
    print(res.keys())

    for label in ["dloc", "applied-models",
                  "description", "body", "meta",
                  "page-elements", "texts", "tables", "figures",
                  "properties"]: 
        assert label in res

    for label in ["instances", "relations"]:
        assert label not in res
                  
    check_dimensions(res["properties"])
    check_dimensions(res["instances"])
    check_dimensions(res["relations"])
