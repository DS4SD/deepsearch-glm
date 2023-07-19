
import os
import json
import subprocess

def load_pretrained_nlp_models(force:bool=False):

    if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
        RESOURCES_DIR = os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR")
    else:
        # FIXME: not sure about this ...
        ROOT_DIR=os.path.abspath("./deepsearch_glm")
        RESOURCES_DIR=os.path.join(ROOT_DIR, "resources")
    
    with open(f"{RESOURCES_DIR}/models.json") as fr:
        models = json.load(fr)
        
    COS_URL = models["object-store"]
    COS_PRFX = models["nlp"]["prefix"]
    COS_PATH = os.path.join(COS_URL, COS_PRFX)
    
    cmds=[]
    for name,files in models["nlp"]["trained-models"].items():
        source = os.path.join(COS_PATH, files[0])
        target = os.path.join(RESOURCES_DIR, files[1])

        cmd = ["curl", source, "-o", target, "-s"]
        #print(" ".join(cmd))        
        cmds.append(cmd)
        
    for cmd in cmds:
        if force or (not os.path.exists(cmd[3])):
            #print(f"downloading {cmd[3]} ... ", end="")
            print(f"downloading {os.path.basename(cmd[3])} ... ", end="")            
            message = subprocess.run(cmd)#, cwd=ROOT_DIR)    
            print("done!")
        else:
            print(f" -> already downloaded {os.path.basename(cmd[3])}")
