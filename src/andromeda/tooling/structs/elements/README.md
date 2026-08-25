# Legacy Element Structures

The headers in this directory are legacy NLP/JSON adapter structures. They are
not the canonical DocLang document model.

Current allowed uses:

- Existing NLP tokenization and model execution paths.
- Legacy JSON compatibility.
- Temporary compatibility adapters from DocLang XML to existing `subject<T>`
  types.

New native DocLang reader, archive, document, content, and view code should not
depend on these headers.

## Current Roles

- `text_element`
  Stores normalized text and tokenization state used by existing NLP models.

- `table_element`
  Stores the legacy JSON-style table grid cell representation used by existing
  table NLP behavior.

- `page_element`
  Stores legacy page dimension data for JSON-compatible document subjects.

- `prov_element`
  Stores legacy provenance and JSON-reference style locations used by existing
  subject/model code.

These types can remain while the NLP execution layer still needs them, but they
should be treated as compatibility implementation details.
