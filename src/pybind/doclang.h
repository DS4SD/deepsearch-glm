//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_DOCLANG_H_
#define PYBIND_ANDROMEDA_DOCLANG_H_

#include <cstdint>
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>

#include <andromeda.h>
#include <pybind/utils/pybind11_json.h>

namespace andromeda_py
{

  class DocLangXDocument
  {
  public:

    DocLangXDocument();
    ~DocLangXDocument();

    static std::uint64_t hash(const std::string& text);

    bool read(const std::string& path);
    bool read_xml(const std::string& xml);
    bool write(const std::string& path);
    bool apply_nlp(const std::string& models, std::size_t progress_every=25);
    void materialize_edges(const std::string& derived_entity_mode="terms");
    andromeda::doclang::document& mutable_document();
    std::string at(const std::string& xpath, const std::string& mode="auto");

    bool valid() const;
    bool has_archive() const;
    bool has_annotations() const;

    std::string xml() const;
    std::string source_path() const;
    std::string last_error() const;

    std::vector<std::string> archive_paths() const;
    std::vector<std::string> annotation_paths() const;

    nlohmann::json summary() const;
    pybind11::object properties() const;
    pybind11::object entities() const;
    pybind11::object instances() const;
    pybind11::object relations() const;
    pybind11::object edges() const;

    pybind11::object query_properties(const std::string& type="",
                                      const std::string& label="",
                                      const std::string& subj_path="",
                                      float min_conf=0.0) const;

    pybind11::object query_entities(const std::string& type="",
                                    const std::string& subtype="",
                                    const std::string& name="",
                                    const std::string& name_contains="",
                                    const std::string& entity_kind="",
                                    std::size_t min_count=0) const;

    pybind11::object query_instances(const std::string& type="",
                                     const std::string& subtype="",
                                     const std::string& name="",
                                     const std::string& name_contains="",
                                     const std::string& subj_path="",
                                     float min_conf=0.0,
                                     std::uint64_t entity_hash=0) const;

    pybind11::object query_relations(const std::string& name="",
                                     const std::string& name_i="",
                                     const std::string& name_j="",
                                     const std::string& name_contains="",
                                     float min_conf=0.0) const;

    pybind11::object query_edges(const std::string& name="",
                                 std::uint64_t hash_i=0,
                                 std::uint64_t hash_j=0,
                                 std::size_t min_count=0) const;

  private:

    static pybind11::object dataframe_from_table(const nlohmann::json& table);
    static std::string pandas_dtype_for_column(const std::string& column);

    nlohmann::json properties_table() const;
    nlohmann::json entities_table() const;
    nlohmann::json instances_table() const;
    nlohmann::json relations_table() const;
    nlohmann::json edges_table() const;

    std::shared_ptr<andromeda::doclang::document> doc;
  };

  class DocLangXNlp
  {
  public:

    DocLangXNlp();
    explicit DocLangXNlp(const std::string& models);
    ~DocLangXNlp();

    bool initialise(const std::string& models);
    bool apply(DocLangXDocument& doc, std::size_t progress_every=25);

    bool initialised() const;
    std::string model_expr() const;
    std::string last_error() const;
    std::vector<std::string> models() const;

  private:

    bool is_initialised;
    std::string model_expr_value;
    std::string last_error_value;
    std::vector<std::shared_ptr<andromeda::base_nlp_model> > nlp_models;
  };

  inline DocLangXDocument::DocLangXDocument():
    doc(std::make_shared<andromeda::doclang::document>())
  {}

  inline DocLangXDocument::~DocLangXDocument()
  {}

  inline std::uint64_t DocLangXDocument::hash(const std::string& text)
  {
    return andromeda::doclang::document::hash(text);
  }

  inline bool DocLangXDocument::read(const std::string& path)
  {
    return andromeda::doclang::reader::read(path, *doc);
  }

  inline bool DocLangXDocument::read_xml(const std::string& xml)
  {
    return andromeda::doclang::reader::read_dclg_buffer(xml, *doc);
  }

  inline bool DocLangXDocument::write(const std::string& path)
  {
    return andromeda::doclang::writer::write_dclx(path, *doc);
  }

  inline bool DocLangXDocument::apply_nlp(const std::string& models,
                                          std::size_t progress_every)
  {
    DocLangXNlp nlp;
    if(not nlp.initialise(models))
      {
        doc->set_last_error(nlp.last_error());
        return false;
      }

    return nlp.apply(*this, progress_every);
  }

  inline void DocLangXDocument::materialize_edges(const std::string& derived_entity_mode)
  {
    doc->materialize_edges(derived_entity_mode);
  }

  inline andromeda::doclang::document& DocLangXDocument::mutable_document()
  {
    return *doc;
  }

  inline std::string DocLangXDocument::at(const std::string& xpath,
                                          const std::string& mode)
  {
    return doc->at(xpath, mode);
  }

  inline DocLangXNlp::DocLangXNlp():
    is_initialised(false),
    model_expr_value(""),
    last_error_value(""),
    nlp_models({})
  {}

  inline DocLangXNlp::DocLangXNlp(const std::string& models):
    DocLangXNlp()
  {
    initialise(models);
  }

  inline DocLangXNlp::~DocLangXNlp()
  {}

  inline bool DocLangXNlp::initialise(const std::string& models)
  {
    nlp_models.clear();
    model_expr_value = "";
    last_error_value = "";
    is_initialised = false;

    if(not andromeda::to_models(models, nlp_models, true))
      {
        last_error_value = "could not initialise models: " + models;
        return false;
      }

    model_expr_value = models;
    is_initialised = true;
    return true;
  }

  inline bool DocLangXNlp::apply(DocLangXDocument& document,
                                 std::size_t progress_every)
  {
    if(not is_initialised)
      {
        last_error_value = "models have not been initialised";
        document.mutable_document().set_last_error(last_error_value);
        return false;
      }

    andromeda::doclang::nlp_apply_options options;
    options.document_name = document.mutable_document().get_source_path().string();
    options.progress_every = progress_every;

    andromeda::doclang::nlp_apply_result result;
    return andromeda::doclang::apply_models(document.mutable_document(),
                                            nlp_models,
                                            options,
                                            result);
  }

  inline bool DocLangXNlp::initialised() const
  {
    return is_initialised;
  }

  inline std::string DocLangXNlp::model_expr() const
  {
    return model_expr_value;
  }

  inline std::string DocLangXNlp::last_error() const
  {
    return last_error_value;
  }

  inline std::vector<std::string> DocLangXNlp::models() const
  {
    std::vector<std::string> names;
    for(const auto& model:nlp_models)
      {
        names.push_back(model->get_key());
      }

    return names;
  }

  inline bool DocLangXDocument::valid() const
  {
    return static_cast<bool>(doc->root());
  }

  inline bool DocLangXDocument::has_archive() const
  {
    return doc->has_archive();
  }

  inline bool DocLangXDocument::has_annotations() const
  {
    return doc->has_annotations();
  }

  inline std::string DocLangXDocument::xml() const
  {
    return andromeda::doclang::serialize_xml(doc->xml());
  }

  inline std::string DocLangXDocument::source_path() const
  {
    return doc->get_source_path().string();
  }

  inline std::string DocLangXDocument::last_error() const
  {
    return doc->get_last_error();
  }

  inline std::vector<std::string> DocLangXDocument::archive_paths() const
  {
    if(not doc->has_archive())
      {
        return {};
      }

    return doc->artifacts().paths();
  }

  inline std::vector<std::string> DocLangXDocument::annotation_paths() const
  {
    return {
      andromeda::doclang::PROPERTIES_CSV,
      andromeda::doclang::INSTANCES_CSV,
      andromeda::doclang::ENTITIES_CSV,
      andromeda::doclang::RELATIONS_CSV,
      andromeda::doclang::EDGES_CSV
    };
  }

  inline nlohmann::json DocLangXDocument::summary() const
  {
    return nlohmann::json::object({
        {"valid", valid()},
        {"source_path", source_path()},
        {"has_archive", has_archive()},
        {"has_annotations", has_annotations()},
        {"properties", doc->get_properties().size()},
        {"instances", doc->get_instances().size()},
        {"entities", doc->get_entities().size()},
        {"relations", doc->get_relations().size()},
        {"edges", doc->get_edges().size()}
      });
  }

  inline pybind11::object DocLangXDocument::dataframe_from_table(
    const nlohmann::json& table)
  {
    pybind11::module_ pandas = pybind11::module_::import("pandas");
    pybind11::object dataframe = pandas.attr("DataFrame")(
      pyjson::from_json(table.at("data")),
      pybind11::arg("columns") = pyjson::from_json(table.at("headers")));

    pybind11::dict dtypes;
    for(const auto& header:table.at("headers"))
      {
        const std::string column = header.get<std::string>();
        const std::string dtype = pandas_dtype_for_column(column);
        if(not dtype.empty())
          {
            dtypes[pybind11::str(column)] = pybind11::str(dtype);
          }
      }

    return dataframe.attr("astype")(dtypes);
  }

  inline std::string DocLangXDocument::pandas_dtype_for_column(
    const std::string& column)
  {
    if(column=="type" or
       column=="subtype" or
       column=="subj_name" or
       column=="subj_path" or
       column=="label" or
       column=="entity_kind" or
       column=="name" or
       column=="original" or
       column=="parent" or
       column=="name_i" or
       column=="name_j")
      {
        return "string";
      }

    if(column=="confidence" or column=="conf" or column=="probability")
      {
        return "Float32";
      }

    if(column=="wtok-match")
      {
        return "boolean";
      }

    if(column=="subj_hash" or
       column=="hash" or
       column=="ihash" or
       column=="hash_i" or
       column=="hash_j" or
       column=="parent_hash" or
       column=="count" or
       column=="coor_i" or
       column=="coor_j" or
       column=="char_i" or
       column=="char_j" or
       column=="ctok_i" or
       column=="ctok_j" or
       column=="wtok_i" or
       column=="wtok_j" or
       column=="flvr" or
       column=="total-count")
      {
        return "UInt64";
      }

    return "";
  }

  inline nlohmann::json DocLangXDocument::properties_table() const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_property::HEADERS;
    result["data"] = nlohmann::json::array();

    for(auto prop:doc->get_properties())
      {
        result["data"].push_back(prop.to_json_row());
      }

    return result;
  }

  inline nlohmann::json DocLangXDocument::entities_table() const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_entity::HEADERS;
    result["data"] = nlohmann::json::array();

    for(const auto& ent:doc->get_entities())
      {
        result["data"].push_back(ent.to_json_row());
      }

    return result;
  }

  inline nlohmann::json DocLangXDocument::instances_table() const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_instance::HEADERS;
    result["data"] = nlohmann::json::array();

    for(const auto& inst:doc->get_instances())
      {
        result["data"].push_back(inst.to_json_row());
      }

    return result;
  }

  inline nlohmann::json DocLangXDocument::relations_table() const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_relation::headers();
    result["data"] = nlohmann::json::array();

    for(auto rel:doc->get_relations())
      {
        result["data"].push_back(rel.to_json_row());
      }

    return result;
  }

  inline nlohmann::json DocLangXDocument::edges_table() const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_graph_edge::HEADERS;
    result["data"] = nlohmann::json::array();

    for(auto edge:doc->get_edges())
      {
        result["data"].push_back(edge.to_json_row());
      }

    return result;
  }

  inline pybind11::object DocLangXDocument::properties() const
  {
    return dataframe_from_table(properties_table());
  }

  inline pybind11::object DocLangXDocument::entities() const
  {
    return dataframe_from_table(entities_table());
  }

  inline pybind11::object DocLangXDocument::instances() const
  {
    return dataframe_from_table(instances_table());
  }

  inline pybind11::object DocLangXDocument::relations() const
  {
    return dataframe_from_table(relations_table());
  }

  inline pybind11::object DocLangXDocument::edges() const
  {
    return dataframe_from_table(edges_table());
  }

  inline pybind11::object DocLangXDocument::query_properties(
    const std::string& type,
    const std::string& label,
    const std::string& subj_path,
    float min_conf) const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_property::HEADERS;
    result["data"] = nlohmann::json::array();

    for(auto prop:doc->get_properties())
      {
        if((not type.empty()) and prop.get_type()!=type)
          {
            continue;
          }
        if((not label.empty()) and prop.get_label()!=label)
          {
            continue;
          }
        if((not subj_path.empty()) and prop.get_subj_path()!=subj_path)
          {
            continue;
          }
        if(prop.get_conf()<min_conf)
          {
            continue;
          }

        result["data"].push_back(prop.to_json_row());
      }

    return dataframe_from_table(result);
  }

  inline pybind11::object DocLangXDocument::query_entities(
    const std::string& type,
    const std::string& subtype,
    const std::string& name,
    const std::string& name_contains,
    const std::string& entity_kind,
    std::size_t min_count) const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_entity::HEADERS;
    result["data"] = nlohmann::json::array();

    for(const auto& ent:doc->get_entities())
      {
        if((not type.empty()) and ent.get_type()!=type)
          {
            continue;
          }
        if((not subtype.empty()) and ent.get_subtype()!=subtype)
          {
            continue;
          }
        if((not name.empty()) and ent.get_name()!=name)
          {
            continue;
          }
        if((not name_contains.empty()) and
           ent.get_name().find(name_contains)==std::string::npos)
          {
            continue;
          }
        if((not entity_kind.empty()) and ent.get_entity_kind()!=entity_kind)
          {
            continue;
          }
        if(ent.get_count()<min_count)
          {
            continue;
          }

        result["data"].push_back(ent.to_json_row());
      }

    return dataframe_from_table(result);
  }

  inline pybind11::object DocLangXDocument::query_instances(
    const std::string& type,
    const std::string& subtype,
    const std::string& name,
    const std::string& name_contains,
    const std::string& subj_path,
    float min_conf,
    std::uint64_t entity_hash) const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_instance::HEADERS;
    result["data"] = nlohmann::json::array();

    for(const auto& inst:doc->get_instances())
      {
        if((not type.empty()) and inst.get_type()!=type)
          {
            continue;
          }
        if((not subtype.empty()) and inst.get_subtype()!=subtype)
          {
            continue;
          }
        if((not name.empty()) and inst.get_name()!=name)
          {
            continue;
          }
        if((not name_contains.empty()) and
           inst.get_name().find(name_contains)==std::string::npos)
          {
            continue;
          }
        if((not subj_path.empty()) and inst.get_subj_path()!=subj_path)
          {
            continue;
          }
        if(entity_hash!=0 and inst.get_ehash()!=entity_hash)
          {
            continue;
          }
        if(inst.get_conf()<min_conf)
          {
            continue;
          }

        result["data"].push_back(inst.to_json_row());
      }

    return dataframe_from_table(result);
  }

  inline pybind11::object DocLangXDocument::query_relations(
    const std::string& name,
    const std::string& name_i,
    const std::string& name_j,
    const std::string& name_contains,
    float min_conf) const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_relation::headers();
    result["data"] = nlohmann::json::array();

    for(auto rel:doc->get_relations())
      {
        if((not name.empty()) and rel.get_name()!=name)
          {
            continue;
          }
        if((not name_i.empty()) and rel.get_name_i()!=name_i)
          {
            continue;
          }
        if((not name_j.empty()) and rel.get_name_j()!=name_j)
          {
            continue;
          }
        if((not name_contains.empty()) and
           rel.get_name_i().find(name_contains)==std::string::npos and
           rel.get_name_j().find(name_contains)==std::string::npos)
          {
            continue;
          }
        if(rel.get_conf()<min_conf)
          {
            continue;
          }

        result["data"].push_back(rel.to_json_row());
      }

    return dataframe_from_table(result);
  }

  inline pybind11::object DocLangXDocument::query_edges(
    const std::string& name,
    std::uint64_t hash_i,
    std::uint64_t hash_j,
    std::size_t min_count) const
  {
    nlohmann::json result = nlohmann::json::object();
    result["headers"] = andromeda::base_graph_edge::HEADERS;
    result["data"] = nlohmann::json::array();

    for(auto edge:doc->get_edges())
      {
        if((not name.empty()) and edge.get_name()!=name)
          {
            continue;
          }
        if(hash_i!=0 and edge.get_hash_i()!=hash_i)
          {
            continue;
          }
        if(hash_j!=0 and edge.get_hash_j()!=hash_j)
          {
            continue;
          }
        if(edge.get_count()<min_count)
          {
            continue;
          }

        result["data"].push_back(edge.to_json_row());
      }

    return dataframe_from_table(result);
  }

}

#endif
