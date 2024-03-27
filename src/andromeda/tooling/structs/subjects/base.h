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

    const static inline std::string subj_hash_lbl = "subj_hash";
    const static inline std::string text_hash_lbl = "text_hash"; // for text
    
    const static inline std::string dloc_lbl = "dloc"; // location in the document
    const static inline std::string sref_lbl = "sref"; // self-reference via path
    const static inline std::string jref_lbl = "$ref"; // json-ref convention

    const static inline std::string name_lbl = "name";
    const static inline std::string type_lbl = "type";

    const static inline std::string applied_models_lbl = "applied_models";

    const static inline std::string text_lbl = "text"; // for text
    const static inline std::string orig_lbl = "orig"; // for text
    
    const static inline std::string table_data_lbl = "data"; // for tables and figures
    const static inline std::string figure_data_lbl = "data"; // for tables and figures

    const static inline std::string confidence_lbl = "confidence"; // for tables and figures
    const static inline std::string created_by_lbl = "created_by"; // for tables and figures

    const static inline std::set<std::string> implicit_models = {"lapos"};
    
  public:

    base_subject();
    base_subject(subject_name name);

    base_subject(uint64_t dhash, std::string dloc, subject_name name);//, prov_element& prov);

    virtual ~base_subject() {}

    std::string get_dloc();
    void set_dloc(std::string dloc);
    
    std::string get_self_ref();
    void set_self_ref(std::string sref);
    
    bool is_valid() const { return valid; }
    void set_valid(bool val) { this->valid=val; }
    
    static bool set_prov_refs(const nlohmann::json& data,
			      const std::vector<std::shared_ptr<prov_element> >& doc_provs,
			      std::vector<std::shared_ptr<prov_element> >& base_provs);

    static nlohmann::json get_prov_refs(const std::vector<std::shared_ptr<prov_element> >& provs);

    subject_name get_name() const { return name; }
    hash_type get_hash() const { return hash; }
    
    std::vector<base_property>& get_properties() { return properties; }
    std::vector<base_instance>& get_instances() { return instances; }
    std::vector<base_relation>& get_relations() { return relations; }

    std::vector<std::string> get_property_labels(model_name name); 
    
    void clear();

    void clear_models();

    void sort();
    
    virtual nlohmann::json to_json(const std::set<std::string>& filters={}) = 0;

    virtual bool from_json(const nlohmann::json& item) = 0;
    virtual bool from_json(const nlohmann::json& item,
			   const std::vector<std::shared_ptr<prov_element> >& doc_provs) = 0;
    
  protected:

    nlohmann::json _to_json(const std::set<std::string>& filters);
    
    nlohmann::json _to_json(const std::set<std::string>& filters,
			    const std::vector<std::shared_ptr<prov_element> >& provs);

    bool _from_json(const nlohmann::json& item);
    
    template<typename item_type>
    static void to_json(nlohmann::json& result,
                        std::string key, std::vector<item_type>& val);

    template<typename item_type, typename filters_type>
    static void to_json(nlohmann::json& result,
                        std::string key, std::vector<item_type>& val,
                        filters_type filters);

    template<typename item_type>
    static bool from_json(const nlohmann::json& item,
			  std::string key,
			  std::vector<std::shared_ptr<item_type> >& val);

    template<typename item_type>
    static bool from_json(const nlohmann::json& item,
			  const std::vector<std::shared_ptr<prov_element> >& doc_provs,
			  std::string key,
			  std::vector<std::shared_ptr<item_type> >& vals);
    
    //public:
  protected:
    
    bool valid;
    subject_name name;

    hash_type hash; // hash of the item
    hash_type dhash; // hash of the document of the item
    
    std::string dloc; // location of item in the document <doc-hash>#<JSON-path-in-doc>
    std::string sref;
    
  public:
    
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

    dloc("#"),
    sref("#"),
    
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

    dloc("#"),
    sref("#"),
    
    applied_models({}),

    properties({}),
    instances({}),
    relations({})
  {}

  base_subject::base_subject(uint64_t dhash,
                             std::string dloc,
                             subject_name name):
    valid(true),
    name(name),

    hash(-1),
    dhash(dhash),

    dloc(dloc),
    sref("#"),
    
    applied_models({}),

    properties({}),
    instances({}),
    relations({})
  {
    auto parts = utils::split(dloc, "#");
    if(parts.size()==2)
      {
	sref += parts.at(1);
      }
    else
      {
	LOG_S(WARNING) << "could not derive sref from dloc: " << dloc;
      }
  }

  void base_subject::set_dloc(std::string dloc)
  {
    this->dloc = dloc;
  }
  
  std::string base_subject::get_dloc()
  {
    return dloc;
  }
  
  void base_subject::set_self_ref(std::string sref)
  {
    this->sref = sref;
  }
  
  std::string base_subject::get_self_ref()
  {
    return sref;
  }

  std::vector<std::string> base_subject::get_property_labels(model_name name)
  {
    std::vector<std::string> labels={};
    for(auto& prop:properties)
      {
	if(prop.is_model(name))
	  {
	    labels.push_back(prop.get_label());
	  }
      }
    
    return labels;
  }
  
  bool base_subject::set_prov_refs(const nlohmann::json& data,
				   const std::vector<std::shared_ptr<prov_element> >& doc_provs,
				   std::vector<std::shared_ptr<prov_element> >& base_provs)
  {
    if(data.count(prov_lbl)==0)
      {
	return false;
      }

    base_provs.clear();
    for(auto& item:data.at(prov_lbl))
      {
	std::string path = item.at(jref_lbl).get<std::string>();

	auto parts = utils::split(path, "/");
	auto index = std::stoi(parts.back());
	
	base_provs.push_back(doc_provs.at(index));
      }

    return true;
  }
  
  nlohmann::json base_subject::get_prov_refs(const std::vector<std::shared_ptr<prov_element> >& provs)
  {
    nlohmann::json result = nlohmann::json::array({});
    for(auto& prov:provs)
      {
        if(prov!=NULL)
          {
            nlohmann::json pref;
	    pref[base_subject::jref_lbl] = prov->get_self_ref();

            result.push_back(pref);
          }
        else
          {
            LOG_S(WARNING) << "base_subject encountered prov with NULL";
          }
      }

    return result;
  }

  void base_subject::sort()
  {
    //LOG_S(INFO) << __FILE__ << ":" << __LINE__ << " -> sort properties";
    std::sort(properties.begin(), properties.end());

    //LOG_S(INFO) << __FILE__ << ":" << __LINE__ << " -> sort instances";
    std::sort(instances.begin(), instances.end());

    //LOG_S(INFO) << __FILE__ << ":" << __LINE__ << " -> sort relations";
    std::sort(relations.begin(), relations.end());
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

  nlohmann::json base_subject::_to_json(const std::set<std::string>& filters)
  {
    nlohmann::json result = nlohmann::json::object({});

    {
      result[subj_hash_lbl] = hash;
      result[dloc_lbl] = dloc;
      result[sref_lbl] = sref;
    }
    
    if((properties.size()>0) and (filters.size()==0 or filters.count(prps_lbl)))
      {
        nlohmann::json& props = result[prps_lbl];
        andromeda::to_json(properties, props);
      }

    if((instances.size()>0) and (filters.size()==0 or filters.count(insts_lbl)))
      {
        nlohmann::json& insts = result[insts_lbl];
        andromeda::to_json(instances, insts);
      }

    if((relations.size()>0) and (filters.size()==0 or filters.count(rels_lbl)))
      {
        nlohmann::json& rels = result[rels_lbl];
        andromeda::to_json(relations, rels);
      }

    if(filters.size()==0 or filters.count(applied_models_lbl))
      {
	for(auto implicit_model:implicit_models)
	  {
	    applied_models.erase(implicit_model);
	  }
	
	result[applied_models_lbl] = applied_models;
      }
    
    return result;
  }

  nlohmann::json base_subject::_to_json(const std::set<std::string>& filters,
					const std::vector<std::shared_ptr<prov_element> >& provs)
  {
    nlohmann::json result = _to_json(filters);

    result[prov_lbl] = get_prov_refs(provs);
      
    if(provs.size()>0)
      {
	result[base_subject::type_lbl] = provs.at(0)->get_type();
      }
    else // default type
      {
	result[base_subject::type_lbl] = "text";
      }      

    return result;
  }
  
  bool base_subject::_from_json(const nlohmann::json& item)
  {
    hash = item.value(subj_hash_lbl, hash);

    dloc = item.value(dloc_lbl, dloc);
    sref = item.value(sref_lbl, sref);

    bool read_props=true, read_insts=true, read_rels=true;

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

    applied_models.clear();
    if(item.count(applied_models_lbl))
      {
	applied_models = item.value(applied_models_lbl, applied_models);
      }
    else
      {
	for(auto& prop:properties)
	  {
	    applied_models.insert(prop.get_type());
	  }

	for(auto& inst:instances)
	  {
	    applied_models.insert(inst.get_type());
	  }

	for(auto& rel:relations)
	  {
	    applied_models.insert(rel.get_type());
	  }

      }
    
    for(auto implicit_model:implicit_models)
      {
	applied_models.erase(implicit_model);
      }

    return (read_props and read_insts and read_rels);
  }

  template<typename item_type>
  void base_subject::to_json(nlohmann::json& result,
                             std::string key,
			     std::vector<item_type>& vals)
  {
    nlohmann::json& json_vals = result[key];
    json_vals = nlohmann::json::array({});

    for(auto& val:vals)
      {
        json_vals.push_back(val->to_json());
      }
  }

  template<typename item_type, typename filters_type>
  void base_subject::to_json(nlohmann::json& result,
                             std::string key,
			     std::vector<item_type>& vals,
                             filters_type filters)
  {
    nlohmann::json& json_vals = result[key];
    json_vals = nlohmann::json::array({});
    
    for(auto& val:vals)
      {
        json_vals.push_back(val->to_json(filters));
      }
  }

  template<typename item_type>
  bool base_subject::from_json(const nlohmann::json& result,
			       std::string key,
			       std::vector<std::shared_ptr<item_type> >& vals)
  {
    vals.clear();
    
    if(not result.count(key))
      {
	LOG_S(WARNING) << "no " << key << " found in the document ...";
	return false;
      }
    
    auto& items = result.at(key);
    for(auto& item:items)
      {
	std::shared_ptr<item_type> val = std::make_shared<item_type>();
	val->from_json(item);
	
	vals.push_back(val);
      }
    
    return true;
  }
  
  template<typename item_type>
  bool base_subject::from_json(const nlohmann::json& result,
			       const std::vector<std::shared_ptr<prov_element> >& doc_provs,
			       std::string key,
			       std::vector<std::shared_ptr<item_type> >& vals)			       
  {
    vals.clear();
    
    if(not result.count(key))
      {
	LOG_S(WARNING) << "no " << key << " found in the document ...";
	return false;
      }
    
    auto& items = result.at(key);
    for(auto& item:items)
      {
	std::shared_ptr<item_type> val = std::make_shared<item_type>();
	val->from_json(item, doc_provs);
	
	vals.push_back(val);
      }
    
    return true;
  }

}

#endif
