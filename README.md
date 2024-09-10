# Graph Language Models

![build](https://github.com/DS4SD/deepsearch-glm/actions/workflows/cmake.yml/badge.svg)
![tests](https://github.com/DS4SD/deepsearch-glm/actions/workflows/tests.yml/badge.svg)

[![License MIT](https://img.shields.io/github/license/ds4sd/deepsearch-glm)](https://opensource.org/licenses/MIT)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

[![PyPI version](https://img.shields.io/pypi/v/deepsearch-glm)](https://pypi.org/project/deepsearch-glm/)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/deepsearch-glm)](https://pypi.org/project/deepsearch-glm/)

![PyPI - Downloads](https://img.shields.io/pypi/dm/deepsearch-glm)

## Getting Started

### Finding entities and relations via NLP on text and documents

To get easily started, simply install the `deepsearch-glm` package from PyPi. This can be
done using the traditional `pip install deepsearch-glm` or via poetry `poetry add deepsearch-glm`.

Below, you can find the code-snippet to process pieces of text,

```python
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models
from deepsearch_glm.nlp_utils import init_nlp_model, print_on_shell

load_pretrained_nlp_models(force=False, verbose=False)
mdl = init_nlp_model()

# from Wikipedia (https://en.wikipedia.org/wiki/France)
text = """
France (French: [fʁɑ̃s] Listen), officially the French Republic
(French: République française [ʁepyblik fʁɑ̃sɛz]),[14] is a country
located primarily in Western Europe. It also includes overseas regions
and territories in the Americas and the Atlantic, Pacific and Indian
Oceans,[XII] giving it one of the largest discontiguous exclusive
economic zones in the world.
"""

res = mdl.apply_on_text(text)
print_on_shell(text, res)
```

The last command will print the pandas dataframes on the shell and provides the
following output,

```sh
text:

   #France (French: [fʁɑ̃s] Listen), officially the French Republic
(French: République française [ʁepyblik fʁɑ̃sɛz]),[14] is a country
located primarily in Western Europe. It also includes overseas regions
and territories in the Americas and the Atlantic, Pacific and Indian
Oceans, giving it one of the largest discontiguous exclusive economic
zones in the world.

properties:

         type label  confidence
0  language    en    0.897559

instances:

  type         subtype               subj_path      char_i    char_j  original
-----------  --------------------  -----------  --------  --------  ---------------------------------------------------------------------
sentence                           #                   1       180  France (French: [fʁɑ̃s] Listen), officially the French Republic
                                                                    (French: République française [ʁepyblik fʁɑ̃sɛz]),[14] is a country
                                                                    located primarily in Western Europe.
term         single-term           #                   1         8  #France
expression   wtoken-concatenation  #                   1         8  #France
parenthesis  round brackets        #                   9        36  (French: [fʁɑ̃s] Listen)
expression   wtoken-concatenation  #                  18        28  [fʁɑ̃s]
term         single-term           #                  29        35  Listen
term         single-term           #                  53        68  French Republic
parenthesis  round brackets        #                  69       125  (French: République française [ʁepyblik fʁɑ̃sɛz])
term         single-term           #                  78       100  République française
term         single-term           #                 112       124  fʁɑ̃sɛz]
parenthesis  reference             #                 126       130  [14]
numval       ival                  #                 127       129  14
term         single-term           #                 136       143  country
term         single-term           #                 165       179  Western Europe
sentence                           #                 181       373  It also includes overseas regions and territories in the Americas and
                                                                    the Atlantic, Pacific and Indian Oceans, giving it one of the largest
                                                                    discontiguous exclusive economic zones in the world.
term         single-term           #                 198       214  overseas regions
term         enum-term-mark-3      #                 207       230  regions and territories
term         single-term           #                 219       230  territories
term         single-term           #                 238       246  Americas
term         enum-term-mark-4      #                 255       290  Atlantic, Pacific and Indian Oceans
term         single-term           #                 255       263  Atlantic
term         single-term           #                 265       272  Pacific
term         single-term           #                 277       290  Indian Oceans
term         single-term           #                 313       359  largest discontiguous exclusive economic zones
term         single-term           #                 367       372  world
```

The NLP can also be applied on entire documents which were converted using
Deep Search. A simple example is shown below,

```python
from deepsearch_glm.utils.load_pretrained_models import load_pretrained_nlp_models
from deepsearch_glm.nlp_utils import init_nlp_model, print_on_shell

load_pretrained_nlp_models(force=False, verbose=False)
mdl = init_nlp_model()

with open("<path-to-json-file-of-converted-pdf-doc>", "r") as fr:
    doc = json.load(fr)

enriched_doc = mdl.apply_on_doc(doc)
```

### Creating Graphs from NLP entities and relations in document collections

To create graphs, you need two ingredients, namely,

1. a collection of text or documents
2. a set of NLP models that provide entities and relations

Below is a code snippet to create the graph using these basic ingredients,

```python
odir = "<ouput-dir-to-save-graph>"
json_files = ["json-file of converted PDF document"]
model_names = "<list of NLP models:langauge;term;verb;abbreviation>"

glm = create_glm_from_docs(odir, json_files, model_names)	
```

### Querying Graphs 

TBD

## Install for development

### Python installation

To use the python interface, first make sure all dependencies are installed. We use [poetry](https://python-poetry.org/docs/)
for that. To install all the dependent python packages and get the python bindings, simply execute,

```sh
poetry install --all-extras
```

### CXX compilation

To compile from scratch, simply run the following command in the `deepsearch-glm` root folder to
create the `build` directory,

```sh
cmake -B ./build; 
```

Next, compile the code from scratch,

```sh
cmake --build ./build -j
```

## Run using the Python Interface

### NLP and GLM examples

_Note: Some of the examples require to convert PDF documents with Deep Search. For this to run, it is required to install the [deepsearch-toolkit](https://github.com/DS4SD/deepsearch-toolkit) package. You can install it with `pip install deepsearch-glm[toolkit]`._

To run the examples, simply do execute the scripts as `poetry run python <script> <input>`. Examples are,

1. **apply NLP on document(s)**
```sh
poetry run python ./deepsearch_glm/nlp_apply_on_docs.py --pdf './data/documents/articles/2305.*.pdf' --models 'language;term'
```
2. **analyse NLP on document(s)**
```sh
poetry run python ./deepsearch_glm/nlp_apply_on_docs.py --json './data/documents/articles/2305.*.nlp.json' 
```
3. **create GLM from document(s)**
```sh
poetry run python ./deepsearch_glm/glm_create_from_docs.py --pdf ./data/documents/reports/2022-ibm-annual-report.pdf
```

### Deep Search utilities

To use the Deep Search capabilities, it is required to install the [deepsearch-toolkit](https://github.com/DS4SD/deepsearch-toolkit) package.
You can install it with `pip install deepsearch-glm[toolkit]`.


1. **Query and download document(s)**
```sh
poetry run python ./deepsearch_glm/utils/ds_query.py --index patent-uspto --query "\"global warming potential\" AND \"etching\""
```
2. **Converting PDF document(s) into JSON**
```sh
poetry run python ./deepsearch_glm/utils/ds_convert.py --pdf './data/documents/articles/2305.*.pdf'"
```

## Run using CXX executables

If you like to be bare-bones, you can also use the executables for NLP and GLM's directly. In general, we
follow a simple scheme of the form

```sh
./nlp.exe -m <mode> -c <JSON-config file>
./glm.exe -m <mode> -c <JSON-config file>
```

In both cases, the modes can be queried directly via the `-h` or `--help`

```sh
./nlp.exe -h
./glm.exe -h
```

and the configuration files can be generated,

```sh
./nlp.exe -m create-configs
./glm.exe -m create-configs
```

### Natural Language Processing (NLP)

After you have generated the configuration files (see above), you can

1. train simple NLP models
```sh
./nlp.exe -m train -c nlp_train_config.json
```
2. leverage pre-trained models
```sh
./nlp.exe -m predict -c nlp_predict.example.json
```

### Graph Language Models (GLM)

1. create a GLM
```sh
./glm.exe -m create -c glm_config_create.json
```
2. explore interactively the GLM
```sh
./glm.exe -m explore -c glm_config_explore.json
```

## Testing

To run the tests, simply execute (after installation),

```sh
poetry run pytest ./tests -vvv -s
```