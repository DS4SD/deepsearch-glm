
import os
import json
import subprocess

from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models

ROOT_DIR=os.path.abspath("./")
BUILD_DIR=os.path.join(ROOT_DIR, "build")
RESOURCES_DIR=os.path.join(ROOT_DIR, "resources")

if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
    RESOURCES_DIR = os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR")
    
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

    cmds = [
        [f"cmake -B {BUILD_DIR}", ROOT_DIR],
        #["make install -j", BUILD_DIR]
        [f"cmake --build {BUILD_DIR} --target install -j", ROOT_DIR]
    ]

    for cmd in cmds:
        if not run(cmd[0], cwd=cmd[1]):
            break

"""        
def load(setup_kwargs=None):

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
        #print("executing: ", " ".join(cmd))
        
        if not os.path.exists(cmd[3]):
            print(f"downloading {os.path.basename(cmd[3])} ... ", end="")
            message = subprocess.run(cmd, cwd=ROOT_DIR)    
            print("done!")
        else:
            print(f" -> already downloaded {os.path.basename(cmd[3])}")
"""

if "__main__"==__name__:

    load_pretrained_nlp_models(False)
    
    build()


