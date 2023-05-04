#!/usr/bin/env python

import os

import json
import glob

import argparse
import requests

import andromeda_nlp
import andromeda_glm

from tabulate import tabulate

URL = "http://127.0.0.1:8000"

def load_glms():

    config = {
        "IO": {
	    "load": {
                "root": "../../build/glm-model-small"
            }
        }
    }

    for name in ["glm-model-arxiv-big-v1"]:

        config["IO"]["load"]["root"] = f"../../build/{name}"        
        print(config)
        
        response = requests.post(f"{URL}/load/GLM/{name}", json=config)
        print(" --> code: ", response.status_code)

    response = requests.get(f"{URL}/list/GLM")
    glm_names = response.json()
    
    print(f"GLM's: {glm_names}")
        
def query_glms():

    response = requests.get(f"{URL}/list/GLM")
    glm_names = response.json()
    
    print(f"GLM's: {glm_names}")
    glm_name = glm_names[-1]
    
    qry = andromeda_glm.glm_query()
    qry.select({"nodes":[["superconductor"], ["superconductors"]]})
    qry.filter_by({"mode": "node-flavor", "node-flavors":["term"]})
    #qry.traverse({"edge":"to-singular"})
    #qry.traverse({"edge":"to-path"})
    #qry.traverse({"edge":"from-path"})
    #qry.traverse({"edge":"to-singular"})
    qry.traverse({"edge":"tax-up"})
    #qry.traverse({"edge":"tax-up"})
    qry.traverse({"edge":"to-singular"})    
    #qry.subgraph({"edges":["tax-up"]})
    
    config = qry.to_config()    
    print("query: ", json.dumps(config, indent=2))    

    print(f"{URL}/query/GLM/{glm_name}")
    
    response = requests.post(f"{URL}/query/GLM/{glm_name}", json=config)
    result = response.json()

    print_query(result)

def interactive():

    response = requests.get(f"{URL}/list/GLM")
    glm_names = response.json()
    
    print(f"GLM's: {glm_names}")
    glm_name = glm_names[-1]
    
    while True:

        line = input("traversal: ")
        if line=="quit":
            break

        parts = line.split("->")

        qry = andromeda_glm.glm_query()
        for i,part in enumerate(parts):

            name=part.strip()            
            if i==0:
                qry.select({"nodes":[[name]]})
            else:
                qry.traverse({"edge":name})

        response = requests.post(f"{URL}/query/GLM/{glm_name}", json=qry.to_config())
        result = response.json()

        print_query(result)                
                
def print_query(result, max_nodes=16):

    #print(json.dumps(result, indent=2))
    
    print(tabulate(result["overview"]["data"], headers=result["overview"]["headers"]))
    for i,item in enumerate(result["result"]):

        num_nodes = len(item["nodes"]["data"])
        print(f" -> result {i}: {num_nodes}")
        num_nodes = min(max_nodes, num_nodes)

        print(tabulate(item["nodes"]["data"][0:num_nodes], headers=item["nodes"]["headers"]))    

def find_units():

    response = requests.get(f"{URL}/list/GLM")
    glm_names = response.json()
    
    print(f"GLM's: {glm_names}")
    glm_name = glm_names[-1]
    
    qry = andromeda_glm.glm_query()
    
    qry.select({"nodes":[["__ival__"], ["__fval__"], ["__fexp__"], ["__irng__"], ["__frng__"], ["__fsci__"]]})
    #qry.select({"nodes":[["__fexp__"]]})
    qry.filter_by({"mode": "node-flavor", "node-flavors":["token"]})
    qry.traverse({"edge":"next"})
    qry.filter_by({"mode": "node-flavor", "node-flavors":["token"]})
    #qry.subgraph({"edges":["to-pos"]})
    
    config = qry.to_config()    
    #print("query: ", json.dumps(config, indent=2))        

    response = requests.post(f"{URL}/query/GLM/{glm_name}", json=config)
    result_i = response.json()

    print_query(result_i, 64)
    
    num_nodes = 32

    nodes = result_i["result"][-1]["nodes"]
    
    headers = nodes["headers"]
    data = nodes["data"]

    print(tabulate(data[0:num_nodes], headers=headers))

    weights=[]
    for j,row in enumerate(data):

        text_j = row[headers.index("text")]
        print(j, " => ", text_j)
        
        qry = andromeda_glm.glm_query()
    
        qry.select({"nodes":[[text_j]]})
        flid_0 = qry.get_last_flid()

        qry.traverse({"edge":"prev"})
        qry.filter_by({"mode": "node-flavor", "node-flavors":["token"]})
        qry.get_last_flid()

        qry.traverse({"edge":"to-pos", "source":flid_0})
        
        response = requests.post(f"{URL}/query/GLM/{glm_name}", json=qry.to_config())
        result_j = response.json()

        #print(result_j["result"][-1].keys())
        nodes_l = result_j["result"][0]["nodes"]
        nodes_j = result_j["result"][2]["nodes"]
        nodes_k = result_j["result"][3]["nodes"]

        headers_l = nodes_l["headers"]
        data_l = nodes_l["data"]
        
        headers_j = nodes_j["headers"]
        data_j = nodes_j["data"]

        headers_k = nodes_k["headers"]
        data_k = nodes_k["data"]

        #print(tabulate(data_k, headers=headers_k))
        print_query(result_j)

        weight=0.0
        for k,row in enumerate(data_j):
            if row[headers.index("text")] in ["__ival__", "__fval__", "__irng__", "__frng__", "__fsci__", "__fexp__"]:
                weight += row[headers.index("weight")]

        hoverview = result_j["overview"]["headers"]
        hdata = result_j["overview"]["data"]

        #print(tabulate(hdata, headers=hoverview))
        #print(hoverview.index("#-nodes"))
        
        if len(data_k)>0:
            pos = data_k[0][headers_k.index("text")]        
            weights.append([weight, data_l[0][headers_l.index("count")], text_j, pos, hdata[-1][hoverview.index("#-nodes")]])

    weights = sorted(weights, key=lambda x:-x[0])
    print(tabulate(weights, headers=["weights", "#-occurence", "text", "POS", "#-pos"]))
        
if __name__ == '__main__':

    load_glms()

    #interactive()    
    #query_glms()
    find_units()

