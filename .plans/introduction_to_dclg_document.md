# Introduction of `dclg_document`

## Objective

Split the current native DocLang model into a sidecar-free DCLG base and a
DCLX-specific derived type. The resulting ownership hierarchy is:

```text
dclg_document
  raw DCLG/XML bytes, parsed pugixml DOM, XML traversal, XPath/text access

dclx_document : dclg_document
  archive, source path, CSV annotations, NLP state, BibTeX, DCLG sidecars
```

The Python hierarchy must mirror this design:

```text
DoclangDocument
  shared_ptr<dclg_document>

DocLangXDocument : DoclangDocument
  shared_ptr<dclx_document>, shared with the base wrapper
```

## Non-negotiable invariants

1. `dclg_document` owns no archive, path, CSV vector, NLP model result,
   BibTeX content, or sidecar.
2. `dclg_document` owns exactly one raw DCLG string and exactly one
   `pugi::xml_document` parsed from that string.
3. A successful parse updates both raw and parsed state. A failed parse must
   leave the object empty and expose a useful error message.
4. `dclx_document` has no second PugiXML DOM for `document.xml`; inherited
   DCLG state is the sole primary-document XML owner.
5. Summary, ToC, and concepts are optional parsed `dclg_document` objects,
   not unvalidated strings.
6. The writer serialises the DCLG string owned by each parsed sidecar. The
   reader parses every DCLG archive entry before exposing it.
7. Existing `.dclx` files with none of the new sidecars remain valid.

## Phase 1: complete `dclg_document`

`src/andromeda/tooling/doclang/dclg_document.h` must become the complete XML
layer. Add these public operations:

```cpp
static hash_type hash(std::string_view text);
void clear();
bool read(std::string_view dclg);
bool valid() const;
const std::string& raw() const;
pugi::xml_document& xml();
const pugi::xml_document& xml() const;
pugi::xml_node root();
pugi::xml_node root() const;
iterate_elements(...);
iterate_table_text(...);
iterate_picture_text(...);
std::string at(std::string_view xpath, std::string_view mode="auto");
const std::string& get_last_error() const;
void set_last_error(std::string);
```

- [x] `clear()`
- [x] `read(std::string_view)`
- [x] `valid()`
- [x] `raw()`
- [x] `xml()` / `const xml()`
- [x] `root()` / `const root()`
- [x] `get_last_error()`
- [x] `static hash(std::string_view)`
- [x] `set_last_error(std::string)`
- [x] `iterate_elements(...)` (both overloads)
- [x] `iterate_table_text(...)` (both overloads)
- [x] `iterate_picture_text(...)` (both overloads)
- [x] `at(xpath, mode)`

Move or copy the XML-only helpers currently in `dclx_document.h`:

- [x] element iteration by direct `<doclang>` child and by name;
- [x] table text extraction;
- [x] picture/figure text extraction;
- [x] `node_text_content`, `picture_text_content`, and XPath lookup handling;
- [x] XML node serialisation and text-mode behaviour.

Do **not** remove the duplicate DCLX methods in this phase.

- [x] Add direct C++ tests for valid DCLG, invalid XML, missing `<doclang>`,
      raw-string retention, direct element iteration, table text, picture
      text, and `at()` modes (`app/test_unit_dclg_document.cpp`).

## Phase 2: remove duplicate DCLX XML state

- [x] Make `dclx_document` inherit `dclg_document`.
- [x] Remove its `xml_doc`, XML root/accessor methods, XML traversal methods,
      XPath method, XML error field, and XML helper implementations.
- [x] Its `clear()` must call `dclg_document::clear()` before clearing only
      DCLX state.
- [x] Update helper signatures that merely inspect XML to accept
      `const dclg_document&`; keep NLP and archive APIs typed as
      `dclx_document&`.

Update `reader` to expose:

```cpp
read_dclg_buffer(std::string_view, dclg_document&);
read_dclx_buffer(std::span<const std::byte>, dclx_document&);
```

- [x] `read_dclg_buffer` takes `dclg_document&`.
- [x] `read_dclx_buffer` takes `dclx_document&`.
- [x] The DCLX reader parses `document.xml` through the inherited DCLG parser,
      then attaches its archive and loads annotations.
- [x] Preserve the original `document.xml` bytes as the primary document's raw
      DCLG string.

## Phase 3: parsed DCLG sidecars

- [x] Replace `optional<string>` summary, ToC, and concepts fields with
      optional `shared_ptr<dclg_document>` fields.
- [x] Use a small common helper that parses a sidecar, forwards parse errors to
      the owning DCLX document, then performs sidecar-specific validation.

- [x] Summary: valid DCLG root only.
- [x] ToC: one `<toc>`; every `<entry>` has non-empty `xpath` and exactly one
      `<description>`.
- [x] Concepts: one `<concepts>`; every `<concept>` has one `<header>` and at
      most one each of `<abbreviation>`, `<description>`, and `<table>`.

- [x] Getters return optional shared pointers.
- [x] Clearing resets the optional.
- [x] Writer uses `sidecar->raw()`; reader creates parsed sidecars.
- [x] Tests cover absent, valid, invalid, replacement, clearing, and DCLX round
      trips.

## Phase 4: pybind migration

- [x] `DoclangDocument` owns `shared_ptr<dclg_document>` and exposes DCLG
      parsing, status, raw XML, errors, `elements()`, and `__iter__`.
      Iterator items remain `{name, xml, text}` dictionaries.
- [x] `DocLangXDocument` inherits `DoclangDocument` in C++ and pybind. It owns
      a `shared_ptr<dclx_document>` and passes the same allocation to its base
      as a `shared_ptr<dclg_document>`.
- [x] Remove duplicate XML methods from the derived wrapper.
- [x] `document_summary()`, `toc()`, and `concepts()` return optional
      `DoclangDocument` wrappers sharing the parsed sidecar allocation.
- [x] Keep Python `summary()` as the count/status dictionary; use
      `document_summary()` for the summary sidecar to avoid a name collision.

## Migration and verification order

1. [x] Build after Phase 1; run DCLG-only native tests.
2. [x] Build after Phase 2; run all native DocLang tests and `nlp-on-dclx` tests.
3. [x] Build after Phase 3; run archive round-trip tests.
4. [x] Build after Phase 4; run `tests/test_doclang.py` against the built module.
5. [x] Run `git diff --check` and inspect public-header include dependencies.

No phase may introduce a second owner for the primary XML DOM, expose a
sidecar through `DoclangDocument`, or leave a raw-string/DOM mismatch after a
parse failure.

## Implementation notes

Two deliberate deviations from the phase text, and one drive-by fix:

- **`at()` sits on `DoclangDocument`, not only on `DocLangXDocument`.** The
  Phase 4 surface list for `DoclangDocument` omits `at()`, but Phase 2 makes
  the DCLG layer the sole owner of XPath access, and Phase 4 requires the
  derived wrapper to carry no duplicate XML methods. Putting `at()` on the base
  satisfies both; `DocLangXDocument.at()` still resolves, by inheritance.
- **`DoclangDocument.xml()` returns `raw()`.** The wrapper no longer keeps its
  own copy of the DCLG string, so "raw XML" is the base's retained buffer. The
  DCLX writer still serialises the DOM for `document.xml`, so DOM edits keep
  round-tripping.
- **`reader::read()` no longer loses the source path on `.dclx`.** It used to
  call `set_source_path()` before `read_dclx_buffer()`, whose `clear()` wiped
  it again. The path is now set after the format dispatch.

`app/test_unit_doclang_reader.cpp` still referred to the deleted
`andromeda::doclang::document`, so its target did not compile at the start of
this work; it now uses `dclx_document` / `dclg_document`.
