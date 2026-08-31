# Remove `deepsearch-toolkit`

## Goal

Remove the `deepsearch-toolkit` integration from the package, including dependency metadata, import-time coupling, CLI/docs references, and lockfile state.

## Scope

1. Remove Deep Search integration modules:
   - `docling_nlp/utils/ds_utils.py`
   - `docling_nlp/utils/ds_query.py`
2. Keep JSON-based document workflows working:
   - `docling_nlp/nlp_apply_on_docs.py`
   - `docling_nlp/glm_create_from_docs.py`
3. Remove Deep Search retrieval from semantic classifier training while preserving local JSON prepare/train/eval/refine flows.
4. Update packaging:
   - remove the `toolkit` extra
   - keep `all` consistent as the union of remaining extras
5. Update README examples to use JSON documents and remove Deep Search installation/query/conversion instructions.
6. Rename executable Deep Search-era environment variables used by utilities.
7. Regenerate `uv.lock`.

## Validation

- Search for removed integration references:
  - `deepsearch-toolkit`
  - `docling-nlp[toolkit]`
  - `from docling_nlp.utils.ds_utils`
  - `ds_query`
  - `convert_pdffiles`
- Run `uv lock`.
- Run targeted import/help checks for changed modules.

## Non-goals

- Do not rewrite historical changelog entries.
- Do not rename historical model/data storage prefixes unless they are executable integration points.
