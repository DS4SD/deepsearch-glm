//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_PARAGRAPH_H_
#define ANDROMEDA_SUBJECTS_PARAGRAPH_H_

namespace andromeda
{
  template<>
  class subject<PARAGRAPH>: public provenance,
			    public text_element
  {
  public:

    subject();
    subject(uint64_t dhash,
	    uint64_t index);
    
    ~subject();

    void clear();

    nlohmann::json to_json();
    bool from_json(const nlohmann::json& data);
    
    bool set_text(const std::string& ctext);
    bool set_data(const nlohmann::json& item);
    
    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
		    std::shared_ptr<utils::text_normaliser> text_normaliser);
    
    bool set(const std::string& ctext,
	     std::shared_ptr<utils::char_normaliser> char_normaliser,
	     std::shared_ptr<utils::text_normaliser> text_normaliser);

    void sort();

    typename std::vector<base_entity>::iterator ents_beg(std::array<uint64_t, 2> char_rng);
    typename std::vector<base_entity>::iterator ents_end(std::array<uint64_t, 2> char_rng);
    
    bool get_property_label(const std::string name, std::string& label);

    uint64_t get_hash() const { return hash; }
    std::string get_text() const;

    std::string get_text(range_type rng);

    void apply_wtoken_contractions(std::vector<candidate_type>& candidates);

    void contract_wtokens_from_entities(model_name name);
    void contract_wtokens_from_entities(model_name name, std::string subtype);

    void show(bool txt=true,
              bool mdls=false,

              bool ctokens=false,
              bool wtokens=true,

              bool prps=true,
              bool ents=true,
              bool rels=true);

  public:

    uint64_t hash;
    
    uint64_t dhash;
    uint64_t index;
    
    std::set<std::string> applied_models, labels;

    std::vector<base_property> properties;
    std::vector<base_entity> entities;
    std::vector<base_relation> relations;
  };

  subject<PARAGRAPH>::subject():
    dhash(-1),
    index(-1),
    
    applied_models(),
    labels(),
    
    properties({}),
    entities({}),
    relations({})
  {}

  subject<PARAGRAPH>::subject(uint64_t dhash,
			      uint64_t index):
    dhash(dhash),
    index(index),
    
    applied_models(),
    labels(),
    
    properties({}),
    entities({}),
    relations({})
  {}
  
  subject<PARAGRAPH>::~subject()
  {}

  void subject<PARAGRAPH>::clear()
  {
    text_element::clear();

    applied_models.clear();
    labels.clear();
      
    properties.clear();
    entities.clear();
    relations.clear();
  }

  bool subject<PARAGRAPH>::set_text(const std::string& ctext)
  {
    clear();
    
    text_element::set_text(ctext);

    std::vector<uint64_t> hashes={dhash, hash};
    hash = utils::to_hash(hashes);

    return text_element::valid;
  }

  bool subject<PARAGRAPH>::set_data(const nlohmann::json& item)
  {    
    if(item.count("text"))
      {
	std::string ctext = item["text"].get<std::string>();
	set_text(ctext);
      }
    else
      {
	return false;
      }

    if(item.count("type"))
      {
	std::string label = item["type"].get<std::string>();
	labels.insert(label);
      }

    if(item.count("name"))
      {
	std::string label = item["name"].get<std::string>();
	labels.insert(label);
      }    
    
    if(item.count("prov"))
      {
	provenance::set(item["prov"]);
      }

    return true;
  }

  bool subject<PARAGRAPH>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
				      std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    return text_element::set_tokens(char_normaliser, text_normaliser);
  }  
  
  bool subject<PARAGRAPH>::set(const std::string& ctext,
			       std::shared_ptr<utils::char_normaliser> char_normaliser,
			       std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    if(set_text(ctext))
      {
	return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }

  void subject<PARAGRAPH>::sort()
  {
    std::sort(entities.begin(), entities.end());
  }

  typename std::vector<base_entity>::iterator subject<PARAGRAPH>::ents_beg(std::array<uint64_t, 2> char_rng)
  {
    base_entity fake(NULL_MODEL, "fake", "fake", "fake",
		     char_rng, {0,0}, {0,0});
    
    return std::lower_bound(entities.begin(), entities.end(), fake);    
  }
  
  typename std::vector<base_entity>::iterator subject<PARAGRAPH>::ents_end(std::array<uint64_t, 2> char_rng)
  {
    base_entity fake(NULL_MODEL, "fake", "fake", "fake",
		     char_rng, {0,0}, {0,0});

    return std::upper_bound(entities.begin(), entities.end(), fake);    
  }
  
  bool subject<PARAGRAPH>::get_property_label(const std::string name, std::string& label)
  {
    for(auto& prop:properties)
      {
	if(name==prop.get_type())
	  {
	    label = prop.get_name();
	    return true;
	  }
      }

    return false;
  }

  std::string subject<PARAGRAPH>::get_text(range_type rng)
  {
    return text_element::get_text(rng);
  }

  void subject<PARAGRAPH>::apply_wtoken_contractions(std::vector<candidate_type>& candidates)
  {
    text_element::apply_word_contractions(candidates);

    for(auto& ent:entities)
      {
        ent.ctok_range = text_element::get_char_token_range(ent.char_range);
        ent.wtok_range = text_element::get_word_token_range(ent.char_range);

        ent.verify_wtok_range_match(word_tokens);
      }
  }

  void subject<PARAGRAPH>::contract_wtokens_from_entities(model_name name)
  {
    std::vector<candidate_type> candidates={};

    for(auto& ent:entities)
      {
        if(ent.model_type==name and
	   ent.wtok_range[0]<ent.wtok_range[1])
          {
            candidates.emplace_back(ent.wtok_range[0],
                                    ent.wtok_range[1],
                                    ent.name);
          }
      }

    apply_wtoken_contractions(candidates);

    for(auto& ent:entities)
      {
        if(ent.model_type==name and
	   ent.wtok_range[0]<ent.wtok_range[1])
          {
            word_tokens.at(ent.wtok_range[0]).set_tag(ent.model_subtype);
          }
      }
  }

  void subject<PARAGRAPH>::contract_wtokens_from_entities(model_name name, std::string subtype)
  {
    std::vector<candidate_type> candidates={};

    for(auto& ent:entities)
      {
        if(ent.model_type==name and
           ent.model_subtype==subtype)
          {
            candidates.emplace_back(ent.wtok_range[0],
                                    ent.wtok_range[1],
                                    ent.name);
          }
      }

    apply_wtoken_contractions(candidates);

    for(auto& ent:entities)
      {
        if(ent.model_type==name and
           ent.model_subtype==subtype)
          {
            word_tokens.at(ent.wtok_range[0]).set_tag(ent.model_subtype);
          }
      }
  }

  nlohmann::json subject<PARAGRAPH>::to_json()
  {
    nlohmann::json result = nlohmann::json::object({});

    {
      result["hash"] = text_element::hash;
      
      result["orig"] = text_element::orig;
      result["text"] = text_element::text;

      result["word-tokens"] = andromeda::to_json(text_element::word_tokens, text);
    }

    result["applied-models"] = applied_models;
    
    {
      nlohmann::json& props = result["properties"];
      andromeda::to_json(properties, props);
    }

    {
      nlohmann::json& ents = result["entities"];
      ents = nlohmann::json::object({});

      ents["headers"] = base_entity::headers(PARAGRAPH);

      nlohmann::json& data = ents["data"];      
      data = nlohmann::json::array({});

      for(std::size_t l=0; l<entities.size(); l++)
	{
	  data.push_back(entities.at(l).to_json_row(PARAGRAPH));
	}
    }

    {
      nlohmann::json& rels = result["relations"];
      rels = nlohmann::json::object({});

      rels["headers"] = base_relation::headers();

      nlohmann::json& data = rels["data"];      
      data = nlohmann::json::array({});
      
      for(std::size_t l=0; l<relations.size(); l++)
	{
	  data.push_back(relations.at(l).to_json_row());
	}      
    }

    return result;
  }

  bool subject<PARAGRAPH>::from_json(const nlohmann::json& data)
  {    
    if(data.count("hash")>0 and data.count("applied-models")>0 and
       data.count("orig")>0 and data.count("text")>0)
      {
	text_element::hash = data.value("hash", text_element::hash);
	applied_models = data.value("applied-models", applied_models);
	
	text_element::orig = data.value("orig", text_element::orig);
	text_element::text = data.value("text", text_element::text);
      }
    else
      {
	LOG_S(WARNING) << "could not read `hash`, `applied-models`, `orig` and `text` labels";
	return false;
      }

    if(data.count("word-tokens"))
      {
	auto& wtokens = data["word-tokens"];
	andromeda::from_json(text_element::word_tokens, wtokens);	
      }
    else
      {
	LOG_S(WARNING) << "could not read `word-tokens`";
	return false;
      }
    
    if(data.count("properties"))
      {
	auto& props = data["properties"];
	andromeda::from_json(properties, props);	
      }
    else
      {
	LOG_S(WARNING) << "could not read `properties`";
	return false;
      }

    return true;
  }
  
  void subject<PARAGRAPH>::show(bool txt, bool mdls,
                                bool ctok, bool wtok,
                                bool prps, bool ents, bool rels)
  {
    std::stringstream ss;

    if(provenance::elements.size()>0)
      {
	ss << "prov: "
	   << provenance::elements.at(0).page << ", "
	   << " ["
	   << provenance::elements.at(0).bbox[0] << ", "
	   << provenance::elements.at(0).bbox[1] << ", "
	   << provenance::elements.at(0).bbox[2] << ", "
	   << provenance::elements.at(0).bbox[3]
	   << "]";
      }
    
    if(txt)
      {
        ss << "\ntext: ";
        for(auto label:labels)
          {
            ss << label << ", ";
          }
        ss << "[done]\n";
	
        utils::show_string(text, ss, 6);
      }

    if(mdls)
      {
        ss << "\nmodels: ";
        for(auto model:applied_models)
          {
            ss << model << ", ";
          }
        ss << "[done]\n";
      }

    if(ctok)
      {
        ss << "\nchar-tokens: \n";
        ss << tabulate(char_tokens);
      }

    if(wtok)
      {
        ss << "\nword-tokens: \n";
        ss << tabulate(word_tokens, text);
      }

    if(prps)
      {
        ss << tabulate(properties);
      }

    if(ents)
      {
        ss << tabulate(text, entities);
      }

    if(rels)
      {
        ss << tabulate(entities, relations);
      }

    LOG_S(INFO) << "NLP-output: \n" << ss.str();
  }

}

#endif
