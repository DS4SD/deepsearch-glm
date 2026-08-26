//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_DOCUMENT_H_
#define ANDROMEDA_TOOLING_DOCLANG_DOCUMENT_H_

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <pugixml.hpp>

#include <andromeda/utils.h>
#include <andromeda/enums.h>
#include <andromeda/tooling/base_types.h>
#include <andromeda/tooling/structs/tokens.h>
#include <andromeda/tooling/structs/items.h>
#include <andromeda/tooling/doclang/archive.h>
#include <andromeda/tooling/doclang/content.h>

namespace andromeda::doclang
{

  class document
  {
  public:

    document() = default;

    void clear();

    pugi::xml_document& xml() { return xml_doc; }
    const pugi::xml_document& xml() const { return xml_doc; }

    pugi::xml_node root() { return xml_doc.child("doclang"); }
    pugi::xml_node root() const { return xml_doc.child("doclang"); }

    template<typename callback_type>
    void iterate_elements(callback_type&& callback) const;

    template<typename callback_type>
    void iterate_elements(std::string_view name, callback_type&& callback) const;

    template<typename callback_type>
    void iterate_table_text(callback_type&& callback) const;

    template<typename callback_type>
    void iterate_table_text(pugi::xml_node table, callback_type&& callback) const;

    template<typename callback_type>
    void iterate_picture_text(callback_type&& callback) const;

    template<typename callback_type>
    void iterate_picture_text(pugi::xml_node picture, callback_type&& callback) const;

    void set_source_path(std::filesystem::path path) { source_path = std::move(path); }
    const std::filesystem::path& get_source_path() const { return source_path; }

    bool has_archive() const { return artifact_archive.has_value(); }

    archive& artifacts() { return artifact_archive.value(); }
    const archive& artifacts() const { return artifact_archive.value(); }

    void set_archive(archive value) { artifact_archive = std::move(value); }
    void clear_archive() { artifact_archive.reset(); }

    std::shared_ptr<std::vector<base_property> > shared_properties() { return properties; }
    std::shared_ptr<std::vector<base_instance> > shared_instances() { return instances; }
    std::shared_ptr<std::vector<base_relation> > shared_relations() { return relations; }

    std::vector<base_property>& mutable_properties() { return *properties; }
    std::vector<base_instance>& mutable_instances() { return *instances; }
    std::vector<base_relation>& mutable_relations() { return *relations; }

    const std::vector<base_property>& get_properties() const { return *properties; }
    const std::vector<base_instance>& get_instances() const { return *instances; }
    const std::vector<base_relation>& get_relations() const { return *relations; }

    bool has_annotations() const;
    void clear_annotations();

    const std::string& get_last_error() const { return last_error; }
    void set_last_error(std::string msg) { last_error = std::move(msg); }

  private:

    pugi::xml_document xml_doc;
    std::shared_ptr<std::vector<base_property> > properties =
      std::make_shared<std::vector<base_property> >();
    std::shared_ptr<std::vector<base_instance> > instances =
      std::make_shared<std::vector<base_instance> >();
    std::shared_ptr<std::vector<base_relation> > relations =
      std::make_shared<std::vector<base_relation> >();

    std::optional<archive> artifact_archive;
    std::filesystem::path source_path;
    std::string last_error;
  };

  void document::clear()
  {
    xml_doc.reset();
    clear_annotations();
    artifact_archive.reset();
    source_path.clear();
    last_error.clear();
  }

  bool document::has_annotations() const
  {
    return (not properties->empty()) or
      (not instances->empty()) or
      (not relations->empty());
  }

  void document::clear_annotations()
  {
    properties->clear();
    instances->clear();
    relations->clear();
  }

  template<typename callback_type>
  void document::iterate_elements(callback_type&& callback) const
  {
    for(pugi::xml_node node:root().children())
      {
        if(node.type()==pugi::node_element)
          {
            callback(node);
          }
      }
  }

  template<typename callback_type>
  void document::iterate_elements(std::string_view name, callback_type&& callback) const
  {
    iterate_elements([&](pugi::xml_node node)
    {
      if(std::string_view(node.name())==name)
        {
          callback(node);
        }
    });
  }

  template<typename callback_type>
  void document::iterate_table_text(callback_type&& callback) const
  {
    iterate_elements("table", [&](pugi::xml_node table)
    {
      iterate_table_text(table, callback);
    });
  }

  template<typename callback_type>
  void document::iterate_table_text(pugi::xml_node table, callback_type&& callback) const
  {
    if(not table or std::string_view(table.name())!="table")
      {
        return;
      }

    for(pugi::xml_node child:table.children())
      {
        if((child.type()==pugi::node_pcdata or child.type()==pugi::node_cdata) and
           not is_xml_whitespace(child.value()))
          {
            callback(table, child);
          }
      }
  }

  bool is_picture_like_name(std::string_view name)
  {
    return name=="picture" or name=="figure";
  }

  bool is_picture_text_skip_name(std::string_view name)
  {
    return name=="location" or name=="src" or name=="ldiv" or name=="marker";
  }

  template<typename callback_type>
  void iterate_picture_text_descendants(pugi::xml_node picture,
                                        pugi::xml_node node,
                                        callback_type& callback)
  {
    for(pugi::xml_node child:node.children())
      {
        if(child.type()!=pugi::node_element)
          {
            continue;
          }

        const std::string_view name = child.name();
        if(is_picture_text_skip_name(name))
          {
            continue;
          }

        const std::string text = direct_text_content(child);
        if(not is_xml_whitespace(text.c_str()))
          {
            callback(picture, child);
          }

        iterate_picture_text_descendants(picture, child, callback);
      }
  }

  template<typename callback_type>
  void document::iterate_picture_text(callback_type&& callback) const
  {
    iterate_elements([&](pugi::xml_node node)
    {
      if(is_picture_like_name(node.name()))
        {
          iterate_picture_text(node, callback);
        }
    });
  }

  template<typename callback_type>
  void document::iterate_picture_text(pugi::xml_node picture, callback_type&& callback) const
  {
    if(not picture or not is_picture_like_name(picture.name()))
      {
        return;
      }

    iterate_picture_text_descendants(picture, picture, callback);
  }

}

#endif
