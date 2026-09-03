//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_WRITER_H_
#define ANDROMEDA_TOOLING_DOCLANG_WRITER_H_

#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

#include <andromeda/tooling/doclang/annotations.h>
#include <andromeda/tooling/doclang/content.h>
#include <andromeda/tooling/doclang/document.h>

namespace andromeda::doclang
{

  struct writer_options
  {
    bool include_annotations = true;
  };

  class writer
  {
  public:

    static bool write_dclx(const std::filesystem::path& path,
                           document& doc,
                           const writer_options& options = writer_options());

    static bool write_dclx_buffer(document& doc,
                                  std::vector<std::byte>& out,
                                  const writer_options& options = writer_options());
  };

  bool writer::write_dclx(const std::filesystem::path& path,
                          document& doc,
                          const writer_options& options)
  {
    std::vector<std::byte> data;
    if(not write_dclx_buffer(doc, data, options))
      {
        return false;
      }

    std::ofstream ofs(path, std::ios::binary);
    if(not ofs)
      {
        doc.set_last_error("could not open file for writing: " + path.string());
        return false;
      }

    ofs.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if(not ofs)
      {
        doc.set_last_error("could not write file: " + path.string());
        return false;
      }

    return true;
  }

  bool writer::write_dclx_buffer(document& doc,
                                 std::vector<std::byte>& out,
                                 const writer_options& options)
  {
    archive zip;
    if(doc.has_archive())
      {
        zip = doc.artifacts();
      }
    else
      {
        zip.set_text("[Content_Types].xml", "<Types></Types>");
        zip.set_text("_rels/.rels", "<Relationships></Relationships>");
      }

    zip.set_text("document.xml", serialize_xml(doc.xml()));

    if(options.include_annotations)
      {
        if(doc.mutable_entities().empty() and not doc.mutable_instances().empty())
          {
            doc.compute_entities();
          }

        zip.set_text(PROPERTIES_CSV, to_properties_csv(doc.mutable_properties()));
        zip.set_text(INSTANCES_CSV, to_instances_csv(doc.mutable_instances()));
        zip.set_text(ENTITIES_CSV, to_entities_csv(doc.mutable_entities()));
        zip.set_text(RELATIONS_CSV, to_relations_csv(doc.mutable_relations()));
        zip.set_text(EDGES_CSV, to_edges_csv(doc.mutable_edges()));
      }
    else
      {
        zip.erase(PROPERTIES_CSV);
        zip.erase(INSTANCES_CSV);
        zip.erase(ENTITIES_CSV);
        zip.erase(RELATIONS_CSV);
        zip.erase(EDGES_CSV);
      }

    if(not zip.write_to_memory(out))
      {
        doc.set_last_error("could not write DocLang archive");
        return false;
      }

    return true;
  }

}

#endif
