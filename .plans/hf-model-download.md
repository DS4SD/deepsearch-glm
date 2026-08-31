# Migrate NLP model downloads from S3 to Hugging Face

## Scope

Replace pretrained NLP model fetching from the S3 object store with fetching
from `docling-project/docling-nlp-models`:

https://huggingface.co/docling-project/docling-nlp-models

Keep training-data downloads from S3 unchanged unless explicitly migrated
separately.

## Current model artifacts

The existing S3 registry declares these eight runtime artifacts:

- `models/crf/part-of-speech/crf_pos_model_en.bin`
- `models/crf/reference/crf_reference.bin`
- `models/crf/ucmi/crf_material.bin`
- `models/fasttext/language/fst_language.bin`
- `models/fasttext/person-name/fst_person_name.bin`
- `models/fasttext/semantic/fst_semantic.bin`
- `models/fasttext/metadata/fst_author.bin`
- `models/rgx/geoloc/rgx_geoloc.json`

All eight are currently present under
`docling_nlp/resources/models/`. The local tokenizer
`models/tok/default-tokenizer.model` is also present but is not part of the
current S3 download list.

## Upload contract

Upload the eight pretrained artifacts to the HF repository while preserving
their existing relative paths. Also upload
`models/tok/default-tokenizer.model` if the HF repository is intended to be
the complete model-assets source.

Before changing runtime code, verify the uploaded repository contains every
expected file and record the chosen revision or commit hash.

The runtime is pinned to revision
`c1be31d505b231948ca432c29ceb5205d9971a77`.

## Implementation

1. Add `huggingface_hub` as a runtime dependency in the relevant dependency
   groups.
2. Replace the S3 URL construction in
   `load_pretrained_nlp_models()` with HF downloads.
3. Keep the existing local target paths and model names unchanged.
4. Preserve `force` and `verbose` behavior:
   - reuse existing local artifacts by default;
   - force refresh when requested;
   - report each downloaded or already-present model.
5. Use HF's cache/download handling rather than manually streaming files with
   `requests`.
6. Fail clearly when a required repository file is missing or a download
   fails.
7. Keep `load_training_data()` on its existing S3 implementation.

## Configuration

Store the HF repository identifier and optional revision in
`models.json`, for example:

```json
{
  "huggingface": {
    "repo-id": "docling-project/docling-nlp-models",
    "revision": "main"
  }
}
```

Prefer a pinned revision for releases, with an explicit configuration or
environment override if development workflows need `main`.

## Tests

- Mock HF downloads and verify all eight model names map to the existing
  resource paths.
- Verify an existing local file is reused when `force=False`.
- Verify `force=True` requests a refresh.
- Verify missing-file and download-error behavior.
- Keep the existing model smoke tests in
  `tests/test_nlp.py`, `tests/test_structs.py`, and `tests/test_glm.py`.
- Run a real repository download smoke test separately when network access is
  available.

## Validation

After implementation:

1. Start from an empty temporary resources directory.
2. Download the models from HF.
3. Confirm all expected files exist at the paths consumed by the native NLP
   engine.
4. Run the existing NLP model-loading and inference tests.
5. Confirm no pretrained-model code path still constructs the S3 NLP URL.
