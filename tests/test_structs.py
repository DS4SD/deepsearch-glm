#!/usr/bin/env python
"""Module to test the nlp structs from andromeda_nlp"""

import json

import pandas as pd
from tabulate import tabulate

from deepsearch_glm.andromeda_structs import ds_document, ds_table, ds_text
from deepsearch_glm.nlp_utils import init_nlp_model
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models

TEXTS = [
    """Anarchism is a political philosophy and movement that is sceptical
of authority and rejects all involuntary, coercive forms of hierarchy. Anarchism
calls for the abolition of the state, which it holds to be unnecessary,
undesirable, and harmful. As a historically left-wing movement, placed on the
farthest left of the political spectrum, it is usually described alongside
communalism and libertarian Marxism as the libertarian wing (libertarian
socialism) of the socialist movement, and has a strong historical association
with anti-capitalism and socialism.""",
    """Humans lived in societies without formal hierarchies long before the
establishment of formal states, realms, or empires. With the rise of organised
hierarchical bodies, scepticism toward authority also rose. Although traces of
anarchist thought are found throughout history, modern anarchism emerged from
the Enlightenment. During the latter half of the 19th and the first decades of
the 20th century, the anarchist movement flourished in most parts of the world
and had a significant role in workers' struggles for emancipation. Various
anarchist schools of thought formed during this period. Anarchists have taken
part in several revolutions, most notably in the Paris Commune, the Russian
Civil War and the Spanish Civil War, whose end marked the end of the classical
era of anarchism. In the last decades of the 20th and into the 21st century,
the anarchist movement has been resurgent once more.""",
]

TABLES = [
    [
        ["Year", "Company", "Revenue"],
        ["2020", "XXX", "10B USD"],
        ["2021", "XXX", "10B USD"],
        ["2022", "XXX", "10B USD"],
        ["2020", "YYY", "12B USD"],
        ["2021", "YYY", "13B USD"],
        ["2022", "YYY", "14B USD"],
    ]
]


def to_dataframe(obj):
    return pd.DataFrame(obj["data"], columns=obj["headers"])


def test_0A_load_nlp_models():
    models = load_pretrained_nlp_models(force=False, verbose=True)

    assert "language" in models
    assert "semantic" in models
    assert "name" in models
    assert "reference" in models


def test_01A():
    subj = ds_text()
    print(subj)

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_01B():
    subj = ds_table()

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_01C():
    subj = ds_document()

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_02A():
    subj = ds_text()

    subj.set_text(TEXTS[0])

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_02B():
    subj = ds_table()

    subj.set_data(TABLES[0])
    res = subj.to_json(set([]))

    assert True


def test_02C():
    subj = ds_document()
    print(subj)

    for text in TEXTS:
        text_subj = ds_text()
        text_subj.set_text(text)

        subj.append_text(text_subj)

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_03A():
    subj = ds_text()
    print(subj)

    subj.set_text(TEXTS[0])

    res = subj.to_json(set([]))
    # print(json.dumps(res, indent=2))

    model = init_nlp_model(
        "language;semantic;sentence;term;verb;conn;geoloc;abbreviation"
    )
    model.apply_on_text(subj)

    res = subj.to_json(set([]))
    print("keys: ", res.keys())

    print(res["applied_models"])

    for key in ["properties", "instances", "relations"]:
        if key in res:
            df = to_dataframe(res[key])
            print(df)

    assert True


def test_03B():
    subj = ds_table()

    subj.set_data(TABLES[0])
    res = subj.to_json(set([]))

    model = init_nlp_model(
        "language;semantic;sentence;term;verb;conn;geoloc;abbreviation"
    )
    model.apply_on_table(subj)

    res = subj.to_json(set([]))
    # res = subj.to_json(set(["word-tokens", "word_tokens", "properties", "instances", "relations"]))
    print("keys: ", res.keys())

    for key in ["properties", "instances", "relations"]:
        if key in res:
            df = to_dataframe(res[key])
            print(df)

    assert True


def test_03C():
    subj = ds_document()

    for text in TEXTS:
        text_subj = ds_text()
        text_subj.set_text(text)

        subj.append_text(text_subj)

    for table in TABLES:
        table_subj = ds_table()
        table_subj.set_data(table)

        subj.append_table(table_subj)

    jdoc = subj.to_json(set())
    print(json.dumps(jdoc, indent=2))

    model = init_nlp_model(
        "language;semantic;sentence;term;verb;conn;geoloc;abbreviation"
    )
    model.apply_on_doc(subj)

    res = subj.to_json(set([]))
    print("keys: ", res.keys())

    for key in ["properties", "instances", "relations"]:
        if key in res:
            df = to_dataframe(res[key])
            print(df)

    assert True
