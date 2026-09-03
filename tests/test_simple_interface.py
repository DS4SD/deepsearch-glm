from docling_nlp.andromeda_nlp import nlp_model
from docling_nlp.andromeda_structs import ds_document, ds_text


def make_document():
    text = ds_text()
    text.set_text("FeSe is a material.")

    doc = ds_document()
    doc.set_title("Simple interface document")
    doc.append_text(text)

    return doc


def test_simple_interface_v1():
    model = nlp_model()
    model.set_loglevel("WARNING")

    config = model.get_apply_configs()[0]
    config["models"] = ""
    config["subject-filters"] = []

    model.initialise(config)

    doc = make_document()
    output = model.apply_on_doc(doc)
    assert output


def test_simple_interface_v2():
    model = nlp_model(loglevel="warning", text_ordering=True)

    doc = make_document()
    output = model.apply_on_doc(doc)
    assert output
