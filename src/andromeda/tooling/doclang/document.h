//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_DOCUMENT_H_
#define ANDROMEDA_TOOLING_DOCLANG_DOCUMENT_H_

#include <filesystem>
#include <memory>
#include <set>
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

    static base_types::hash_type hash(std::string_view text);

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
    std::shared_ptr<std::vector<base_entity> > shared_entities() { return entities; }
    std::shared_ptr<std::vector<base_relation> > shared_relations() { return relations; }
    std::shared_ptr<std::vector<base_graph_edge> > shared_edges() { return edges; }

    std::vector<base_property>& mutable_properties() { return *properties; }
    std::vector<base_instance>& mutable_instances() { return *instances; }
    std::vector<base_entity>& mutable_entities() { return *entities; }
    std::vector<base_relation>& mutable_relations() { return *relations; }
    std::vector<base_graph_edge>& mutable_edges() { return *edges; }

    const std::vector<base_property>& get_properties() const { return *properties; }
    const std::vector<base_instance>& get_instances() const { return *instances; }
    const std::vector<base_entity>& get_entities() const { return *entities; }
    const std::vector<base_relation>& get_relations() const { return *relations; }
    const std::vector<base_graph_edge>& get_edges() const { return *edges; }

    bool has_annotations() const;
    void clear_annotations();
    void compute_entities();
    void materialize_edges(std::string_view derived_entity_mode="terms");

    std::string at(std::string_view xpath, std::string_view mode="auto");

    const std::string& get_last_error() const { return last_error; }
    void set_last_error(std::string msg) { last_error = std::move(msg); }

  private:

    pugi::xml_document xml_doc;
    std::shared_ptr<std::vector<base_property> > properties =
      std::make_shared<std::vector<base_property> >();
    std::shared_ptr<std::vector<base_instance> > instances =
      std::make_shared<std::vector<base_instance> >();
    std::shared_ptr<std::vector<base_entity> > entities =
      std::make_shared<std::vector<base_entity> >();
    std::shared_ptr<std::vector<base_relation> > relations =
      std::make_shared<std::vector<base_relation> >();
    std::shared_ptr<std::vector<base_graph_edge> > edges =
      std::make_shared<std::vector<base_graph_edge> >();

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
      (not entities->empty()) or
      (not relations->empty()) or
      (not edges->empty());
  }

  void document::clear_annotations()
  {
    properties->clear();
    instances->clear();
    entities->clear();
    relations->clear();
    edges->clear();
  }

  void document::compute_entities()
  {
    *entities = compute_entities_from_instances(*instances);
  }

  void document::materialize_edges(std::string_view derived_entity_mode)
  {
    edges->clear();
    if(entities->empty() and not instances->empty())
      {
        compute_entities();
      }

    std::set<base_types::hash_type> included_entities;
    for(const auto& entity:*entities)
      {
        const bool exact = entity.get_entity_kind()=="exact";
        const bool derived_term =
          entity.get_type()=="term" and entity.get_entity_kind()=="derived";
        const bool include =
          derived_entity_mode=="all" or
          (derived_entity_mode=="terms" and (exact or derived_term)) or
          (derived_entity_mode=="none" and exact);

        if(include)
          {
            included_entities.insert(entity.get_hash());
          }
      }

    for(const auto& entity:*entities)
      {
        if(included_entities.count(entity.get_hash())==0)
          {
            continue;
          }

        const auto parent_hash = entity.get_parent_hash();
        if(parent_hash!=0 and included_entities.count(parent_hash)>0)
          {
            edges->emplace_back("tax-up", entity.get_hash(), parent_hash);
            edges->emplace_back("tax-dn", parent_hash, entity.get_hash());
          }
      }

    for(const auto& inst:*instances)
      {
        if(included_entities.count(inst.get_ehash())==0)
          {
            continue;
          }

        edges->emplace_back("to-instances", inst.get_ehash(), inst.get_ihash());
        edges->emplace_back("to-entities", inst.get_ihash(), inst.get_ehash());
      }

    for(const auto& rel:*relations)
      {
        edges->emplace_back(rel.get_name(), rel.get_hash_i(), rel.get_hash_j());
      }
  }

  base_types::hash_type document::hash(std::string_view text)
  {
    return utils::to_reproducible_hash(std::string(text));
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

  std::string join_doclang_text(std::vector<std::string> values)
  {
    std::ostringstream oss;

    bool first = true;
    for(const auto& value:values)
      {
        if(value.empty())
          {
            continue;
          }

        if(not first)
          {
            oss << "\n";
          }

        oss << value;
        first = false;
      }

    return oss.str();
  }

  std::string table_text_content(pugi::xml_node table)
  {
    std::vector<std::string> values;

    for(pugi::xml_node child:table.children())
      {
        if((child.type()==pugi::node_pcdata or child.type()==pugi::node_cdata) and
           not is_xml_whitespace(child.value()))
          {
            values.push_back(strip_content(child.value()));
          }
      }

    return join_doclang_text(values);
  }

  std::string picture_text_content(const document& doc, pugi::xml_node picture)
  {
    std::vector<std::string> values;

    doc.iterate_picture_text(picture, [&](pugi::xml_node, pugi::xml_node text_node)
    {
      values.push_back(strip_content(direct_text_content(text_node)));
    });

    return join_doclang_text(values);
  }

  std::string node_text_content(const document& doc, pugi::xml_node node)
  {
    if(node.type()==pugi::node_pcdata or node.type()==pugi::node_cdata)
      {
        return strip_content(node.value());
      }

    if(std::string_view(node.name())=="table")
      {
        return table_text_content(node);
      }

    if(is_picture_like_name(node.name()))
      {
        return picture_text_content(doc, node);
      }

    return strip_content(direct_text_content(node));
  }

  std::string document::at(std::string_view xpath, std::string_view mode)
  {
    const auto lookup = resolve_doclang_path(root(), xpath);
    if(not lookup.found)
      {
        set_last_error(lookup.error);
        return "";
      }

    if(mode=="doclang")
      {
        last_error.clear();
        return serialize_node(lookup.node);
      }

    if(mode=="text")
      {
        last_error.clear();
        return node_text_content(*this, lookup.node);
      }

    if(mode=="auto")
      {
        const std::string text = node_text_content(*this, lookup.node);
        if(not text.empty() or
           lookup.node.type()==pugi::node_pcdata or
           lookup.node.type()==pugi::node_cdata or
           std::string_view(lookup.node.name())=="table" or
           is_picture_like_name(lookup.node.name()))
          {
            last_error.clear();
            return text;
          }

        last_error.clear();
        return serialize_node(lookup.node);
      }

    set_last_error("unsupported DocLang access mode: " + std::string(mode));
    return "";
  }

}

#endif
