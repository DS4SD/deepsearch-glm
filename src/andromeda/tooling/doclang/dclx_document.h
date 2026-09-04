//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_DCLX_DOCUMENT_H_
#define ANDROMEDA_TOOLING_DOCLANG_DCLX_DOCUMENT_H_

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
#include <andromeda/tooling/doclang/dclg_document.h>

namespace andromeda::doclang
{

  class dclx_document: public dclg_document
  {
  public:

    // summary, ToC and concepts are DCLG sidecars: parsed, never raw strings
    typedef std::optional<std::shared_ptr<dclg_document> > sidecar_type;

    dclx_document() = default;

    void clear();

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

    bool has_document_reference() const { return document_reference.has_value(); }
    bool has_references() const { return references.has_value(); }
    bool has_summary() const { return summary.has_value(); }
    bool has_toc() const { return toc.has_value(); }
    bool has_concepts() const { return concepts.has_value(); }

    const std::optional<std::string>& get_document_reference() const { return document_reference; }
    const std::optional<std::string>& get_references() const { return references; }
    const sidecar_type& get_summary() const { return summary; }
    const sidecar_type& get_toc() const { return toc; }
    const sidecar_type& get_concepts() const { return concepts; }

    void set_document_reference(std::string value) { document_reference = std::move(value); }
    void set_references(std::string value) { references = std::move(value); }
    void set_summary(std::shared_ptr<dclg_document> value) { summary = std::move(value); }
    void set_toc(std::shared_ptr<dclg_document> value) { toc = std::move(value); }
    void set_concepts(std::shared_ptr<dclg_document> value) { concepts = std::move(value); }

    void clear_document_reference() { document_reference.reset(); }
    void clear_references() { references.reset(); }
    void clear_summary() { summary.reset(); }
    void clear_toc() { toc.reset(); }
    void clear_concepts() { concepts.reset(); }

  private:

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

    std::optional<std::string> document_reference;
    std::optional<std::string> references;
    sidecar_type summary;
    sidecar_type toc;
    sidecar_type concepts;

    std::optional<archive> artifact_archive;
    std::filesystem::path source_path;
  };

  void dclx_document::clear()
  {
    dclg_document::clear();

    clear_annotations();
    clear_document_reference();
    clear_references();
    clear_summary();
    clear_toc();
    clear_concepts();
    artifact_archive.reset();
    source_path.clear();
  }

  bool dclx_document::has_annotations() const
  {
    return (not properties->empty()) or
      (not instances->empty()) or
      (not entities->empty()) or
      (not relations->empty()) or
      (not edges->empty()) or
      has_document_reference() or
      has_references() or
      has_summary() or
      has_toc() or
      has_concepts();
  }

  void dclx_document::clear_annotations()
  {
    properties->clear();
    instances->clear();
    entities->clear();
    relations->clear();
    edges->clear();
  }

  void dclx_document::compute_entities()
  {
    *entities = compute_entities_from_instances(*instances);
  }

  void dclx_document::materialize_edges(std::string_view derived_entity_mode)
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

}

#endif
