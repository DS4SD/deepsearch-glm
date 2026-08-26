#!/usr/bin/env python

GENERATE = False

import glob
import json
import os

from tabulate import tabulate

from docling_nlp.nlp_train_crf import create_crf_model

# from docling_nlp.nlp_train_semantic import train_semantic
from docling_nlp.nlp_train_tok import create_tok_model
from docling_nlp.nlp_utils import (
    eval_crf,
    eval_fst,
    init_nlp_model,
    list_nlp_model_configs,
    prepare_data_for_fst_training,
    train_crf,
    train_fst,
    train_tok,
)
from docling_nlp.utils.load_pretrained_models import (  # load_pretrained_nlp_data,
    get_resources_dir,
    list_training_data,
    load_pretrained_nlp_models,
    load_training_data,
)


def round_floats(o):
    if isinstance(o, float):
        return round(o, 2)
    if isinstance(o, dict):
        return {k: round_floats(v) for k, v in o.items()}
    if isinstance(o, (list, tuple)):
        return [round_floats(x) for x in o]
    return o


def check_dimensions(item):
    assert "headers" in item
    assert "data" in item

    headers = item["headers"]
    for row in item["data"]:
        assert len(row) == len(headers)


def test_01_load_nlp_models():
    models = load_pretrained_nlp_models(force=False, verbose=True)
    # print(f"models: {models}")

    assert "language" in models
    assert "semantic" in models
    assert "name" in models
    assert "reference" in models


# _run_nlp_models_on_text():
def test_02A():
    source = "./tests/data/texts/test_02A_text_01.jsonl"
    target = source

    model = init_nlp_model("spm;sentence;language;term")

    sres = model.apply_on_text("FeSe is a material.")
    sres = round_floats(sres)

    if GENERATE:  # generate the test-data
        fw = open(source, "w")
        fw.write(json.dumps(sres) + "\n")
        fw.close()

        assert True

    else:
        with open(target) as fr:
            tres = json.load(fr)
            tres = round_floats(tres)

        for label in ["properties", "instances"]:
            check_dimensions(sres[label])
            assert label in sres

        for label in ["relations"]:
            assert label not in sres

        # print(tres["properties"])
        # print(sres["properties"])

        assert tres == sres


# _run_nlp_models_on_text():
def test_02B():
    source = "./tests/data/texts/test_02B_text_01.jsonl"
    target = source

    filters = ["properties"]

    model = init_nlp_model("sentence;language;term", filters)

    sres = model.apply_on_text("FeSe is a material.")
    sres = round_floats(sres)

    if GENERATE:  # generate the test-data
        fw = open(source, "w")
        fw.write(json.dumps(sres) + "\n")
        fw.close()

        assert True

    else:
        with open(target) as fr:
            tres = json.load(fr)
            tres = round_floats(tres)

        for label in ["text", "properties"]:
            assert label in sres

        for label in ["instances", "relations"]:
            assert label not in sres

        assert tres == sres


# test term model
def test_04A():
    source = "./tests/data/texts/terms.jsonl"
    target = "./tests/data/texts/terms.nlp.jsonl"

    model = init_nlp_model("language;semantic;sentence;term;verb;conn;geoloc")

    if GENERATE:  # generate the test-data
        with open(source) as fr:
            lines = fr.readlines()

        fw = open(target, "w")

        for line in lines:
            data = json.loads(line)
            data = round_floats(data)

            res = model.apply_on_text(data["text"])
            res = round_floats(res)

            fw.write(json.dumps(res) + "\n")

        fw.close()

    else:
        with open(target) as fr:
            lines = fr.readlines()

        for line in lines:
            data = json.loads(line)
            data = round_floats(data)

            res = model.apply_on_text(data["text"])
            res = round_floats(res)

            """
            for i,row_i in enumerate(res["properties"]["data"]):
                row_j = data["properties"]["data"][i]
                assert row_i==row_j

            for i,row_i in enumerate(res["instances"]["data"]):
                row_j = data["instances"]["data"][i]
                assert row_i==row_j
            """

            assert res == data

    assert True


# test semantic classifier
def test_04B():
    model = init_nlp_model("semantic")

    source = "./tests/data/texts/semantics.jsonl"
    target = "./tests/data/texts/semantics.nlp.jsonl"

    if GENERATE:  # generate the test-data
        with open(source) as fr:
            lines = fr.readlines()

        fw = open(target, "w")

        for line in lines:
            data = json.loads(line)
            data = round_floats(data)

            res = model.apply_on_text(data["text"])
            res = round_floats(res)

            fw.write(json.dumps(res) + "\n")

        fw.close()
        assert True

    else:
        with open(target) as fr:
            lines = fr.readlines()

        for line in lines:
            data = json.loads(line)
            data = round_floats(data)

            res = model.apply_on_text(data["text"])
            res = round_floats(res)

            """
            for i,row_i in enumerate(res["properties"]["data"]):
                row_j = data["properties"]["data"][i]
                assert row_i==row_j
            """

            assert res == data


# test reference model
def test_04C():
    model = init_nlp_model("reference")

    source = "./tests/data/texts/references.jsonl"
    target = "./tests/data/texts/references.nlp.jsonl"

    if GENERATE:  # generate the test-data
        with open(source) as fr:
            lines = fr.readlines()

        fw = open(target, "w")

        for line in lines:
            data = json.loads(line)
            data = round_floats(data)

            res = model.apply_on_text(data["text"])
            res = round_floats(res)

            fw.write(json.dumps(res) + "\n")

        fw.close()
        assert True

    else:
        with open(target) as fr:
            lines = fr.readlines()

        for line in lines:
            data = json.loads(line)
            data = round_floats(data)

            res = model.apply_on_text(data["text"])
            res = round_floats(res)

            assert res == data


# download CRF data
def test_06A():
    verbose = False

    done, data = load_training_data(
        data_type="crf", data_name="materials", force=False, verbose=verbose
    )
    # done, data = load_pretrained_nlp_data(key="crf", force=False, verbose=verbose)

    if verbose:
        print(json.dumps(data, indent=2))

    assert done


# train CRF
def test_06B():
    resources_dir = get_resources_dir()

    crf_files = glob.glob(f"{resources_dir}/data/nlp/crf.*.jsonl")
    assert len(crf_files) > 0

    # print(crf_files)
    for crf_file in crf_files:
        if crf_file.endswith(".annot.jsonl"):
            continue

        print(f"training on {crf_file}")
        annot_file, model_file, metrics_file = create_crf_model(
            mode="all", ifile=crf_file, odir=os.path.dirname(crf_file), max_items=1000
        )

        assert os.path.exists(annot_file)
        assert os.path.exists(model_file)
        assert os.path.exists(metrics_file)

    assert True


# predict CRF
def test_06C():
    verbose = False

    resources_dir = get_resources_dir()

    # print(f"{resources_dir}/data_nlp/crf.*.jsonl")
    crf_files = glob.glob(f"{resources_dir}/data_nlp/crf.*.jsonl")

    for crf_file in crf_files:
        if crf_file.endswith(".annot.jsonl"):
            continue

        name = os.path.basename(crf_file).replace("crf.", "").replace(".jsonl", "")

        # print(f"running on {crf_file}")
        annot_file, model_file, metrics_file = create_crf_model(
            mode="files", ifile=crf_file, odir=os.path.dirname(crf_file), max_items=1000
        )

        assert os.path.exists(model_file)

        model = init_nlp_model(
            f"language;custom_crf({name}:{model_file})",
            filters=["properties", "instances"],
        )

        fr = open(crf_file)

        while True:
            line = fr.readline()

            if line is None or len(line.strip()) == 0:
                break

            data = json.loads(line)
            sres = model.apply_on_text(data["text"])

            if verbose:
                print(sres["text"])
                print(
                    tabulate(
                        sres["instances"]["data"], headers=sres["instances"]["headers"]
                    )
                )

        fr.close()

    assert True


# download text data for tokenizers
def test_07A():
    verbose = True

    done, data = load_training_data(
        data_type="text",
        data_name="arxiv-abstracts-2020-Jan-txt",
        force=False,
        verbose=verbose,
    )

    if verbose:
        print(json.dumps(data, indent=2))

    assert done


# train tokenizer
def test_07B():
    resources_dir = get_resources_dir()

    txt_file = f"{resources_dir}/data/text/arxiv-abstracts-2020-Jan.txt"
    assert os.path.exists(txt_file)

    model_type = "unigram"
    model_name = "test-tokenizer-model"

    print(f"training on {txt_file}")
    model_file = create_tok_model(
        model_type=model_type, model_name=model_name, ifile=txt_file
    )

    assert os.path.exists(model_name + ".model")
    assert os.path.exists(model_name + ".vocab")


# load custom tokenizer
def test_07C():
    model_name = "test-tokenizer-model"
    model_file = f"{model_name}.model"

    assert os.path.exists(model_file)

    model = init_nlp_model(
        f"language;custom_spm({model_name}:{model_file})",
        filters=["properties", "instances"],
    )


# download text data for fst-classifier
def test_08A():
    verbose = True

    done, data = load_training_data(
        data_type="fst",
        data_name="names-classifier-small",
        force=False,
        verbose=verbose,
    )

    if verbose:
        print(json.dumps(data, indent=2))

    assert done


# train fst-classifier
def test_08B():
    resources_dir = get_resources_dir()

    data_file = f"{resources_dir}/data/nlp/nlp.fst.names-classifier.small.jsonl"
    assert os.path.exists(data_file)

    prepare_data_for_fst_training(data_file=data_file, loglevel="INFO")

    model_file = data_file + ".fst_model.bin"
    metrics_file = data_file + ".fst_metrics.txt"

    train_fst(
        data_file=data_file,
        model_file=model_file,
        metrics_file=metrics_file,
        ngram=3,
        duration=60,
        autotune=True,
        modelsize="1M",
        loglevel="INFO",
    )

    assert os.path.exists(model_file)

    eval_fst(
        data_file=data_file,
        model_file=model_file,
        metrics_file=metrics_file,
        loglevel="INFO",
    )

    assert os.path.exists(metrics_file)
