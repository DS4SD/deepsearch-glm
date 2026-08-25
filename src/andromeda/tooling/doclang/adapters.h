//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_ADAPTERS_H_
#define ANDROMEDA_TOOLING_DOCLANG_ADAPTERS_H_

#include <sstream>
#include <string>
#include <string_view>

#include <andromeda/tooling/doclang/document.h>
#include <andromeda/tooling/doclang/view.h>
#include <andromeda/utils.h>
#include <andromeda/enums.h>
#include <andromeda/tooling/base_types.h>
#include <andromeda/tooling/structs.h>

namespace andromeda::doclang
{

  struct adapter_options
  {
    bool include_tables_as_text = true;
    std::string document_name = "";
  };

  class subject_adapter
  {
  public:

    static bool to_subject_document(const document& in,
                                    subject<DOCUMENT>& out,
                                    const adapter_options& options = {});

  private:

    static bool should_adapt_as_text(pugi::xml_node node,
                                     const adapter_options& options);

    static std::string doclang_path(std::size_t index);

    static std::string compatibility_type(pugi::xml_node node);

    static nlohmann::json payload_for(pugi::xml_node node,
                                      std::string_view path);
  };

  bool subject_adapter::to_subject_document(const document& in,
                                            subject<DOCUMENT>& out,
                                            const adapter_options& options)
  {
    if(not in.root())
      {
        return false;
      }

    out.clear();

    if(options.document_name.empty())
      {
        const std::filesystem::path& source = in.get_source_path();
        if(source.empty())
          {
            out.set_name("doclang-document");
          }
        else
          {
            out.set_name(source.string());
          }
      }
    else
      {
        out.set_name(options.document_name);
      }

    std::size_t adapted_index = 0;
    in.iterate_elements([&](pugi::xml_node node)
    {
      if(not should_adapt_as_text(node, options))
        {
          return;
        }

      std::string text = node.child_value();
      if(text.empty())
        {
          return;
        }

      const std::string path = doclang_path(adapted_index);
      auto text_subject = std::make_shared<subject<TEXT> >(out.get_hash(), path);

      if(not text_subject->set_text(text))
        {
          return;
        }

      text_subject->set_type(compatibility_type(node));
      text_subject->payload = payload_for(node, path);

      out.push_back(text_subject);
      adapted_index += 1;
    });

    return true;
  }

  bool subject_adapter::should_adapt_as_text(pugi::xml_node node,
                                             const adapter_options& options)
  {
    const std::string_view name = node.name();

    if(name=="table")
      {
        return options.include_tables_as_text;
      }

    return (
      name=="text" or
      name=="heading" or
      name=="page_header" or
      name=="page_footer" or
      name=="footnote" or
      name=="caption" or
      name=="list" or
      name=="formula" or
      name=="equation" or
      name=="code"
    );
  }

  std::string subject_adapter::doclang_path(std::size_t index)
  {
    std::stringstream ss;
    ss << "#/doclang/" << index;
    return ss.str();
  }

  std::string subject_adapter::compatibility_type(pugi::xml_node node)
  {
    const std::string_view name = node.name();

    if(name.empty())
      {
        return "text";
      }

    return std::string(name);
  }

  nlohmann::json subject_adapter::payload_for(pugi::xml_node node,
                                              std::string_view path)
  {
    nlohmann::json payload = nlohmann::json::object({});

    payload["doclang_path"] = std::string(path);
    payload["doclang_name"] = node.name();

    if(node.attribute("level"))
      {
        payload["doclang_heading_level"] = node.attribute("level").as_uint();
      }

    nlohmann::json locations = nlohmann::json::array({});
    for(pugi::xml_node loc:node.children("location"))
      {
        if(loc.attribute("value"))
          {
            locations.push_back(loc.attribute("value").as_double());
          }
      }

    if(not locations.empty())
      {
        payload["doclang_location"] = locations;
      }

    return payload;
  }

}

#endif
