//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_BASE_SUBJECT_H
#define ANDROMEDA_SUBJECTS_BASE_SUBJECT_H

namespace andromeda
{

  class base_subject: public base_types
  {
  public:

    const static inline std::string head_lbl = "headers";
    const static inline std::string data_lbl = "data";

    const static inline std::string captions_lbl = "captions";
    const static inline std::string footnotes_lbl = "footnotes";
    const static inline std::string mentions_lbl = "mentions";

    const static inline std::string prps_lbl = "properties";
    const static inline std::string ents_lbl = "entities";
    const static inline std::string rels_lbl = "relations";
    const static inline std::string recs_lbl = "records";

  public:

    base_subject();
    base_subject(subject_name name);

    base_subject(uint64_t dhash, subject_name name, prov_element& prov);

    subject_name get_name() const { return name; };
    hash_type get_hash() const { return hash; }
    std::string get_path() const { return (provs.size()>0? provs.at(0).to_path():"#"); }

    void clear();

    void clear_models();

    nlohmann::json to_json();
    bool from_json(const nlohmann::json& item);
    
  public:

    bool valid;
    subject_name name;

    hash_type hash;
    hash_type dhash;

    std::vector<prov_element> provs;

    std::set<std::string> applied_models;

    std::vector<base_property> properties;
    std::vector<base_entity> entities;
    std::vector<base_relation> relations;
  };

  base_subject::base_subject():
    valid(false),
    name(UNDEF),

    hash(-1),
    dhash(-1),

    provs({}),

    applied_models({}),

    properties({}),
    entities({}),
    relations({})
  {}

  base_subject::base_subject(subject_name name):
    valid(true),
    name(name),

    hash(-1),
    dhash(-1),

    provs({}),

    applied_models({}),

    properties({}),
    entities({}),
    relations({})
  {}

  base_subject::base_subject(uint64_t dhash, subject_name name, prov_element& prov):
    valid(true),
    name(name),

    hash(-1),
    dhash(dhash),

    provs({prov}),

    applied_models({}),

    properties({}),
    entities({}),
    relations({})
  {}

  void base_subject::clear()
  {
    valid = false;

    hash = -1;
    dhash = -1;

    provs.clear();

    clear_models();
  }

  void base_subject::clear_models()
  {
    applied_models.clear();

    properties.clear();
    entities.clear();
    relations.clear();
  }

  nlohmann::json base_subject::to_json()
  {
    nlohmann::json result = nlohmann::json::object({});

    {
      result["hash"] = hash;
      result["applied-models"] = applied_models;
    }

    {
      nlohmann::json& props = result[prps_lbl];
      andromeda::to_json(properties, props);
    }

    {
      nlohmann::json& ents = result[ents_lbl];
      andromeda::to_json(entities, ents);
    }

    {
      nlohmann::json& rels = result[rels_lbl];
      andromeda::to_json(relations, rels);
    }


    return result;
  }

  bool base_subject::from_json(const nlohmann::json& item)
  {
    hash = item.value("hash", hash);
    applied_models = item.value("applied-models", applied_models);

    bool read_props=false, read_ents=false, read_rels=false;

    properties.clear();
    if(item.count(prps_lbl))
      {
        const nlohmann::json& props = item[prps_lbl];
        read_props = andromeda::from_json(properties, props);
      }

    entities.clear();
    if(item.count(ents_lbl))
      {
        const nlohmann::json& ents = item[ents_lbl];
        read_ents = andromeda::from_json(entities, ents);
      }

    relations.clear();
    if(item.count(rels_lbl))
      {
        const nlohmann::json& rels = item[rels_lbl];
        read_rels = andromeda::from_json(relations, rels);
      }

    return (read_props and read_ents and read_rels);

  }

}

#endif
