
from ds_utils import get_scratch_dir

#import andromeda_nlp
import andromeda_glm

def explore_glm_config(glmdir:str):

    config = {
        "IO": {
            "load": {
                "root": glmdir
            }
        },
        "mode": "explore"
    }

    return config


def create_glm_config(jsondir:str, models:str="conn;verb;term;abbreviation"):
    
    scratch_dir = get_scratch_dir()
    
    config = {
        "IO": {
            "load": {
                "root": f"{jsondir}-glm-model"
            },
            "save": {
                "root": f"{jsondir}-glm-model",
                "write-CSV": False,
                "write-JSON": False,
                "write-path-text": False
            }
        },
        "create": {
            "enforce-max-size": False,
            "model": {
                "max-edges": 1e8,
                "max-nodes": 1e7
            },
            "number-of-threads": 4,
            "worker": {
                "local-reading-break": True,
                "local-reading-range": [
                    256,
                    2560
                ],
                "max-edges": 1e7,
                "max-nodes": 1e6
            },
            "write-nlp-output": False
        },
        "mode": "create",
        "parameters": {
            "glm-padding": 1,
            "glm-paths": {
                "keep-concatenation": True,
                "keep-connectors": True,

                "keep-terms": True,
                "keep-verbs": True,
                
                "keep-sentences": True,
                
                "keep-tables": True,
                "keep-texts": True,

                "keep-docs": True,
            },
            "nlp-models": "conn;verb;term;abbreviation"
        },
        "producers": [
            {
                "input-format": "json",
                "input-paths": [
                    jsondir
                ],
                "keep-figures": True,
                "keep-tables": True,
                "keep-text": True,

                "order-text": True,

                "output": False,
                "output-format": "nlp.json",

                "subject-type": "DOCUMENT"
            }
        ]
    }

    return config

def create_glm(jsondir:str, models:str="conn;verb;term;abbreviation"):

    config = create_glm_config(jsondir)
    
    glm = andromeda_glm.glm_model()
    glm.create(config)

    return glm    
