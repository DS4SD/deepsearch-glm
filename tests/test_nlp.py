#!/usr/bin/env python

GENERATE = False

import glob
import json
import os

from tabulate import tabulate

from deepsearch_glm.nlp_train_crf import create_crf_model

# from deepsearch_glm.nlp_train_semantic import train_semantic
from deepsearch_glm.nlp_train_tok import create_tok_model
from deepsearch_glm.nlp_utils import (
    eval_crf,
    eval_fst,
    extract_references_from_doc,
    init_nlp_model,
    list_nlp_model_configs,
    prepare_data_for_fst_training,
    train_crf,
    train_fst,
    train_tok,
)
from deepsearch_glm.utils.doc_utils import to_legacy_document_format
from deepsearch_glm.utils.load_pretrained_models import (  # load_pretrained_nlp_data,
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


def get_reduced_instances(instances):
    headers = instances["headers"]

    table = []
    for row in instances["data"]:
        if "reference" in row[0] and "texts" in row[4]:
            table.append([row[0], row[1], row[4], row[5], row[-2]])

    return table, [headers[0], headers[1], headers[4], headers[5], headers[-2]]


def compare_docs(doc_i, doc_j):
    for key, val in doc_j.items():
        if isinstance(val, dict) and ("data" in val) and ("headers" in val):
            assert val["headers"] == doc_i[key]["headers"]

            """
            if val["headers"] != doc_i[key]["headers"]:
                return False
            """

            assert len(val["data"]) == len(doc_i[key]["data"])

            """
            if len(val["data"]) != len(doc_i[key]["data"]):
                return False
            """

            for j, row in enumerate(val["data"]):
                """
                if row != doc_i[key]["data"][j]:
                    return False
                """

                if row != doc_i[key]["data"][j]:
                    print(f"1 ({j}): ", row)
                    print(f"2 ({j}): ", doc_i[key]["data"][j])

                assert row == doc_i[key]["data"][j]

        elif doc_i[key] != doc_j[key]:
            assert doc_i[key] == doc_j[key]
            return False

    return True


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


# _run_nlp_models_on_document():
def test_03A():
    with open("./tests/data/docs/1806.02284.json") as fr:
        doc = json.load(fr)

    model = init_nlp_model("sentence;language;term;reference;abbreviation")

    res = model.apply_on_doc(doc)
    res = round_floats(res)

    for label in [
        "description",
        "body",
        "meta",
        "page-elements",
        "texts",
        "tables",
        "figures",
        "properties",
        "instances",
        "relations",
    ]:
        assert label in res

    check_dimensions(res["properties"])
    check_dimensions(res["instances"])
    check_dimensions(res["relations"])


# _run_nlp_models_on_document():
def test_03B():
    with open("./tests/data/docs/1806.02284.json") as fr:
        doc = json.load(fr)

    filters = ["applied_models", "properties"]

    model = init_nlp_model("sentence;language;term;reference", filters)

    res = model.apply_on_doc(doc)
    res = round_floats(res)

    for label in [
        "dloc",
        "applied_models",
        "description",
        "body",
        "meta",
        "page-elements",
        "texts",
        "tables",
        "figures",
        "properties",
    ]:
        assert label in res

    for label in ["instances", "relations"]:
        assert label not in res

    check_dimensions(res["properties"])


# _run_nlp_models_on_document():
def test_03C():
    model = init_nlp_model("language;semantic;sentence;term;verb;conn;geoloc;reference")

    source = "./tests/data/docs/1806.02284.json"
    target = "./tests/data/docs/1806.02284.nlp.json"

    if GENERATE:  # generate the test-data
        with open(source) as fr:
            doc = json.load(fr)

        res = model.apply_on_doc(doc)
        res = round_floats(res)

        # extract_references_from_doc(res)

        fw = open(target, "w")
        fw.write(json.dumps(res, indent=2) + "\n")
        fw.close()

        assert True

    else:
        with open(source) as fr:
            sdoc = json.load(fr)

        res = model.apply_on_doc(sdoc)
        res = round_floats(res)

        with open(target) as fr:
            tdoc = json.load(fr)
            tdoc = round_floats(tdoc)

        assert res == tdoc


# run_nlp_models_on_document():
def test_03D():
    model_i = init_nlp_model("term")
    model_j = init_nlp_model("reference")
    # model_j = init_nlp_model("verb")

    model_k = init_nlp_model("term;reference")
    # model_k = init_nlp_model("term;verb")

    source = "./tests/data/docs/1806.02284.json"

    # target_i = "./tests/data/docs/1806.02284.nlp.i.json"
    # target_j = "./tests/data/docs/1806.02284.nlp.j.json"
    # target_k = "./tests/data/docs/1806.02284.nlp.k.json"

    with open(source) as fr:
        doc = json.load(fr)

    # print("apply model_i")
    res_i = model_i.apply_on_doc(doc)
    # res_i = round_floats(res_i)

    """
    fw = open(target_i, "w")
    fw.write(json.dumps(res_i, indent=2) + "\n")
    fw.close()
    """

    # print("apply model_j")
    res_j = model_j.apply_on_doc(res_i)
    # res_j = model_j.apply_on_doc(doc)
    res_j = round_floats(res_j)

    """
    fw = open(target_j, "w")
    fw.write(json.dumps(res_j, indent=2) + "\n")
    fw.close()
    """

    # print("apply model_k")
    res_k = model_k.apply_on_doc(doc)
    res_k = round_floats(res_k)

    """
    fw = open(target_k, "w")
    fw.write(json.dumps(res_k, indent=2) + "\n")
    fw.close()
    """

    assert res_j["tables"] == res_k["tables"]

    """
    print(tabulate(res_j["properties"]["data"][0:30],
    headers=res_j["properties"]["headers"]))
        
    print(tabulate(res_k["properties"]["data"][0:30],
    headers=res_k["properties"]["headers"]))
    """

    assert len(res_j["properties"]["data"]) == len(res_k["properties"]["data"])
    assert res_j["properties"]["data"] == res_k["properties"]["data"]

    table_i, headers_i = get_reduced_instances(res_i["instances"])
    table_j, headers_j = get_reduced_instances(res_j["instances"])
    table_k, headers_k = get_reduced_instances(res_k["instances"])

    # print(tabulate(table_j, headers=headers_j))
    # print(tabulate(table_k, headers=headers_k))

    """
    print("#-inst-i: ", len(table_i))
    print("#-inst-j: ", len(table_j))
    print("#-inst-k: ", len(table_k))
    """
    assert table_j == table_k

    print("#-instances-j: ", len(res_j["instances"]["data"]))
    print("#-instances-k: ", len(res_k["instances"]["data"]))

    # print(tabulate(res_j["instances"]["data"][-30:]))
    # print(tabulate(res_k["instances"]["data"][-30:]))

    for j in range(0, len(res_j["instances"]["data"])):
        found = False
        for k in range(0, len(res_k["instances"]["data"])):
            if res_k["instances"]["data"][k] == res_j["instances"]["data"][j]:
                found = True

        if not found:
            # print(i)
            # print(res_j["instances"]["data"][j])
            print(res_k["instances"]["data"][j])

    for k in range(0, len(res_k["instances"]["data"])):
        found = False
        for j in range(0, len(res_j["instances"]["data"])):
            if res_k["instances"]["data"][k] == res_j["instances"]["data"][j]:
                found = True

        if not found:
            # print(i)
            # print(res_j["instances"]["data"][j])
            print(res_k["instances"]["data"][k])

    assert len(res_j["instances"]["data"]) == len(res_k["instances"]["data"])
    assert res_j["instances"]["data"] == res_k["instances"]["data"]

    assert res_j == res_k


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


def test_05A():
    model = init_nlp_model("reference;term")

    source = "./tests/data/docs/doc_01.old.json"

    target_leg = "./tests/data/docs/doc_01.leg.json"
    target_nlp = "./tests/data/docs/doc_01.nlp.json"

    # print(f"reading {source} ... ", end="")
    with open(source, "r") as fr:
        doc_i = json.load(fr)

    if GENERATE:
        doc_j = model.apply_on_doc(doc_i)
        doc_j = round_floats(doc_j)

        with open(target_nlp, "w") as fw:
            fw.write(json.dumps(doc_j, indent=2))

        doc_i = to_legacy_document_format(doc_j, doc_i)
        doc_i = round_floats(doc_i)

        with open(target_leg, "w") as fw:
            fw.write(json.dumps(doc_i, indent=2))
    else:
        with open(target_nlp, "r") as fr:
            doc_nlp = json.load(fr)
            doc_nlp = round_floats(doc_nlp)

        with open(target_leg, "r") as fr:
            doc_leg = json.load(fr)
            doc_leg = round_floats(doc_leg)

        doc_j = model.apply_on_doc(doc_i)
        doc_j = round_floats(doc_j)

        assert compare_docs(doc_j, doc_nlp)
        """
        for key,val in doc_j.items():

            if isinstance(val, dict) and ("data" in val) and ("headers" in val):
                
                assert val["headers"] == doc_nlp[key]["headers"]
                assert len(val["data"]) == len(doc_nlp[key]["data"])

                for j,row in enumerate(val["data"]):
                    assert row == doc_nlp[key]["data"][j]
                    
            else:
                assert doc_j[key] == doc_nlp[key]
        """

        assert doc_j == doc_nlp

        doc_i = to_legacy_document_format(doc_j, doc_i)
        doc_i = round_floats(doc_i)

        assert compare_docs(doc_i, doc_leg)

        """
        #assert doc_i == doc_leg
        for key,val in doc_i.item():
            assert doc_i[key] == doc_leg[key]
        """


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

        fr = open(crf_file, "r")

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
