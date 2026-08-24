#!/usr/bin/env python

import argparse
import glob
import json
import sys

# import os
from typing import List

import pandas as pd
from tabulate import tabulate

from docling_nlp.andromeda_nlp import nlp_model
from docling_nlp.utils.doc_utils import to_legacy_document_format, to_xml_format


def parse_arguments():
    """Parse arguments for `nlp_apply_on_docs`"""

    parser = argparse.ArgumentParser(
        prog="nlp_apply_on_docs",
        description="Apply NLP on JSON documents",
        epilog="""
examples of execution: 

1. run on single JSON document with default model (=`language`):

     uv run python ./docling_nlp/nlp_apply_on_docs.py --json './data/documents/articles/2305.02334.json'

2. run on multiple JSON documents:

     uv run python ./docling_nlp/nlp_apply_on_docs.py --json './data/documents/articles/*.json'

3. run on multiple documents with non-default models:

     uv run python ./docling_nlp/nlp_apply_on_docs.py --json './data/documents/articles/2305.*.json' --models 'language;term'

""",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "--json",
        required=False,
        type=str,
        default=None,
        help="filename(s) of json document",
    )

    parser.add_argument(
        "--models",
        required=False,
        type=str,
        default="language;semantic",
        help="set NLP models (e.g. `language` or `term;sentence`)",
    )

    parser.add_argument(
        "--filters",
        required=False,
        type=str,
        default="",
        help="set output filters (e.g. `properties;instances` only keeps properties and instances), default is no filter",
    )

    parser.add_argument(
        "--legacy",
        required=False,
        type=bool,
        default=False,
        help="enforce old legacy format",
    )

    parser.add_argument(
        "--xml",
        required=False,
        type=bool,
        default=False,
        help="enforce xml format",
    )

    args = parser.parse_args()

    json = args.json

    if json is None:
        sys.exit(-1)

    json_files = sorted(glob.glob(json))

    return (
        json_files,
        args.models,
        args.filters,
        args.legacy,
        args.xml,
    )


# FIXME: to be replaced with function in nlp_utils
def init_nlp_model(models: str, filters: List[str] = []):
    """Function to initialse NLP models"""

    # model = andromeda_nlp.nlp_model()
    model = nlp_model()

    config = model.get_apply_configs()[0]

    config["models"] = models
    config["subject-filters"] = filters

    model.initialise(config)
    model.set_loglevel("INFO")

    return model


def show_texts(doc_j, props):
    """Function to show the text of the document on shell"""

    data = []
    for item in doc_j["texts"]:
        label = ""
        selection = props[props["subj_hash"] == item["subj_hash"]]
        if len(selection) > 0:
            label = selection.iloc[0]["label"]

        data.append([item["subj_hash"], label, item["text"][0:48]])

    print(tabulate(data, headers=["subj_hash", "label", "text"]))


def show_doc(doc_j):
    """Function to show the document"""

    props = None
    if "properties" in doc_j:
        props = pd.DataFrame(
            doc_j["properties"]["data"], columns=doc_j["properties"]["headers"]
        )
        print("properties: \n\n", props.to_string())

    if "texts" in doc_j:
        show_texts(doc_j, props)

    if "instances" in doc_j:
        inst = pd.DataFrame(
            doc_j["instances"]["data"], columns=doc_j["instances"]["headers"]
        )
        # print("instances: \n\n", inst.to_string())

        meta = inst[inst["type"] == "metadata"]
        print("meta: \n\n", meta)

        """
        terms = inst[inst["type"] == "term"]
        print("terms: \n\n", terms)

        hist = terms["hash"].value_counts()
        for key, val in hist.items():
            name = terms[terms["hash"] == key].iloc[0]["name"]
            print(f"{val}\t{name}")
        """


if __name__ == "__main__":
    (
        json_files,
        model_names,
        filters,
        legacy,
        xml,
    ) = parse_arguments()

    json_files = sorted(list(set(json_files)))

    filters_list = []
    if len(filters) > 0:
        filters_list = filters.split(";")

    model = init_nlp_model(model_names, filters_list)

    for json_file in json_files:
        print(f"reading {json_file} ... ", end="")
        with open(json_file, encoding="utf-8") as fr:
            doc_i = json.load(fr)

        print("applying models ... ", end="")
        doc_j = model.apply_on_doc(doc_i)

        # print(doc_j.keys())
        show_doc(doc_j)

        nlp_file = json_file.replace(".json", ".nlp.json")
        print(f"writing  models {nlp_file}")

        with open(nlp_file, "w", encoding="utf-8") as fw:
            fw.write(json.dumps(doc_j, indent=2))

        if legacy:
            doc_i = to_legacy_document_format(doc_j, doc_i)

            nlp_file = json_file.replace(".json", ".leg.json")
            print(f"writing  models {nlp_file}")

            with open(nlp_file, "w", encoding="utf-8") as fw:
                fw.write(json.dumps(doc_i, indent=2))

        if xml:
            doc_xmlstr = to_xml_format(doc_j, normalised_pagedim=100)

            nlp_file = json_file.replace(".json", ".xml.json")
            print(f"writing  models {nlp_file}")

            with open(nlp_file, "w", encoding="utf-8") as fw:
                fw.write(doc_xmlstr)
