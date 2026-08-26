//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_DOCLANG_H_
#define PYBIND_ANDROMEDA_DOCLANG_H_

#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <andromeda.h>

namespace andromeda_py
{

  class DocLangXDocument
  {
  public:

    DocLangXDocument();
    ~DocLangXDocument();

    bool read(const std::string& path);
    bool read_xml(const std::string& xml);
    bool write(const std::string& path);
    bool apply_nlp(const std::string& models, std::size_t progress_every=25);

    bool valid() const;
    bool has_archive() const;
    bool has_annotations() const;

    std::string xml() const;
    std::string source_path() const;
    std::string last_error() const;

    std::vector<std::string> archive_paths() const;
    std::vector<std::string> annotation_paths() const;

    nlohmann::json summary() const;
    nlohmann::json properties() const;
    nlohmann::json instances() const;
    nlohmann::json relations() const;

    nlohmann::json query_properties(const std::string& type="",
                                    const std::string& label="",
                                    const std::string& subj_path="",
                                    float min_conf=0.0) const;

    nlohmann::json query_instances(const std::string& type="",
                                   const std::string& subtype="",
                                   const std::string& name="",
                                   const std::string& name_contains="",
                                   const std::string& subj_path="",
                                   float min_conf=0.0) const;

    nlohmann::json query_relations(const std::string& name="",
                                   const std::string& name_i="",
                                   const std::string& name_j="",
                                   const std::string& name_contains="",
                                   float min_conf=0.0) const;

  private:

    std::shared_ptr<andromeda::doclang::document> doc;
  };

  inline DocLangXDocument::DocLangXDocument():
    doc(std::make_shared<andromeda::doclang::document>())
  {}

  inline DocLangXDocument::~DocLangXDocument()
  {}

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
    std::vector<std::shared_ptr<andromeda::base_nlp_model> > nlp_models;
    if(not andromeda::to_models(models, nlp_models, true))
      {
        doc->set_last_error("could not initialise models: " + models);
        return false;
      }

    andromeda::doclang::nlp_apply_options options;
    options.document_name = doc->get_source_path().string();
    options.progress_every = progress_every;

    andromeda::doclang::nlp_apply_result result;
    return andromeda::doclang::apply_models(*doc, nlp_models, options, result);
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
      andromeda::doclang::RELATIONS_CSV
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
        {"relations", doc->get_relations().size()}
      });
  }

  inline nlohmann::json DocLangXDocument::properties() const
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

  inline nlohmann::json DocLangXDocument::instances() const
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

  inline nlohmann::json DocLangXDocument::relations() const
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

  inline nlohmann::json DocLangXDocument::query_properties(
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

    return result;
  }

  inline nlohmann::json DocLangXDocument::query_instances(
    const std::string& type,
    const std::string& subtype,
    const std::string& name,
    const std::string& name_contains,
    const std::string& subj_path,
    float min_conf) const
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
        if(inst.get_conf()<min_conf)
          {
            continue;
          }

        result["data"].push_back(inst.to_json_row());
      }

    return result;
  }

  inline nlohmann::json DocLangXDocument::query_relations(
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

    return result;
  }

}

#endif
