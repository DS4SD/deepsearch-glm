#!/usr/bin/env python

import json
import os
import subprocess

# from deepsearch_glm.andromeda_nlp import nlp_model


def get_resources_dir():
    if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
        resources_dir = os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR")
    else:
        from deepsearch_glm.andromeda_nlp import nlp_model

        model = nlp_model()
        resources_dir = model.get_resources_path()

    return resources_dir


def load_pretrained_nlp_data(key: str, force: bool = False, verbose: bool = False):
    """
    if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
        RESOURCES_DIR = str(os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR"))
    else:
        from deepsearch_glm.andromeda_nlp import nlp_model

        model = nlp_model()
        RESOURCES_DIR = model.get_resources_path()
    """

    resources_dir = get_resources_dir()

    with open(f"{resources_dir}/data_nlp.json") as fr:
        nlp_data = json.load(fr)

    cos_url = nlp_data["object-store"]
    cos_prfx = nlp_data["nlp"]["prefix"]
    cos_path = os.path.join(cos_url, cos_prfx)

    cmds = {}
    for name, files in nlp_data["nlp"][key].items():
        source = os.path.join(cos_path, files[0])
        target = os.path.join(resources_dir, files[1])

        cmd = ["curl", source, "-o", target, "-s"]
        cmds[name] = cmd

    done = True
    data = {}

    for name, cmd in cmds.items():
        print(f"{name}: {cmd}")

        data_file = cmd[3]

        if force or (not os.path.exists(data_file)):
            if verbose:
                print(f"downloading {name} ... ", end="")

            message = subprocess.run(cmd)
            print(message)

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
    """
    if "DEEPSEARCH_GLM_RESOURCES_DIR" in os.environ:
        RESOURCES_DIR = str(os.getenv("DEEPSEARCH_GLM_RESOURCES_DIR"))
    else:
        from deepsearch_glm.andromeda_nlp import nlp_model

        model = nlp_model()
        RESOURCES_DIR = model.get_resources_path()
    """

    resources_dir = get_resources_dir()

    with open(f"{resources_dir}/models.json") as fr:
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

            message = subprocess.run(cmd)

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
