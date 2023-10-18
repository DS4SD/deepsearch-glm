
import os
import json
import subprocess

def load_pretrained_nlp_models(force:bool=False, verbose:bool=False):

    if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
        RESOURCES_DIR = os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR")
    else:
        from deepsearch_glm.andromeda_nlp import nlp_model

        model = nlp_model()
        RESOURCES_DIR = model.get_resources_path()
    
    with open(f"{RESOURCES_DIR}/models.json") as fr:
        models = json.load(fr)

    print(models)
        
    COS_URL = models["object-store"]
    COS_PRFX = models["nlp"]["prefix"]
    COS_PATH = os.path.join(COS_URL, COS_PRFX)
    
    cmds={}
    for name,files in models["nlp"]["trained-models"].items():
        source = os.path.join(COS_PATH, files[0])
        target = os.path.join(RESOURCES_DIR, files[1])

        cmd = ["curl", source, "-o", target, "-s"]
        cmds[name] = cmd

    models=[]
        
    for name,cmd in cmds.items():

        model_weights = cmd[3]
        
        if force or (not os.path.exists(model_weights)):

            if verbose:
                #print(f"downloading {os.path.basename(model_weights)} ... ", end="")
                print(f"downloading {name} ... ", end="")            

            message = subprocess.run(cmd)    

            if verbose:
                print("done!")
            models.append(name)

        elif os.path.exists(model_weights):
            if verbose:
                #print(f" -> already downloaded {os.path.basename(cmd[3])}")
                print(f" -> already downloaded {name}")
            models.append(name)
            
        else:
            print(f" -> missing {name}")
            
    return models
