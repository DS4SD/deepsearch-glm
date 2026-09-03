# Native C++ DocLang Support

## Goal

Make DocLang the canonical native C++ document representation in `docling-nlp`.
The C++ document pipeline should be able to read:

- `.dclg`: a pure DocLang XML file.
- `.dclx`: a zip archive containing `document.xml` plus artifacts such as
  extracted images, page images, relationship files, and content type metadata.

The canonical in-memory representation should be the parsed DocLang XML document,
not the legacy JSON-shaped DoclingDocument/Deep Search document schema. Existing
NLP models can continue to operate on `subject<TEXT>`, `subject<TABLE>`, and
`subject<DOCUMENT>` during the migration, but those types should become
compatibility views/adapters rather than the primary document store.

## Current C++ Situation

The current C++ data structures under `src/andromeda/tooling/structs` are
strongly coupled to the legacy JSON document format:

- `base_subject` makes `to_json()` and `from_json()` abstract core methods.
- `subject<DOCUMENT>` stores the original document as `nlohmann::json orig`.
- `subject<DOCUMENT>` hard-codes legacy labels such as `main-text`,
  `page-dimensions`, `page-elements`, `texts`, `tables`, and `figures`.
- `producer<DOCUMENT>` currently advertises and reads JSON input.
- `text_element`, `table_element`, `page_element`, and `prov_element` are not
  neutral DocLang document elements. They include JSON labels, tokenization
  state, legacy provenance paths, and table-grid assumptions.

This makes the current implementation difficult to adapt cleanly to DocLang:
DocLang is XML-first, while the current model is JSON-first.

## Target Architecture

Introduce an isolated native DocLang module, for example:

```text
src/andromeda/tooling/doclang/
  document.h
  archive.h
  reader.h
  writer.h
  view.h
  adapters.h
```

The module should own all DocLang-specific IO, parsing, archive handling,
document traversal, and mutation APIs. Other parts of the codebase should not
need to know whether the input came from `.dclg`, `.dclx`, or an in-memory XML
buffer.

The intended layering is:

```text
.dclg/.dclx bytes
      |
      v
doclang::Reader / doclang::Archive
      |
      v
doclang::Document  (pugi::xml_document is source of truth)
      |
      v
doclang::View APIs
      |
      v
compatibility adapters to subject<DOCUMENT> for existing NLP models
```

The important design rule is that DocLang XML remains the source of truth.
Conversion to legacy subjects should be an execution view, not a destructive or
canonical import step.

## Step 1: Add Native XML and Zip Dependencies

Add native C++ dependencies to CMake:

- `pugixml` for XML parsing, traversal, modification, and serialization.
- `miniz` for in-memory `.dclx` zip archive reading.

Rationale:

- `pugixml` gives a small C++ DOM-style API and can parse from memory buffers.
- `miniz` fits the existing dependency style because it is small, C/CMake
  friendly, and supports ZIP archive APIs without requiring extraction to disk.

Implementation details:

- Add `cmake/extlib_pugixml.cmake`.
- Add `cmake/extlib_miniz.cmake`.
- Add both dependency targets to `DEPENDENCIES` in `CMakeLists.txt`.
- Include both external dependency files next to the other `extlib_*` includes.
- Keep `USE_SYSTEM_DEPS` support where possible via `pkg-config`.

Acceptance criteria:

- CMake configuration sees `pugixml` and `miniz` targets.
- Existing executable, static library, and pybind module targets link those
  dependencies through the existing `DEPENDENCIES` mechanism.
- No document behavior changes are introduced in this step.

## Step 2: Add DocLang Reader Skeleton

Add a new native reader API without changing existing producers yet.

Proposed files:

```text
src/andromeda/tooling/doclang/document.h
src/andromeda/tooling/doclang/archive.h
src/andromeda/tooling/doclang/reader.h
src/andromeda/tooling/doclang.h
```

Initial API sketch:

```cpp
namespace andromeda::doclang
{
  enum class format
  {
    dclg,
    dclx,
    unknown
  };

  class archive
  {
  public:
    bool has(std::string_view path) const;
    std::string_view text(std::string_view path) const;
    std::span<const std::byte> bytes(std::string_view path) const;
    std::vector<std::string> paths() const;
  };

  class document
  {
  public:
    pugi::xml_document& xml();
    const pugi::xml_document& xml() const;

    bool has_archive() const;
    const archive& artifacts() const;
  };

  class reader
  {
  public:
    static format detect_format(const std::filesystem::path& path);
    static bool read(const std::filesystem::path& path, document& out);
    static bool read_dclg_buffer(std::string_view xml, document& out);
    static bool read_dclx_buffer(std::span<const std::byte> bytes, document& out);
  };
}
```

Reader responsibilities:

- For `.dclg`, read the file bytes and parse directly with pugixml.
- For `.dclx`, read the zip archive bytes into memory, extract `document.xml`
  into memory, parse it with pugixml, and retain artifact entries in memory.
- Do not extract archive contents to a temporary directory.
- Provide clear error messages for missing `document.xml`, invalid XML, invalid
  archive structure, and unsupported file extensions.

Acceptance criteria:

- A `.dclg` XML file can be parsed into `doclang::document`.
- A `.dclx` archive can be parsed into `doclang::document`.
- A `.dclx` archive's `assets/*` and `pages/*` entries remain addressable in
  memory through `doclang::archive`.

## Step 3: Preserve DocLang XML as Canonical Data

DocLang documents can contain mixed XML content, CDATA, and inline structural
markers. For example, a table may contain text interleaved with elements such as
`<fcel/>`, `<lcel/>`, `<ched/>`, and `<nl/>`.

The native model must not flatten or normalize this content at read time.

Rules:

- `pugi::xml_document` is owned by `doclang::document`.
- The original XML node order must be preserved.
- Mixed text nodes and CDATA nodes must remain queryable.
- Element attributes such as heading levels and location values must remain
  queryable without conversion to legacy JSON.
- Table marker nodes must remain part of the XML tree.
- Text extraction APIs may provide normalized text views, but those views are
  derived data.

Acceptance criteria:

- Reading and writing a `.dclg` document does not discard CDATA.
- Reading a table with inline marker elements keeps the marker nodes visible in
  document order.
- Any compatibility conversion to old subjects is explicitly separate from the
  canonical DocLang document object.

## Step 4: Add Read-Only DocLang Query Views

Add view classes that expose common document traversal patterns without exposing
legacy JSON paths or forcing callers to manipulate pugixml directly everywhere.

Proposed concepts:

```cpp
namespace andromeda::doclang
{
  class element_view
  {
  public:
    std::string name() const;
    std::string text_content() const;
    std::optional<unsigned> heading_level() const;
    std::vector<float> location() const;
    pugi::xml_node node() const;
  };

  class document_view
  {
  public:
    std::vector<element_view> body_elements() const;
    std::vector<element_view> elements_by_name(std::string_view name) const;
    std::vector<element_view> text_like_elements() const;
    std::vector<element_view> table_elements() const;
  };
}
```

Design principles:

- Views are lightweight and non-owning.
- Views reference `doclang::document`; they do not copy the XML tree.
- Views should not expose old labels such as `main-text` or `page-elements`.
- Views can offer derived helpers for text extraction and locations.
- Mutation APIs should be added later and separately.

Acceptance criteria:

- Callers can iterate text-like elements from a DocLang XML document.
- Callers can access heading level and location metadata from XML.
- Callers can inspect table XML without conversion to legacy table grids.

## Step 5: Keep Old Subjects as Compatibility NLP Views

Existing NLP models operate on:

- `subject<TEXT>`
- `subject<TABLE>`
- `subject<FIGURE>`
- `subject<DOCUMENT>`

Those types should remain functional during the migration. The bridge should be
explicit:

```cpp
namespace andromeda::doclang
{
  class subject_adapter
  {
  public:
    static bool to_subject_document(const document& in,
                                    subject<DOCUMENT>& out,
                                    adapter_options options = {});
  };
}
```

Initial adapter behavior:

- Convert text-like DocLang XML elements into `subject<TEXT>` objects.
- Preserve a back-reference from each subject to its DocLang XML location.
- Convert basic locations into existing `prov_element` only where needed by
  existing NLP code.
- Convert tables conservatively. If full table semantics are not available yet,
  preserve table XML as payload and expose a best-effort text view.
- Keep the legacy `subject<DOCUMENT>` fields populated enough for current
  `base_nlp_model::apply(subject<DOCUMENT>&)` to work.

Important constraint:

The adapter must not replace the canonical XML document. It is a temporary NLP
execution view.

Acceptance criteria:

- Existing NLP models can run on a `.dclg`/`.dclx` document through the adapter.
- NLP output can be mapped back to DocLang XML elements using retained
  references.
- Existing JSON input behavior remains unchanged.

## Step 6: Move Serialization Out of Core Subjects

The current `base_subject` interface requires all subjects to implement JSON IO:

```cpp
virtual nlohmann::json to_json(...) = 0;
virtual bool from_json(...) = 0;
```

This makes JSON a core domain-model dependency. Long term, that should become an
explicit serializer boundary.

Migration approach:

1. Do not remove `to_json()` / `from_json()` immediately.
2. Add explicit serializer/adapters for legacy JSON:

   ```text
   src/andromeda/tooling/serializers/legacy_json/
   ```

3. Move legacy labels and conversion logic into the serializer/adapter layer.
4. Keep deprecated forwarding methods on subjects until downstream callers have
   migrated.
5. Eventually narrow `base_subject` to NLP state and subject identity rather than
   document IO.

Target separation:

- DocLang IO lives in `tooling/doclang`.
- Legacy JSON IO lives in `tooling/serializers/legacy_json`.
- NLP annotations live in `tooling/structs/items`.
- NLP execution views live in `tooling/structs/subjects`.

Acceptance criteria:

- New DocLang code does not depend on legacy JSON labels.
- Legacy JSON behavior remains available through an explicit compatibility path.
- The subject API can evolve without changing the canonical document format.

## Step 7: De-Emphasize Legacy `elements/*.h`

The current `andromeda/tooling/structs/elements/*.h` files should not be treated
as the native document model.

Recommended disposition:

- `text_element`
  Keep temporarily as an NLP token container. It owns normalized text,
  char tokens, and word tokens, which are useful for models, but it should not
  represent a DocLang XML element.

- `table_element`
  Keep temporarily for legacy table NLP behavior. Do not use it as the canonical
  representation of DocLang tables. A DocLang table view should expose the XML
  table node and marker stream directly.

- `page_element`
  Replace with DocLang page/location/artifact views where possible. It can stay
  as an adapter detail for legacy subject conversion.

- `prov_element`
  Replace with DocLang-native references and location views where possible. It
  can stay as an adapter detail for current NLP models that expect provenance.

Acceptance criteria:

- New DocLang code does not add new dependencies on `elements/*.h`.
- Existing NLP models continue to compile and run.
- Each remaining use of `elements/*.h` is either legacy JSON compatibility or
  NLP adapter state.

## Test Strategy

Add tests incrementally as each step lands.

Recommended initial fixtures:

- A tiny `.dclg` file with:
  - root `<doclang>`
  - one heading
  - one text element
  - one element with CDATA
  - one location block

- A tiny `.dclx` file with:
  - `[Content_Types].xml`
  - `_rels/.rels`
  - `document.xml`
  - one `assets/*` entry
  - one `pages/*` entry

- A table-focused `.dclg` fixture with inline marker elements:
  - `<fcel/>`
  - `<lcel/>`
  - `<ched/>`
  - `<nl/>`

Test stages:

1. Dependency build smoke test.
2. `.dclg` reader parses valid XML and rejects invalid XML.
3. `.dclx` reader extracts `document.xml` in memory and exposes artifacts.
4. XML mixed content remains visible in node order.
5. Query views return expected text-like and table-like elements.
6. Subject adapter produces a legacy `subject<DOCUMENT>` that existing NLP
   models can process.
7. Back-reference mapping from NLP output to DocLang element identity is stable.

## Migration Rules

- Do not convert DocLang XML into legacy JSON as the first step of reading.
- Do not extract `.dclx` archives to disk for normal processing.
- Keep existing JSON-based public behavior working while native DocLang support
  is introduced.
- Keep DocLang-specific code isolated from model implementations where possible.
- Add mutation/write APIs only after the read/query/adapter path is stable.
- Prefer small, reviewable steps with tests after each behavior change.

## Open Questions

- What is the canonical way to identify DocLang elements for stable
  back-references when the XML does not carry explicit IDs?
- Should `.dclx` writing preserve original archive relationship files exactly,
  or regenerate them from the current document/artifact set?
- How much table semantics should the first adapter expose to old
  `subject<TABLE>` consumers?
- Should DocLang mutation APIs operate directly on pugixml nodes, or through
  typed command objects to preserve invariants?
