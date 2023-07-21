
import os
import json
import subprocess

from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models

ROOT_DIR=os.path.abspath("./")
BUILD_DIR=os.path.join(ROOT_DIR, "build")

"""
RESOURCES_DIR=os.path.join(ROOT_DIR, "resources")
if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
    RESOURCES_DIR = os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR")
""" 
def run(cmd, cwd="./"):

    print(f"\nlaunch: {cmd}")
    
    parts = cmd.split(" ")
    message = subprocess.run(parts, cwd=cwd)    

    if "returncode=0" in str(message):
        print(f" -> SUCCESS")
        return True

    print(f" -> ERROR with message: '{message}'\n")        
    return False
    
def build(setup_kwargs=None):

    if not os.path.exists(BUILD_DIR):
        cmd = f"cmake -B {BUILD_DIR}"
        run(cmd, cwd=ROOT_DIR)
        
    cmd = f"cmake --build {BUILD_DIR} --target install -j"
    run(cmd, cwd=ROOT_DIR)    

def build_all_python_versions():

    python_versions = glob.glob("/usr/local/bin/python3.*")
    print(f"all found python-versions: {python_versions}")
    
    for pyv in python_versions:

        PYBUILD_DIR = os.path.join(ROOT_DIR, f"build-{pyv}")
        if not os.path.exists(PYBUILD_DIR):

            # cmake -B build-py310 -DPYTHON_EXECUTABLE=/usr/local/bin/python3.10
            cmd = f"cmake -B {PYBUILD_DIR} -DPYTHON_EXECUTABLE={pyv}"
            run(cmd, cwd=ROOT_DIR)
            
        cmd = f"cmake --build {PYBUILD_DIR} --target install -j"
        run(cmd, cwd=ROOT_DIR)    
    
if "__main__"==__name__:

    load_pretrained_nlp_models(False)
    
    build_all_python_versions()
    #build()


