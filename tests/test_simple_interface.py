import json

from deepsearch_glm.andromeda_nlp import nlp_model


def test_simple_interface_v1():
    model = nlp_model()
    model.set_loglevel("WARNING")

    config = model.get_apply_configs()[0]
    config["models"] = ""
    config["subject-filters"] = []

    model.initialise(config)

    doc = json.load(open("tests/data/docs/1806.02284.json"))
    output = model.apply_on_doc(doc)

def test_simple_interface_v2():
    model = nlp_model(loglevel="warning", text_ordering=True)

    doc = json.load(open("tests/data/docs/1806.02284.json"))
    output = model.apply_on_doc(doc)
    
