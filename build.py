

import os
import re

import json
import glob

import subprocess

from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models

ROOT_DIR=os.path.abspath("./")
BUILD_DIR=os.path.join(ROOT_DIR, "build")

def download_nlp_models():
    load_pretrained_nlp_models(False)

def run(cmd, cwd="./"):

    print(f"\nlaunch: {cmd}")
    
    parts = cmd.split(" ")
    message = subprocess.run(parts, cwd=cwd)    

    if "returncode=0" in str(message):
        print(f" -> SUCCESS")
        return True

    print(f" -> ERROR with message: '{message}'\n")        
    return False
    
def build_local(setup_kwargs=None):

    if not os.path.exists(BUILD_DIR):
        cmd = f"cmake -B {BUILD_DIR}"
        run(cmd, cwd=ROOT_DIR)
    else:
        print(f"build directory detected: {BUILD_DIR}")
        
    cmd = f"cmake --build {BUILD_DIR} --target install -j"
    run(cmd, cwd=ROOT_DIR)    

def build_all_python_versions():

    candidates = glob.glob("/usr/local/bin/python3.*")
    candidates += glob.glob("/usr/bin/python3.*")

    print(f"all candidates: {candidates}")
    
    python_versions=[]
    for candidate in candidates:
        pyname = os.path.basename(candidate)
        if re.match("^python3.\d+$", pyname):
            python_versions.append(candidate)

    python_versions = sorted(python_versions)
    print(f"all found python-versions: {python_versions}")
    
    for pyv in python_versions:

        pyn = os.path.basename(pyv)        
        PYBUILD_DIR = os.path.join(ROOT_DIR, f"build-{pyn}")
        if os.path.exists(PYBUILD_DIR):
            print(f"rm {PYBUILD_DIR}")
            continue
        
        cmd = f"cmake -B {PYBUILD_DIR} -DPYTHON_EXECUTABLE={pyv}"
        run(cmd, cwd=ROOT_DIR)
            
        cmd = f"cmake --build {PYBUILD_DIR} --target install -j"
        run(cmd, cwd=ROOT_DIR)    
    
if "__main__"==__name__:

    #load_pretrained_nlp_models(False)

    #build_local()
    build_all_python_versions()
