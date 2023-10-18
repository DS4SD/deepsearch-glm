
from deepsearch_glm.nlp_utils import list_nlp_model_configs, init_nlp_model
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models

def test_1():
    x = 1
    y = 1
    assert x==y

def test_2():
    models = load_pretrained_nlp_models()
    print(f"models: {models}")

    assert "language" in models
    assert "semantic" in models
    assert "name" in models
    assert "reference" in models
    
def test_3():

    model = init_nlp_model("sentence;language;term")
    res = model.apply_on_text("FeSe is a material.")

    assert "text" in res

    assert "properties" in res
    assert "instances" in res
