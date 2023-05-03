# Graph Language Models

## Install

```sh
sh build.mac.sh
```

## National Language Processing (NLP)

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



