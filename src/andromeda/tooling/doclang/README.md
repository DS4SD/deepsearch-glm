# Native DocLang Module

This module owns native C++ DocLang support.

## Canonical Representation

`andromeda::doclang::document` owns the canonical parsed XML document through
`pugi::xml_document`.

DocLang XML is the source of truth. Compatibility conversions to legacy
`subject<T>` data structures are execution views for existing NLP models, not
the canonical document model.

## Boundary Rules

Core DocLang headers should stay independent of legacy subject and element
structures:

- `archive.h`
- `content.h`
- `document.h`
- `reader.h`
- `view.h`

These files may depend on:

- the C++ standard library
- `pugixml`
- `miniz`

They should not include:

- `andromeda/tooling/structs.h`
- `andromeda/tooling/structs/subjects.h`
- `andromeda/tooling/structs/elements.h`
- individual legacy element headers

`adapters.h` is the explicit exception. It may include legacy structs because
its job is to create compatibility views for existing NLP code.

## Traversal

Prefer callback-based traversal on `doclang::document` when possible:

```cpp
doc.iterate_elements([](pugi::xml_node node)
{
  // inspect node without allocating a vector of views
});
```

Use `document_view` when callers benefit from convenience collections such as
`text_like_elements()` or `table_elements()`.

DocLang tables encode cell content as XML text nodes between marker elements
such as `<fcel/>`, `<ched/>`, and `<nl/>`. Use `iterate_table_text()` when code
needs those table text fragments directly:

```cpp
doc.iterate_table_text([](pugi::xml_node table, pugi::xml_node text_node)
{
  // text_node.value() is the table text fragment.
});
```

DocLang pictures and figures can contain text-bearing children such as
`<caption>`, `<list>`, and `<text>`, plus artifact children such as `<src/>`.
Use `iterate_picture_text()` to process the text-bearing descendants without
flattening the whole picture:

```cpp
doc.iterate_picture_text([](pugi::xml_node picture, pugi::xml_node text_node)
{
  // text_node is a text-bearing descendant of picture.
});
```

## Archive Handling

`.dclx` archives should be read from bytes and extracted in memory. Normal
processing should not require temporary files or extraction directories.
