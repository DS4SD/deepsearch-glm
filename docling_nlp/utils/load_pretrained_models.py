#!/usr/bin/env python
"""Module to load binary files of models and data"""

import json
import os
import shutil
from pathlib import Path
from typing import Dict, Tuple

import requests
from huggingface_hub import hf_hub_download


def get_resources_dir():
    """Function to obtain the resources directory"""

    if "DOCLING_NLP_RESOURCES_DIR" in os.environ:
        resources_dir = os.getenv("DOCLING_NLP_RESOURCES_DIR")
    else:
        from docling_nlp.andromeda_nlp import nlp_model

        model = nlp_model()
        resources_dir = model.get_resources_path()

    return os.path.normpath(resources_dir)


def list_training_data(key: str, force: bool = False, verbose: bool = False):
    """Function to list the training data"""

    return []


def download_items(
    items: Dict[str, Tuple[str, Path]], force: bool = False, verbose: bool = False
) -> Tuple[bool, Dict[str, str]]:
    """
    Iterate through all the items and downloads them.
    The return dictionary will contain the location where items are downloaded.
    If any error occur, the first return value will be false.
    """

    data = {}
    done = True
    for name, (source, target) in items.items():
        if force or (not target.exists()):
            if verbose:
                print(f" -> downloading {name} ... ", end="")

            target.parent.mkdir(exist_ok=True, parents=True)

            with target.open("wb") as fw:
                r = requests.get(source, stream=True)
                if r.ok:
                    for chunk in r.iter_content(chunk_size=8192):
                        fw.write(chunk)
                else:
                    print(f"Error downloading {name}: [{r.status_code}] {r.text}")
                    done = False
                if verbose:
                    print("done!")

            data[name] = target.name
        elif target.exists():
            if verbose:
                print(f" -> already downloaded {name}")
            data[name] = target.name
        else:
            print(f" -> missing {name}")
    return done, data


def load_training_data(
    data_type: str, data_name: str, force: bool = False, verbose: bool = False
):
    """Function to load data to train NLP models"""

    assert data_type in ["text", "crf", "fst"]

    resources_dir = Path(get_resources_dir())

    data_file_path = resources_dir / "data.json"
    with data_file_path.open(encoding="utf-8") as fr:
        training_data = json.load(fr)

    cos_url = training_data["object-store"]
    cos_prfx = training_data["data"]["prefix"]
    cos_path = f"{cos_url}/{cos_prfx}"

    downloads = {}
    for name, files in training_data["data"][data_type].items():
        print(name)
        if name == data_name:
            source = f"{cos_path}/{files[0]}"
            target = resources_dir / str(files[1])

            downloads[name] = (source, target)

    done, data = download_items(downloads)
    return done, data


def load_pretrained_nlp_models(force: bool = False, verbose: bool = False):
    """Function to load pretrained NLP models"""

    resources_dir = Path(get_resources_dir())
    models_file_path = os.path.join(resources_dir, "models.json")

    with open(models_file_path, encoding="utf-8") as fr:
        models = json.load(fr)

    huggingface = models["huggingface"]
    repo_id = huggingface["repo-id"]
    revision = huggingface.get("revision")

    downloaded_models = []
    for name, files in models["nlp"]["trained-models"].items():
        source = files[1]
        target = resources_dir / files[1]

        if target.exists() and not force:
            if verbose:
                print(f" -> already downloaded {name}")
            downloaded_models.append(name)
            continue

        if verbose:
            print(f" -> downloading {name} ... ", end="")

        target.parent.mkdir(exist_ok=True, parents=True)
        try:
            cached_file = hf_hub_download(
                repo_id=repo_id,
                filename=source,
                revision=revision,
                force_download=force,
            )
            shutil.copyfile(cached_file, target)
        except Exception as exc:
            if verbose:
                print("failed!")
            raise RuntimeError(
                f"Failed to download NLP model {name!r} from {repo_id!r} ({source!r})"
            ) from exc

        if verbose:
            print("done!")
        downloaded_models.append(name)

    return downloaded_models
