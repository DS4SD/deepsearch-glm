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
    const static inline std::string insts_lbl = "instances";
    const static inline std::string ents_lbl = "entities";
    const static inline std::string rels_lbl = "relations";
    const static inline std::string recs_lbl = "records";

    const static inline std::string prov_lbl = "prov";
    const static inline std::string hash_lbl = "hash";
    
    const static inline std::string dloc_lbl = "dloc";
    const static inline std::string dref_lbl = "dref";
    const static inline std::string jref_lbl = "$ref";

    const static inline std::string applied_models_lbl = "applied-models";

    const static inline std::string text_lbl = "text"; // for text
    const static inline std::string table_data_lbl = "data"; // for tables and figures
    const static inline std::string figure_data_lbl = "data"; // for tables and figures
    
  public:

    base_subject();
    base_subject(subject_name name);

    base_subject(uint64_t dhash, std::string dloc, subject_name name);//, prov_element& prov);

    virtual ~base_subject() {}

    static nlohmann::json get_prov_refs(std::vector<std::shared_ptr<prov_element> >& provs);
    
    subject_name get_name() const { return name; }
    hash_type get_hash() const { return hash; }

    void clear();

    void clear_models();

    virtual nlohmann::json to_json() = 0;
    virtual nlohmann::json to_json(const std::set<std::string>& filters) = 0;
    virtual bool from_json(const nlohmann::json& item) = 0;
    
  protected:
    
    nlohmann::json _to_json();
    nlohmann::json _to_json(const std::set<std::string>& filters);

    bool _from_json(const nlohmann::json& item);
    
  public:

    bool valid;
    subject_name name;

    hash_type hash; // hash of the item
    hash_type dhash; // hash of the document of the item

    std::string dloc; // location of item in the document <doc-hash>#<JSON-path-in-doc>
    
    std::set<std::string> applied_models;

    std::vector<base_property> properties;
    std::vector<base_instance> instances;
    std::vector<base_relation> relations;

    //std::vector<base_entity> entities;
  };

  base_subject::base_subject():
    valid(false),
    name(UNDEF),

    hash(-1),
    dhash(-1),

    dloc(""),

    applied_models({}),

    properties({}),
    instances({}),
    relations({})
  {}

  base_subject::base_subject(subject_name name):
    valid(true),
    name(name),

    hash(-1),
    dhash(-1),

    dloc(""),
    
    applied_models({}),

    properties({}),
    instances({}),
    relations({})
  {}

  base_subject::base_subject(uint64_t dhash,
			     std::string dloc,
			     subject_name name)://, prov_element& prov):
    valid(true),
    name(name),

    hash(-1),
    dhash(dhash),

    dloc(dloc),
    
    applied_models({}),

    properties({}),
    instances({}),
    relations({})
  {}

  nlohmann::json base_subject::get_prov_refs(std::vector<std::shared_ptr<prov_element> >& provs)
  {
    nlohmann::json result = nlohmann::json::array({});
    for(auto& prov:provs)
      {
	if(prov!=NULL)
	  {
	    nlohmann::json pref;
	    pref[base_subject::jref_lbl] = prov->get_pref();
	    
	    result.push_back(pref);
	  }
	else
	  {
	    LOG_S(WARNING) << "base_subject encountered prov with NULL";
	  }
      }

    return result;
  }
  
  void base_subject::clear()
  {
    valid = false;

    hash = -1;
    dhash = -1;

    clear_models();
  }

  void base_subject::clear_models()
  {
    applied_models.clear();

    properties.clear();
    instances.clear();
    relations.clear();
  }

  nlohmann::json base_subject::_to_json()
  {
    nlohmann::json result = nlohmann::json::object({});

    {
      result[base_subject::hash_lbl] = hash;
      result[base_subject::dloc_lbl] = dloc;

      result["applied-models"] = applied_models;
    }

    {
      nlohmann::json& props = result[prps_lbl];
      andromeda::to_json(properties, props);
    }

    {
      nlohmann::json& insts = result[insts_lbl];
      andromeda::to_json(instances, insts);
    }

    {
      nlohmann::json& rels = result[rels_lbl];
      andromeda::to_json(relations, rels);
    }

    return result;
  }

  nlohmann::json base_subject::_to_json(const std::set<std::string>& filters)
  {
    nlohmann::json result = nlohmann::json::object({});

    if(filters.count(base_subject::hash_lbl))
      {	  
	result[base_subject::hash_lbl] = hash;
      }
    
    if(filters.count(base_subject::dloc_lbl))
      {
	result[base_subject::dloc_lbl] = dloc;
      }
    
    if(filters.count(base_subject::applied_models_lbl))
      {
	result["applied-models"] = applied_models;
      }

    if(filters.count(base_subject::prps_lbl))
      {
	nlohmann::json& props = result[prps_lbl];
	andromeda::to_json(properties, props);
      }

    if(filters.count(base_subject::insts_lbl))    
      {
	nlohmann::json& insts = result[insts_lbl];
	andromeda::to_json(instances, insts);
      }

    if(filters.count(base_subject::rels_lbl))
      {
	nlohmann::json& rels = result[rels_lbl];
	andromeda::to_json(relations, rels);
      }

    return result;    
  }
  
  bool base_subject::_from_json(const nlohmann::json& item)
  {
    hash = item.value("hash", hash);
    applied_models = item.value("applied-models", applied_models);

    bool read_props=false, read_insts=false, read_rels=false;

    properties.clear();
    if(item.count(prps_lbl))
      {
        const nlohmann::json& props = item[prps_lbl];
        read_props = andromeda::from_json(properties, props);
      }

    instances.clear();
    if(item.count(insts_lbl))
      {
        const nlohmann::json& insts = item[insts_lbl];
        read_insts = andromeda::from_json(instances, insts);
      }

    relations.clear();
    if(item.count(rels_lbl))
      {
        const nlohmann::json& rels = item[rels_lbl];
        read_rels = andromeda::from_json(relations, rels);
      }

    return (read_props and read_insts and read_rels);
  }

}

#endif
