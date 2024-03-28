//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_TEXT_H_
#define ANDROMEDA_SUBJECTS_TEXT_H_

namespace andromeda
{
  template<>
  class subject<TEXT>: public base_subject,
                       public text_element
  {
  public:

    subject();
    subject(uint64_t dhash, std::string dloc);
    subject(uint64_t dhash, std::string dloc,
            std::shared_ptr<prov_element> prov);

    virtual ~subject();

    void finalise();
    void clear();

    bool is_valid() { return (base_subject::valid and text_element::is_text_valid()); }

    std::vector<std::shared_ptr<prov_element> >& get_provs() { return provs; }
    
    virtual nlohmann::json to_json(const std::set<std::string>& filters);

    virtual bool from_json(const nlohmann::json& data);
    virtual bool from_json(const nlohmann::json& data,
			   const std::vector<std::shared_ptr<prov_element> >& doc_provs);
    
    bool concatenate(std::shared_ptr<subject<TEXT> > other);

    bool set_text(const std::string& ctext);
    bool set_type(const std::string& ctype);
    
    bool set_data(const nlohmann::json& item);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool set(const std::string& ctext,
             std::shared_ptr<utils::char_normaliser> char_normaliser,
             std::shared_ptr<utils::text_normaliser> text_normaliser);

    //void sort();

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

  subject<TEXT>::subject():
    base_subject(TEXT),
    labels({}),
    provs({})
  {}

  subject<TEXT>::subject(uint64_t dhash, std::string dloc):
    base_subject(dhash, dloc, TEXT),
    labels({}),
    provs({})
  {}

  subject<TEXT>::subject(uint64_t dhash, std::string dloc,
                         std::shared_ptr<prov_element> prov):
    base_subject(dhash, dloc, TEXT),
    labels({}),
    provs({prov})
  {}

  subject<TEXT>::~subject()
  {}

  void subject<TEXT>::finalise()
  {}

  void subject<TEXT>::clear()
  {
    base_subject::clear();
    text_element::clear();

    labels.clear();
    provs.clear();
  }

  // FIXME: we might need to add some rules for the text-concatenation ...
  bool subject<TEXT>::concatenate(std::shared_ptr<subject<TEXT> > other)
  {
    std::string ctext = text_element::text;
    auto offset = ctext.size();

    // FIXME ...
    ctext += other->text;

    if(provs.size()>0)
      {
        for(auto& prov:other->provs)
          {
            //prov->char_range.at(0) += offset;
            //prov->char_range.at(1) += offset;

            auto char_range = prov->get_char_range();
            char_range.at(0) += offset;
            char_range.at(1) += offset;

            prov->set_char_range(char_range);

            this->provs.push_back(prov);
          }
      }

    return set_text(ctext);
  }

  bool subject<TEXT>::set_text(const std::string& ctext)
  {
    //LOG_S(INFO) << "subject<TEXT>:: " << __FUNCTION__;
    
    text_element::set_text(ctext);

    //LOG_S(INFO) << " -> subject<TEXT>::text = '" << text_element::text << "'";
    //LOG_S(INFO) << " -> subject<TEXT>::dhash = '" << dhash << "'";
    //LOG_S(INFO) << " -> subject<TEXT>::text_hash = '" << text_element::text_hash << "'";

    std::vector<uint64_t> hashes={dhash, text_element::get_text_hash()};
    base_subject::hash = utils::to_hash(hashes);
    
    //LOG_S(INFO) << " -> base_subject::hash = " << base_subject::hash;
    //LOG_S(INFO) << " -> subject<TEXT>::hash = " << subject<TEXT>::hash;
    
    return text_element::is_text_valid();
  }

  bool subject<TEXT>::set_type(const std::string& ctype)
  {
    if(provs.size()>0)
      {
	for(auto prov:provs)
	  {
	    prov->set_type(ctype);
	  }
      }
    else
      {
	range_type rng = {0, get_len()};
	auto prov = std::make_shared<prov_element>("#", "#", "text", ctype, rng);

	provs.push_back(prov);
      }

    return true;
  }
  
  bool subject<TEXT>::set_data(const nlohmann::json& item)
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
        labels.insert(prov->get_name());
        labels.insert(prov->get_type());
      }

    return valid;
  }

  bool subject<TEXT>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                                 std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    return text_element::set_tokens(char_normaliser, text_normaliser);
  }

  bool subject<TEXT>::set(const std::string& ctext,
                          std::shared_ptr<utils::char_normaliser> char_normaliser,
                          std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    if(set_text(ctext))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }

 

  typename std::vector<base_instance>::iterator subject<TEXT>::insts_beg(std::array<uint64_t, 2> char_rng)
  {
    base_instance fake(base_subject::hash, TEXT, get_self_ref(),
		       NULL_MODEL, "fake", "fake", "fake",
                       char_rng, {0,0}, {0,0});

    return std::lower_bound(instances.begin(), instances.end(), fake);
  }

  typename std::vector<base_instance>::iterator subject<TEXT>::insts_end(std::array<uint64_t, 2> char_rng)
  {
    base_instance fake(base_subject::hash, TEXT, get_self_ref(),
		       NULL_MODEL, "fake", "fake", "fake",
                       char_rng, {0,0}, {0,0});

    return std::upper_bound(instances.begin(), instances.end(), fake);
  }

  bool subject<TEXT>::get_property_label(const std::string model_name, std::string& label)
  {
    for(auto& prop:properties)
      {
        //if(name==prop.get_type())
	if(prop.is_type(model_name))
          {
            label = prop.get_label();
            return true;
          }
      }

    return false;
  }

  std::string subject<TEXT>::get_text(range_type rng)
  {
    return text_element::get_text(rng);
  }

  void subject<TEXT>::apply_wtoken_contractions(std::vector<candidate_type>& candidates)
  {
    text_element::apply_word_contractions(candidates);

    for(auto& inst:instances)
      {
        inst.set_ctok_range(text_element::get_char_token_range(inst.get_char_range()));
        inst.set_wtok_range(text_element::get_word_token_range(inst.get_char_range()));

        inst.verify_wtok_range_match(word_tokens);
      }
  }

  void subject<TEXT>::contract_wtokens_from_instances(model_name model)
  {
    std::vector<candidate_type> candidates={};

    for(auto& inst:instances)
      {
        if(inst.is_model(model) and
           inst.get_wtok_range(0)<inst.get_wtok_range(1))
          {
	    std::vector<int> inds={};
	    std::vector<std::string> subws={};

	    for(index_type i=inst.get_wtok_range(0); i<inst.get_wtok_range(1); i++)
	      {
		auto _inds = word_tokens.at(i).get_inds();
		inds.insert(inds.end(), _inds.begin(), _inds.end());

		auto _subws = word_tokens.at(i).get_subws();
		subws.insert(subws.end(), _subws.begin(), _subws.end());
	      }
	    
            candidates.emplace_back(inst.get_wtok_range(0),
                                    inst.get_wtok_range(1),
                                    inst.get_name(),
				    inds, subws);
          }
      }

    apply_wtoken_contractions(candidates);

    for(auto& inst:instances)
      {
        if(inst.is_model(model) and
           inst.get_wtok_range(0)<inst.get_wtok_range(1))
          {
            word_tokens.at(inst.get_wtok_range(0)).set_tag(inst.get_subtype());
          }
      }
  }

  void subject<TEXT>::contract_wtokens_from_instances(model_name name, std::string subtype)
  {
    std::vector<candidate_type> candidates={};

    for(auto& inst:instances)
      {
        if(inst.is_model(name) and
           inst.is_subtype(subtype))
          {
	    std::vector<int> inds={};
	    std::vector<std::string> subws={};

	    for(index_type i=inst.get_wtok_range(0); i<inst.get_wtok_range(1); i++)
	      {
		auto _inds = word_tokens.at(i).get_inds();
		inds.insert(inds.end(), _inds.begin(), _inds.end());

		auto _subws = word_tokens.at(i).get_subws();
		subws.insert(subws.end(), _subws.begin(), _subws.end());
	      }
	    
            candidates.emplace_back(inst.get_wtok_range(0),
                                    inst.get_wtok_range(1),
                                    inst.get_name(),
				    inds, subws);
	    /*
            candidates.emplace_back(inst.get_wtok_range(0),
                                    inst.get_wtok_range(1),
                                    inst.get_name());
	    */
	  }
      }

    apply_wtoken_contractions(candidates);

    for(auto& inst:instances)
      {
        if(inst.is_model(name) and
           inst.is_subtype(subtype))
          {
            word_tokens.at(inst.get_wtok_range(0)).set_tag(inst.get_subtype());
          }
      }
  }

  nlohmann::json subject<TEXT>::to_json(const std::set<std::string>& filters)
  {
    nlohmann::json base = base_subject::_to_json(filters, provs);
    nlohmann::json elem = text_element::_to_json(filters);

    nlohmann::json result = nlohmann::json::object({});

    result.merge_patch(base);
    result.merge_patch(elem);
    
    return result;
  }
  
  bool subject<TEXT>::from_json(const nlohmann::json& data)
  {
    provs.clear();
    
    bool init_base = base_subject::_from_json(data);
    bool init_text = text_element::_from_json(data);

    if(not (init_base and init_text))
      {
	LOG_S(WARNING) << "init_base: " << init_base << ", init_text: " << init_text;
      }
    
    return (init_base and init_text);
  }

  bool subject<TEXT>::from_json(const nlohmann::json& data,
				const std::vector<std::shared_ptr<prov_element> >& doc_provs)
  {
    bool init_prov = base_subject::set_prov_refs(data, doc_provs, provs);
    
    bool init_base = base_subject::_from_json(data);
    bool init_text = text_element::_from_json(data);
    
    return (init_prov and init_base and init_text);    
  }
  
  void subject<TEXT>::show(bool txt, bool mdls,
                           bool ctok, bool wtok,
                           bool prps, bool insts, bool rels)
  {
    std::stringstream ss;

    if(provs.size()>0)
      {
        ss << "prov: "
           << provs.at(0)->get_page() << ", "
           << " ["
           << provs.at(0)->x0() << ", "
           << provs.at(0)->y0() << ", "
           << provs.at(0)->x1() << ", "
           << provs.at(0)->y1()
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
