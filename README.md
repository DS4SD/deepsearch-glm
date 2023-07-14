# Graph Language Models

![build](https://github.com/DS4SD/deepsearch-glm/actions/workflows/cmake.yml/badge.svg)
[![PyPI version](https://img.shields.io/pypi/v/deepsearch-glm)](https://pypi.org/project/deepsearch-glm/)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/deepsearch-glm)](https://pypi.org/project/deepsearch-glm/)
[![License MIT](https://img.shields.io/github/license/ds4sd/deepsearch-glm)](https://opensource.org/licenses/MIT)

## Install

### Python installation

To use the python interface, first make sure all dependencies are installed. We use [poetry](https://python-poetry.org/docs/)
for that. To install all the dependent python packages and get the python bindings, simply execute,

```sh
poetry install
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

