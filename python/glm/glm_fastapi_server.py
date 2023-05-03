#!/usr/bin/env python

import os

import json
import glob

import argparse

import andromeda_nlp
import andromeda_glm

from typing import Optional

from fastapi import FastAPI
from pydantic import BaseModel

glm_models = {}
nlp_models = {} 

RESOURCE_DIR="../../resources/"

app = FastAPI()

@app.get("/list/NLP")
async def list_nlp():
    names = []
    for k,v in nlp_models.items():
        names.append(k)
    return names

@app.get("/list/GLM")
async def list_glm():
    names = []
    for k,v in glm_models.items():
        names.append(k)
    return names

@app.post("/load/NLP/{nlp_model_name}")
async def load_nlp(nlp_model_name:str, config:dict):
    return {}

@app.post("/load/GLM/{glm_model_name}")
async def load_glm(glm_model_name:str, config:dict):
    
    if glm_model_name not in glm_models:

        glm_models[glm_model_name] = andromeda_glm.glm_model()
        glm_models[glm_model_name].set_resource_dir(RESOURCE_DIR)
        glm_models[glm_model_name].load(config)
        
    return glm_models[glm_model_name].get_topology()

@app.post("/query/NLP/{nlp_model_name}")
async def query(nlp_model_name:str, qry:dict):
    return qry

@app.post("/query/GLM/{glm_model_name}")
async def query(glm_model_name:str, qry:dict):

    if glm_model_name in glm_models:
        result = glm_models[glm_model_name].query(qry)
    else:
        result = {"status":False, "error":f"{glm_model_name} is not a known model."}
        
    return result
