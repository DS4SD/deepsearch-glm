# Graph Language Models

[![PyPI version](https://img.shields.io/pypi/v/deepsearch-glm)](https://pypi.org/project/deepsearch-glm/)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/deepsearch-glm)](https://pypi.org/project/deepsearch-glm/)
[![License MIT](https://img.shields.io/github/license/ds4sd/deepsearch-glm)](https://opensource.org/licenses/MIT)

## Install

### CXX compilation

To compile from scratch, simply run the following command in the `deepsearch-glm` root folder, 

```sh
cmake -B ./build; cd build; make install -j
```

### Python installation



## Natural Language Processing (NLP)

```sh
./nlp.exe -m create-configs
```

```sh
mv nlp_<task>.example.json nlp_<task>.json
```

### Train

```sh
./nlp.exe -m train -c nlp_train_config.json
```

### Predict

```sh
./nlp.exe -m predict -c nlp_predict.example.json
```

## Graph Language Models (GLM)

We describe the GLM approach in detail in the [docs](./docs/glm/glm.md). To get started quickly, simply run,

```sh
./glm.exe -m create-configs
```

```sh
mv glm_config_<task>.example.json glm_config_<task>.json
```

```sh
./glm.exe -m create -c glm_config_create.json
```

```sh
./glm.exe -m distill -c glm_config_create.json
```

```sh
./glm.exe -m explore -c glm_config_create.json
```

## Python Interface

To use the python interface, first make sure all dependencies are installed. We use [poetry](https://python-poetry.org/docs/) for that,

```sh
poetry install
```

To run the examples, simply do execute the scripts as `poetry run python <script> <input>`. Examples are,

1. Doing NLP on a single document
```sh
poetry run python3 ./deepsearch_glm/nlp_doc.py -m run-doc -i ../../Articles-v2/2302.05420.json --vpage 10
```
2. Creating a GLM from a single document
```sh
poetry run python ./deepsearch_glm/glm_doc.py --pdf ./data/documents/reports/2022-ibm-annual-report.pdf
```
3. Using a GLM for Q&A:
```sh
poetry run python ./deepsearch_glm/glm_docqa.py --pdf ./data/documents/reports/2022-ibm-annual-report.pdf
```
