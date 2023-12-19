#!/usr/bin/env python
"""Module to do DocQA"""

import argparse

# import glob
import json

import matplotlib.pyplot as plt
import pandas as pd
from tabulate import tabulate

# from deepsearch_glm.andromeda_glm import glm_model, glm_query
from deepsearch_glm.andromeda_glm import glm_query

# import andromeda_glm
# import andromeda_nlp
# from deepsearch_glm.andromeda_nlp import nlp_model
from deepsearch_glm.glm_utils import load_glm
from deepsearch_glm.nlp_utils import init_nlp_model

# import os
# import textwrap


# from ds_convert import convert_pdffile


def parse_arguments():
    """Function to parse arguments for `glm_docqa`"""

    parser = argparse.ArgumentParser(
        prog="glm_docqa",
        description="Do Q&A on pdf document",
        epilog="Text at the bottom of help",
    )

    parser.add_argument(
        "--glm-dir", required=True, type=str, help="directory of GLM model"
    )

    parser.add_argument(
        "--qa-pairs",
        required=False,
        type=str,
        default="prompt",
        help="CSV file with QA pairs or `prompt`",
    )

    parser.add_argument(
        "--models",
        required=False,
        type=str,
        default="name;verb;term;abbreviation",
        help="set NLP models (e.g. `term;sentence`)",
    )

    args = parser.parse_args()

    return args.glm_dir, args.qa_pairs, args.models


def analyse_prompt(prompt, nlp_model):
    """Function to analyse the prompt"""

    res = nlp_model.apply_on_text(prompt)

    print(res.keys())
    print(tabulate(res["word-tokens"]["data"], headers=res["word-tokens"]["headers"]))
    print(tabulate(res["entities"]["data"], headers=res["entities"]["headers"]))

    type_ind = res["entities"]["headers"].index("type")
    name_ind = res["entities"]["headers"].index("name")

    terms = []
    for i, row in enumerate(res["entities"]["data"]):
        if row[type_ind] == "term":
            terms.append([row[name_ind]])

    return terms


def compute_topk_on_documents(df, nlp_mdl, glm_mdl):
    """Function to compute topk of documents"""

    topk = {0: 0}
    for ind in range(1, 10):
        topk[ind] = 0

    for i, row in df.iterrows():
        print(i, row["question"])

        doc_hash = row["doc_hash"]
        question = row["question"]

        context = row["text"]

        qres = nlp_mdl.apply_nlp_on_text(question)
        cres = nlp_mdl.apply_nlp_on_text(context)

        data = cres["instances"]["data"]
        headers = cres["instances"]["headers"]

        insts = []
        for j, row in enumerate(data):
            insts.append(
                [
                    row[headers.index("type")],
                    row[headers.index("subtype")],
                    row[headers.index("name")],
                ]
            )

        data = qres["instances"]["data"]
        headers = qres["instances"]["headers"]

        insts = []
        for j, row in enumerate(data):
            insts.append(
                [
                    row[headers.index("type")],
                    row[headers.index("subtype")],
                    row[headers.index("name")],
                ]
            )

        terms = []
        for j, row in enumerate(insts):
            if "term" == row[0]:
                term = row[2].split()
                terms.append(term)

        # qry = andromeda_glm.glm_query()
        qry = glm_query()
        qry.select({"nodes": terms})
        qry.traverse({"edge": "to-doc"})

        config = qry.to_config()
        # print("query: ", json.dumps(config, indent=2))

        out = glm_mdl.query(config)
        # print(json.dumps(out, indent=2))

        if out["status"] == "success":
            docs = pd.DataFrame(
                out["result"][1]["nodes"]["data"],
                columns=out["result"][1]["nodes"]["headers"],
            )

            doc_hashes = list(docs["text"])

            for k, v in topk.items():
                if k == 0:
                    topk[k] += 1
                elif doc_hash in doc_hashes[0:k]:
                    topk[k] += 1

    print(json.dumps(topk, indent=2))

    x = []
    y = []
    for i in range(1, 10):
        x.append(i)
        y.append(topk[i] / topk[0])

    plt.figure(1)
    plt.plot(x, y, "r.-", label="doc-topk")
    plt.ylim(0, 1.05)
    plt.legend(loc="lower right")
    plt.show()


def compute_topk_on_element(df, nlp_mdl, glm_mdl):
    """Function to compute topk elements"""

    topk = {0: 0}
    for ind in range(1, 10):
        topk[ind] = 0

    for i, row in df.iterrows():
        print(i, row["question"])

        doc_hash = row["doc_hash"]
        question = row["question"]

        context = row["text"]

        qres = nlp_mdl.apply_nlp_on_text(question)
        cres = nlp_mdl.apply_nlp_on_text(context)

        data = qres["instances"]["data"]
        headers = qres["instances"]["headers"]

        insts = []
        for j, row in enumerate(data):
            insts.append(
                [
                    row[headers.index("type")],
                    row[headers.index("subtype")],
                    row[headers.index("name")],
                ]
            )
        """
        print(f"question: {question}\n")
            
        print("instances: ")
        print(tabulate(insts, headers=["type", "subtype", "name"]), "\n")
        """

        terms = []
        for j, row in enumerate(insts):
            if "term" == row[0]:
                term = row[2].split()
                terms.append(term)

        # qry = andromeda_glm.glm_query()
        qry = glm_query()
        qry.select({"nodes": terms})
        qry.traverse({"edge": "to-text"})

        config = qry.to_config()
        # print("query: ", json.dumps(config, indent=2))

        out = glm_mdl.query(config)
        # print(json.dumps(out, indent=2))

        if out["status"] == "success":
            docs = pd.DataFrame(
                out["result"][1]["nodes"]["data"],
                columns=out["result"][1]["nodes"]["headers"],
            )

            doc_hashes = list(docs["text"])

            for k, v in topk.items():
                if k == 0:
                    topk[k] += 1
                elif doc_hash in doc_hashes[0:k]:
                    topk[k] += 1

    print(json.dumps(topk, indent=2))


if __name__ == "__main__":
    glm_dir, qa_pairs_file, models = parse_arguments()

    glm_mdl = load_glm(glm_dir)
    # nlp_mdl = load_nlp(models)
    nlp_mdl = init_nlp_model(
        model_names="name;conn;verb;term;language;reference;abbreviation"
    )

    df = pd.read_csv(qa_pairs_file)

    compute_topk_on_documents(df, nlp_mdl, glm_mdl)
    # compute_topk_on_element(df, nlp_mdl, glm_mdl)
