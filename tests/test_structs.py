#!/usr/bin/env python
"""Module to test the nlp structs from andromeda_nlp"""

import json

import pandas as pd

from deepsearch_glm.andromeda_structs import nlp_document, nlp_table, nlp_text
from deepsearch_glm.nlp_utils import init_nlp_model

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


def to_dataframe(obj):
    return pd.DataFrame(obj["data"], columns=obj["headers"])


def test_01A():
    subj = nlp_text()
    print(subj)

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_01B():
    subj = nlp_table()

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_01C():
    subj = nlp_document()

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_02A():
    subj = nlp_text()
    print(subj)

    subj.set_text(TEXTS[0])

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_02C():
    subj = nlp_document()
    print(subj)

    for text in TEXTS:
        text_subj = nlp_text()
        text_subj.set_text(text)

        subj.append_text(text_subj)

    res = subj.to_json(set([]))
    print(res)

    assert True


def test_03A():
    subj = nlp_text()
    print(subj)

    subj.set_text(TEXTS[0])

    res = subj.to_json(set([]))
    print(json.dumps(res, indent=2))

    model = init_nlp_model(
        "language;semantic;sentence;term;verb;conn;geoloc;abbreviation"
    )
    model.apply_on_text(subj)

    res = subj.to_json(set([]))
    print("keys: ", res.keys())

    for key in ["properties", "instances", "relations"]:
        if key in res:
            df = to_dataframe(res[key])
            print(df)

    assert True


def test_03C():
    subj = nlp_document()

    for text in TEXTS:
        text_subj = nlp_text()
        text_subj.set_text(text)

        subj.append_text(text_subj)

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
