#!/usr/bin/env python
"""Module for NLP utilities"""

# import argparse
import datetime

# import glob
import json
import os
import sys

# import subprocess
import textwrap
from typing import List

import pandas as pd
from rich.console import Console
from tabulate import tabulate

from deepsearch_glm.andromeda_nlp import nlp_model
from deepsearch_glm.utils.common import get_scratch_dir

# import andromeda_nlp


console = Console()


def create_nlp_dir(tdir=None):
    """Function to create NLP directory"""

    if tdir is None:
        tdir = get_scratch_dir()

    now = datetime.datetime.now()
    nlpdir = now.strftime("NLP-model-%Y-%m-%d_%H-%M-%S")

    odir = os.path.join(tdir, nlpdir)
    return odir


def get_max_items(ifile: str, max_lines: int = -1):
    num_lines = sum(1 for _ in open(ifile, "r", encoding="utf-8"))
    if max_lines != -1:
        max_lines = min(max_lines, num_lines)
    else:
        max_lines = num_lines

    return max_lines


def list_nlp_model_configs():
    """Function to list all available NLP models"""

    configs = []

    configs += nlp_model.get_apply_configs()
    configs += nlp_model.get_train_configs()

    return configs


def init_nlp_model(
    model_names: str = "language;term",
    filters: List[str] = [],
    loglevel: str = "WARNING",
):
    """Function to initialise NLP models"""

    # model = andromeda_nlp.nlp_model()
    model = nlp_model()
    model.set_loglevel(loglevel)

    configs = model.get_apply_configs()
    # print(json.dumps(configs, indent=2))

    config = model.get_apply_configs()[0]
    config["models"] = model_names
    config["subject-filters"] = filters

    model.initialise(config)

    return model


def print_key_on_shell(key, items):
    """Function to print NLP-items on shell"""

    df = pd.DataFrame(items["data"], columns=items["headers"])

    if key in ["instances", "entities"]:
        wrapper = textwrap.TextWrapper(width=70)

        df = df[["type", "subtype", "subj_path", "char_i", "char_j", "original"]]

        table = []
        for i, row in df.iterrows():
            _ = []
            for __ in row:
                if isinstance(__, str):
                    _.append("\n".join(wrapper.wrap(__)))
                else:
                    _.append(__)

            table.append(_)

        headers = ["type", "subtype", "subj_path", "char_i", "char_j", "original"]
        console.print(f"{key}: \n", style="yellow")
        console.print(tabulate(table, headers=headers), "\n")

    else:
        df = pd.DataFrame(items["data"], columns=items["headers"])

        console.print(f"{key}: \n", style="yellow")
        console.print(df.to_string(), "\n")


def print_on_shell(text, result):
    """Function to print text on shell"""

    wrapper = textwrap.TextWrapper(width=70)
    console.print(f"\ntext: \n", style="yellow")
    console.print("\n".join(wrapper.wrap(text)), "\n")

    for _ in ["properties", "word-tokens", "instances", "entities", "relations"]:
        if _ in result and len(result[_]["data"]) > 0:
            print_key_on_shell(_, result[_])
        else:
            console.print(f"[yellow]{_}:[/yellow]", " null\n\n")


def extract_metadata_from_doc(doc):
    """Function to extract metadata from document"""

    df = pd.DataFrame(doc["instances"]["data"], columns=doc["instances"]["headers"])

    metadata = df[df["type"] == "metadata"]

    return metadata


def extract_texts_from_doc(doc):
    """Function to extract texts from document"""

    texts = pd.DataFrame.from_records(doc["texts"])

    return texts


def extract_sentences_from_doc(doc):
    """Function to extract the sentences of a document"""

    df = pd.DataFrame(doc["instances"]["data"], columns=doc["instances"]["headers"])

    sents = df[df["type"] == "sentence"]

    return sents


def extract_references_from_doc(doc):
    """Function to extract references from document"""

    texts = pd.DataFrame.from_records(doc["texts"])

    props = pd.DataFrame(
        doc["properties"]["data"], columns=doc["properties"]["headers"]
    )

    insts = pd.DataFrame(doc["instances"]["data"], columns=doc["instances"]["headers"])

    refs = props[props["label"] == "reference"]

    results = []

    for i, ref in refs.iterrows():
        text = texts[texts["hash"] == ref["subj_hash"]]
        refc = insts[insts["subj_hash"] == ref["subj_hash"]]

        results.append(
            {"text": text["text"], "path": text["sref"], "instances": refc.to_records()}
        )

    return results


def train_crf(
    model_name: str,
    train_file: str,
    model_file: str,
    metrics_file: str,
    loglevel: str = "WARNING",
):
    """Function to train CRF model"""

    model = nlp_model()
    model.set_loglevel(loglevel)

    configs = model.get_train_configs()

    was_trained = False
    for config in configs:
        if config["mode"] == "train" and config["model"].startswith("custom_crf"):
            config["files"]["model-name"] = model_name
            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file
            config["files"]["metrics-file"] = metrics_file

            model.train(config)
            was_trained = True

    if not was_trained:
        print(json.dumps(configs, indent=2))
        sys.exit(-1)


def eval_crf(
    model_name: str,
    train_file: str,
    model_file: str,
    metrics_file: str,
    loglevel: str = "WARNING",
):
    """Function to train CRF model"""

    model = nlp_model()
    model.set_loglevel(loglevel)

    configs = model.get_train_configs()

    was_trained = False
    for config in configs:
        if config["mode"] == "train" and config["model"].startswith("custom_crf"):
            config["files"]["model-name"] = model_name
            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file
            config["files"]["metrics-file"] = metrics_file

            model.evaluate(config)
            was_trained = True

    if not was_trained:
        print(json.dumps(configs, indent=2))
        sys.exit(-1)


def train_tok(
    model_type: str,
    model_name: str,
    ifile: str,
    vocab_size: int = 4096,
    control_symbols: List[str] = [],
    user_symbols: List[str] = [],
    loglevel: str = "WARNING",
):
    """Train a new tokenizer model"""

    model = nlp_model()
    model.set_loglevel(loglevel)

    configs = model.get_train_configs()

    was_trained = False
    for config in configs:
        if config["mode"] == "train" and config["model"] == "spm":
            config["args"]["input-file"] = ifile
            config["args"]["model-type"] = model_type
            config["args"]["model-name"] = model_name
            config["args"]["vocab-size"] = vocab_size

            was_trained = model.train(config)
            break

    if not was_trained:
        print(json.dumps(configs, indent=2))
        sys.exit(-1)

    return was_trained, config


# To train a FST model with HPO, one can use
#
# `./fasttext supervised -input <path-to-train.txt> -output model_name -autotune-validation <<path-to-valid.txt>> -autotune-duration 600 -autotune-modelsize 1M`
#
#  => the parameters can be found via
#
# `./fasttext dump model_cooking.bin args`
#
def train_fst_legacy(
    model_name: str, train_file: str, model_file: str, metrics_file: str
):
    """Function to train fasttext model"""

    # model = andromeda_nlp.nlp_model()
    model = nlp_model()

    configs = model.get_train_configs()
    print(configs)

    for config in configs:
        if config["mode"] == "train" and config["model"] == model_name:
            config["files"]["model-file"] = model_file
            config["files"]["train-file"] = train_file
            config["files"]["metrics-file"] = metrics_file

            model.train(config)


def prepare_data_for_fst_training(
    data_file: str,
    loglevel: str = "WARNING"
    # test_file:str, validation_file:str,
    # model_file:str, metrics_file:str,
    # ngram=3, autotune=True, duration=360, modelsize="1M"
):
    """Function to train fasttext classifier"""

    model = nlp_model()

    configs = model.get_train_configs()
    # print(json.dumps(configs, indent=2))

    for config in configs:
        # print(" => ", config["model"])

        if config["mode"] == "train" and config["model"].startswith("custom_fst"):
            config["args"] = {}

            config["files"]["data-file"] = data_file
            model.prepare_data_for_train(config)


# To train a FST model with HPO, one can use
#
# `./fasttext supervised -input <path-to-train.txt> -output model_name -autotune-validation <<path-to-valid.txt>> -autotune-duration 600 -autotune-modelsize 1M`
#
#  => the parameters can be found via
#
# `./fasttext dump model_cooking.bin args`
#
def train_fst(
    # train_file:str, test_file:str, validation_file:str,
    data_file: str,
    model_file: str,
    metrics_file: str,
    autotune=True,
    duration=360,
    modelsize="1M",
    ngram=None,
    loglevel: str = "WARNING",
):
    """Function to train fasttext classifier"""

    model = nlp_model()
    model.set_loglevel(loglevel)

    configs = model.get_train_configs()
    # print(json.dumps(configs, indent=2))

    for config in configs:
        if config["mode"] == "train" and config["model"].startswith("custom_fst"):
            config["args"] = {}
            config["hpo"]["autotune"] = autotune
            config["hpo"]["duration"] = duration
            config["hpo"]["modelsize"] = modelsize

            if ngram != None:
                config["args"]["n-gram"] = ngram

            config["files"]["data-file"] = data_file
            config["files"]["model-file"] = model_file
            config["files"]["metrics-file"] = metrics_file

            # print(json.dumps(config, indent=2))
            model.train(config)


def eval_fst(
    data_file: str, model_file: str, metrics_file: str, loglevel: str = "WARNING"
):
    """Function to evaluate fasttext classifier"""

    model = nlp_model()
    model.set_loglevel(loglevel)

    configs = model.get_train_configs()
    # print(json.dumps(configs, indent=2))

    for config in configs:
        if config["mode"] == "train" and config["model"].startswith("custom_fst"):
            config["args"] = {}

            config["files"]["data-file"] = data_file

            config["files"]["model-file"] = model_file
            config["files"]["metrics-file"] = metrics_file

            model.evaluate(config)
