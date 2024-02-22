#!/usr/bin/env python
"""Module to load binary files of models and data"""


import json
import os
import subprocess


def get_resources_dir():
    """Function to obtain the resources-directory"""

    if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
        resources_dir = os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR")
    else:
        from deepsearch_glm.andromeda_nlp import nlp_model

        model = nlp_model()
        resources_dir = model.get_resources_path()

    return resources_dir


def list_training_data(key: str, force: bool = False, verbose: bool = False):
    """Function to list the training data"""

    return []


def load_training_data(
    data_type: str, data_name: str, force: bool = False, verbose: bool = False
):
    """Function to load data to train NLP models"""

    assert data_type in ["text", "crf"]

    resources_dir = get_resources_dir()
    with open(f"{resources_dir}/data.json", "r", encoding="utf-8") as fr:
        training_data = json.load(fr)

    cos_url = training_data["object-store"]
    cos_prfx = training_data["data"]["prefix"]
    cos_path = os.path.join(cos_url, cos_prfx)

    cmds = {}
    for name, files in training_data["data"][data_type].items():
        print(name)
        if name == data_name:
            source = os.path.join(cos_path, files[0])
            target = os.path.join(resources_dir, files[1])

            cmd = ["curl", source, "-o", target, "-s"]
            cmds[name] = cmd

    done = True
    data = {}

    for name, cmd in cmds.items():
        data_file = cmd[3]
        print(data_file)

        if force or (not os.path.exists(data_file)):
            if verbose:
                print(f" -> downloading {name} ... ", end="")

            subprocess.run(cmd, check=True)

            if verbose:
                print("done!")

            data[name] = data_file

        elif os.path.exists(data_file):
            if verbose:
                print(f" -> already downloaded {name}")

            data[name] = data_file
        else:
            print(f" -> missing {name}")

    return done, data


def load_pretrained_nlp_models(force: bool = False, verbose: bool = False):
    """Function to load pretrained NLP models"""

    resources_dir = get_resources_dir()

    with open(f"{resources_dir}/models.json", "r", encoding="utf-8") as fr:
        models = json.load(fr)

    cos_url = models["object-store"]
    cos_prfx = models["nlp"]["prefix"]
    cos_path = os.path.join(cos_url, cos_prfx)

    cmds = {}
    for name, files in models["nlp"]["trained-models"].items():
        source = os.path.join(cos_path, files[0])
        target = os.path.join(resources_dir, files[1])

        cmd = ["curl", source, "-o", target, "-s"]
        cmds[name] = cmd

    models = []

    for name, cmd in cmds.items():
        model_weights = cmd[3]

        if force or (not os.path.exists(model_weights)):
            if verbose:
                # print(f"downloading {os.path.basename(model_weights)} ... ", end="")
                print(f"downloading {name} ... ", end="")

            message = subprocess.run(cmd, check=True)

            if verbose:
                print("done!")
            models.append(name)

        elif os.path.exists(model_weights):
            if verbose:
                # print(f" -> already downloaded {os.path.basename(cmd[3])}")
                print(f" -> already downloaded {name}")
            models.append(name)

        else:
            print(f" -> missing {name}")

    return models
