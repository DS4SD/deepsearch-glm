//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_PARAGRAPH_H_
#define ANDROMEDA_SUBJECTS_PARAGRAPH_H_

namespace andromeda
{
  template<>
  class subject<PARAGRAPH>: public base_subject,
			    public text_element
  {
  public:

    subject();
    subject(uint64_t dhash);
    subject(uint64_t dhash, std::shared_ptr<prov_element> prov);
    
    ~subject();

    void finalise();
    
    std::string get_path() const { return (provs.size()>0? (provs.at(0)->path):"#"); }
    
    void clear();

    bool is_valid() { return (base_subject::valid and text_element::text_valid); }
    
    nlohmann::json to_json();
    bool from_json(const nlohmann::json& data);

    bool concatenate(std::shared_ptr<subject<PARAGRAPH> > other);
    
    bool set_text(const std::string& ctext);
    bool set_data(const nlohmann::json& item);
    
    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
		    std::shared_ptr<utils::text_normaliser> text_normaliser);
    
    bool set(const std::string& ctext,
	     std::shared_ptr<utils::char_normaliser> char_normaliser,
	     std::shared_ptr<utils::text_normaliser> text_normaliser);

    void sort();

    typename std::vector<base_instance>::iterator insts_beg(std::array<uint64_t, 2> char_rng);
    typename std::vector<base_instance>::iterator insts_end(std::array<uint64_t, 2> char_rng);
    
    bool get_property_label(const std::string name, std::string& label);

    std::string get_text(range_type rng);
    
    std::string get_text() const { return this->text; }

    void apply_wtoken_contractions(std::vector<candidate_type>& candidates);

    void contract_wtokens_from_instances(model_name name);
    void contract_wtokens_from_instances(model_name name, std::string subtype);

    void show(bool txt=true,
              bool mdls=false,

              bool ctokens=false,
              bool wtokens=true,

              bool prps=true,
              bool insts=true,
              bool rels=true);

  public:

    std::set<std::string> labels;

    std::vector<std::shared_ptr<prov_element> > provs;
  };

  subject<PARAGRAPH>::subject():
    base_subject(PARAGRAPH),
    labels({}),
    provs({})
  {}

  subject<PARAGRAPH>::subject(uint64_t dhash):
    base_subject(dhash, PARAGRAPH),
    labels({}),
    provs({})
  {}
  
  subject<PARAGRAPH>::subject(uint64_t dhash, std::shared_ptr<prov_element> prov):
    base_subject(dhash, PARAGRAPH),
    labels({}),
    provs({prov})
  {}
  
  subject<PARAGRAPH>::~subject()
  {}

  void subject<PARAGRAPH>::finalise()
  {
    // FIXME ...
  }
  
  void subject<PARAGRAPH>::clear()
  {
    base_subject::clear();
    text_element::clear();
    
    labels.clear();
    provs.clear();
  }

  // FIXME: we might need to add some rules for the text-concatenation ...
  bool subject<PARAGRAPH>::concatenate(std::shared_ptr<subject<PARAGRAPH> > other)
  {
    std::string ctext = text_element::text;
    auto offset = ctext.size();

    // FIXME ...
    ctext += other->text;
    
    if(provs.size()>0)
      {
	//auto ind = provs.front()->dref.second;

	for(auto& prov:other->provs)
	  {
	    prov->char_range.at(0) += offset;
	    prov->char_range.at(1) += offset;
	    
	    this->provs.push_back(prov);	
	  }
      }
    
    return set_text(ctext);
  }
  
  bool subject<PARAGRAPH>::set_text(const std::string& ctext)
  {
    text_element::set_text(ctext);

    std::vector<uint64_t> hashes={dhash, text_element::text_hash};
    base_subject::hash = utils::to_hash(hashes);

    return text_element::text_valid;
  }

  bool subject<PARAGRAPH>::set_data(const nlohmann::json& item)
  {
    base_subject::clear_models();
    text_element::clear();

    bool valid=false;
    if(item.count("text"))
      {
	std::string ctext = item["text"].get<std::string>();
	valid = set_text(ctext);
      }
    else
      {
	return false;
      }

    for(auto& prov:provs)
      {
	labels.insert(prov->name);
	labels.insert(prov->type);
      }
    
    return valid;
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
    std::sort(instances.begin(), instances.end());
  }

  typename std::vector<base_instance>::iterator subject<PARAGRAPH>::insts_beg(std::array<uint64_t, 2> char_rng)
  {
    base_instance fake(base_subject::hash, NULL_MODEL, "fake", "fake", "fake",
		       char_rng, {0,0}, {0,0});
    
    return std::lower_bound(instances.begin(), instances.end(), fake);    
  }
  
  typename std::vector<base_instance>::iterator subject<PARAGRAPH>::insts_end(std::array<uint64_t, 2> char_rng)
  {
    base_instance fake(base_subject::hash, NULL_MODEL, "fake", "fake", "fake",
		       char_rng, {0,0}, {0,0});

    return std::upper_bound(instances.begin(), instances.end(), fake);    
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

    for(auto& inst:instances)
      {
        inst.ctok_range = text_element::get_char_token_range(inst.char_range);
        inst.wtok_range = text_element::get_word_token_range(inst.char_range);

        inst.verify_wtok_range_match(word_tokens);
      }
  }

  void subject<PARAGRAPH>::contract_wtokens_from_instances(model_name name)
  {
    std::vector<candidate_type> candidates={};

    for(auto& inst:instances)
      {
        if(inst.model_type==name and
	   inst.wtok_range[0]<inst.wtok_range[1])
          {
            candidates.emplace_back(inst.wtok_range[0],
                                    inst.wtok_range[1],
                                    inst.name);
          }
      }

    apply_wtoken_contractions(candidates);

    for(auto& inst:instances)
      {
        if(inst.model_type==name and
	   inst.wtok_range[0]<inst.wtok_range[1])
          {
            word_tokens.at(inst.wtok_range[0]).set_tag(inst.model_subtype);
          }
      }
  }

  void subject<PARAGRAPH>::contract_wtokens_from_instances(model_name name, std::string subtype)
  {
    std::vector<candidate_type> candidates={};

    for(auto& inst:instances)
      {
        if(inst.model_type==name and
           inst.model_subtype==subtype)
          {
            candidates.emplace_back(inst.wtok_range[0],
                                    inst.wtok_range[1],
                                    inst.name);
          }
      }

    apply_wtoken_contractions(candidates);

    for(auto& inst:instances)
      {
        if(inst.model_type==name and
           inst.model_subtype==subtype)
          {
            word_tokens.at(inst.wtok_range[0]).set_tag(inst.model_subtype);
          }
      }
  }

  nlohmann::json subject<PARAGRAPH>::to_json()
  {
    nlohmann::json result = base_subject::to_json();
    
    {      
      result["orig"] = text_element::orig;
      result["text"] = text_element::text;

      result["text-hash"] = text_element::text_hash;
      
      result["word-tokens"] = andromeda::to_json(text_element::word_tokens, text);

      result["prov"] = nlohmann::json::array({});
      for(auto& prov:provs)
	{
	  nlohmann::json item = prov->to_json();
	  result["prov"].push_back(item);
	}
    }
    
    return result;
  }

  bool subject<PARAGRAPH>::from_json(const nlohmann::json& data)
  {    
    if(data.count("hash")>0 and data.count("applied-models")>0 and
       data.count("orig")>0 and data.count("text")>0)
      {
	text_element::text_hash = data.value("text-hash", text_element::text_hash);

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
                                bool prps, bool insts, bool rels)
  {
    std::stringstream ss;

    if(provs.size()>0)
      {
	ss << "prov: "
	   << provs.at(0)->page << ", "
	   << " ["
	   << provs.at(0)->bbox[0] << ", "
	   << provs.at(0)->bbox[1] << ", "
	   << provs.at(0)->bbox[2] << ", "
	   << provs.at(0)->bbox[3]
	   << "]\n";
      }
    else
      {
	ss << "no provenance \n";
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

    if(insts)
      {
        ss << tabulate(text, instances);
      }

    if(rels)
      {
        ss << tabulate(instances, relations);
      }

    LOG_S(INFO) << "NLP-output: \n" << ss.str();
  }

}

#endif
