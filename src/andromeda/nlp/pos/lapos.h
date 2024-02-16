//-*-C++-*-

#ifndef ANDROMEDA_MODELS_POS_LAPOS_H_
#define ANDROMEDA_MODELS_POS_LAPOS_H_

namespace andromeda
{

  template<>
  class nlp_model<POS, LAPOS>: public base_nlp_model
  {
    //typedef typename word_token::index_type index_type;
    
    typedef typename word_token::index_type index_type;
    typedef typename word_token::range_type range_type;
    
    typedef          andromeda_crf::predicter   pos_model_type;
    typedef typename pos_model_type::token_type pos_token_type;

  public:

    nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return POS; }
    virtual model_name get_name() { return LAPOS; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply(subject<TABLE>& subj);

  private:

    bool initialise(bool verbose);

    template<typename subject_type>
    bool check_dependency(const std::set<model_name>& deps,
			  subject_type& subj, std::string& lang);

    bool contract_word_tokens(subject<TEXT>& subj);

    void pre_process(const std::vector<word_token>& wtokens,
		     const range_type rng,
                     std::vector<pos_token_type>& pos_tokens,
                     std::map<index_type, index_type>& ptid_to_wtid);
    
    void run_pos_tagger(subject<TEXT>& subj,
                        std::shared_ptr<pos_model_type> pos_model);

    void run_pos_tagger(subject<TABLE>& subj,
                        std::shared_ptr<pos_model_type> pos_model);    

    void post_process(std::vector<word_token>& wtokens,
                      std::vector<pos_token_type>& pos_tokens,
                      std::map<index_type, index_type>& ptid_to_wtid);
    
    bool post_process(nlohmann::json& insts);

  private:

    const static inline std::set<model_name> text_dependencies = {LANGUAGE, SENTENCE};

    const static inline std::set<model_name> table_dependencies = {LANGUAGE};

    const static inline std::set<model_name> dependencies = text_dependencies;
    
    std::filesystem::path model_file;

    std::map<std::string, std::shared_ptr<pos_model_type> > pos_models;
    std::set<std::string> numbers;
  };

  nlp_model<POS, LAPOS>::nlp_model():
    model_file(get_resources_dir() / "models/crf/part-of-speech/crf_pos_model_en.bin")
  {
    initialise(false);
  }

  /*
  bool nlp_model<POS, LAPOS>::initialise(std::filesystem::path resources_dir, bool verbose)
  {
    {
      //model_file = resources_dir / "models/crf/part-of-speech/en/model_wsj02-21/model.la";
      //auto pos_model = std::make_shared<lapos::predicter>(model_file);

      auto pos_model = std::make_shared<pos_model_type>(model_file, "default", verbose);

      std::string lang = "en";
      pos_models[lang] = pos_model;
    }

    // FIXME: put this in resources ...
    {
      numbers = {"zero",
                 "one", "two", "three", "four", "five",
                 "six", "seven", "eight", "nine", "ten",
                 "eleven", "twelve", "thirteen",
                 "twenty", "thirty"};
    }

    return true;
  }
  */

  bool nlp_model<POS, LAPOS>::initialise(bool verbose)
  {
    {
      auto pos_model = std::make_shared<pos_model_type>(model_file, "default", verbose);

      std::string lang = "en";
      pos_models[lang] = pos_model;
    }

    // FIXME: put this in resources ...
    {
      numbers = {"zero",
                 "one", "two", "three", "four", "five",
                 "six", "seven", "eight", "nine", "ten",
                 "eleven", "twelve", "thirteen",
                 "twenty", "thirty"};
    }

    return true;
  }
  
  template<typename subject_type>
  bool nlp_model<POS, LAPOS>::check_dependency(const std::set<model_name>& deps,
					       subject_type& subj, std::string& lang)
  {
    //LOG_S(INFO) << __FUNCTION__;
    
    bool static_dependency = satisfies_dependencies(subj, deps);

    bool dyn_dependency=false;

    for(auto& prop:subj.properties)
      {
        if(prop.get_type()==to_key(LANGUAGE) and
           pos_models.count(prop.get_label())==1)
          {
            lang = prop.get_label();
            dyn_dependency=true;
          }
      }
    
    return (static_dependency and dyn_dependency);
  }

  bool nlp_model<POS, LAPOS>::apply(subject<TEXT>& subj)
  {
    // initialise
    //for(auto& token:subj.get_word_tokens())
    //{
    //token.set_pos(word_token::UNDEF_POS);
    //}

    subj.init_pos();
    
    std::string lang="null";
    if(not check_dependency(text_dependencies, subj, lang))
      {
	//LOG_S(WARNING) << "skipping POS ...";
        return false;
      }

    std::shared_ptr<pos_model_type> pos_model = pos_models.at(lang);

    run_pos_tagger(subj, pos_model);

    return update_applied_models(subj);
  }

  void nlp_model<POS, LAPOS>::run_pos_tagger(subject<TEXT>& subj,
                                             std::shared_ptr<pos_model_type> pos_model)
  {
    std::vector<pos_token_type> pos_tokens={};
    std::map<index_type, index_type> ptid_to_wtid={};

    auto& wtokens = subj.get_word_tokens();
    auto& instances = subj.instances;
    
    //LOG_S(INFO) << "text: " << subj.get_text();
    //LOG_S(INFO) << "#-wtokens: " << wtokens.size();
    
    std::vector<range_type> sent_ranges={};
    for(auto& inst:instances)
      {
        if(inst.is_model(SENTENCE))
          {
	    sent_ranges.push_back(inst.get_wtok_range());

	    /*
	    LOG_S(INFO) << "sentence (" << inst.get_subtype() << ") : "
			<< sent_ranges.back().at(0) << ", "
			<< sent_ranges.back().at(1);
	    */
	  }
      }
    
    for(auto& rng:sent_ranges)
      {
        pre_process(wtokens, rng, pos_tokens, ptid_to_wtid);

        pos_model->predict(pos_tokens);

        post_process(wtokens, pos_tokens, ptid_to_wtid);
      }    
  }
  
  bool nlp_model<POS, LAPOS>::apply(subject<TABLE>& subj)
  {
    //LOG_S(INFO) << "nlp_model<POS, LAPOS>::apply(subject<TABLE>& subj)";
    
    std::string lang="null";
    if(not check_dependency(table_dependencies, subj, lang))
      {
        return false;
      }
    
    std::shared_ptr<pos_model_type> pos_model = pos_models.at(lang);
    
    for(std::size_t i=0; i<subj.num_rows(); i++)
      {
	for(std::size_t j=0; j<subj.num_cols(); j++)
	  {	    
	    auto& word_tokens = subj(i,j).get_word_tokens();
	    
	    // initialise
	    for(auto& word_token:word_tokens)
	      {
		word_token.set_pos(word_token::UNDEF_POS);
	      }

	    if(subj(i,j).is_numeric() or
	       word_tokens.size()==0)
	      {
		continue;
	      }
	    
	    range_type rng = {0, word_tokens.size()};
	    
	    std::vector<pos_token_type> pos_tokens={};	    
	    std::map<index_type, index_type> ptid_to_wtid={};
	    
	    pre_process(word_tokens, rng, pos_tokens, ptid_to_wtid);
	    
	    pos_model->predict(pos_tokens);
	    
	    post_process(word_tokens, pos_tokens, ptid_to_wtid);

	    //LOG_S(INFO) << andromeda::tabulate(word_tokens, subj(i,j).get_text());
	  }
      }
    
    return update_applied_models(subj);
  }

  void nlp_model<POS, LAPOS>::pre_process(const std::vector<word_token>& wtokens,
					  const range_type rng,
                                          std::vector<pos_token_type>& pos_tokens,
                                          std::map<index_type, index_type>& ptid_to_wtid)  
  {
    pos_tokens.clear();
    ptid_to_wtid.clear();

    std::size_t t0 = rng[0];
    std::size_t t1 = rng[1];

    for(auto wtid=t0; wtid<t1; wtid++)
      {
        auto& token = wtokens.at(wtid);

        auto ptid = pos_tokens.size();
        ptid_to_wtid[ptid] = wtid;

        if(token.has_tag("ival"))
          {
            pos_tokens.emplace_back("1", token.get_rng(0), token.get_rng(1));
          }
        else if(token.has_tag("fval"))
          {
            pos_tokens.emplace_back("1.0", token.get_rng(0), token.get_rng(1));
          }
        else if(token.has_tag("url"))
          {
            pos_tokens.emplace_back("url", token.get_rng(0), token.get_rng(1));
          }
        else if(token.has_tag("doi"))
          {
            pos_tokens.emplace_back("doi", token.get_rng(0), token.get_rng(1));
          }
        else if(token.has_tag("email"))
          {
            pos_tokens.emplace_back("email", token.get_rng(0), token.get_rng(1));
          }
        else
          {
            pos_tokens.emplace_back(token.get_word(), token.get_rng(0), token.get_rng(1));
          }
      }
  }

  void nlp_model<POS, LAPOS>::post_process(std::vector<word_token>& wtokens,
                                           std::vector<pos_token_type>& pos_tokens,
                                           std::map<index_type, index_type>& ptid_to_wtid)
  {
    std::set<std::string> protected_pos = {".", ",", ";", ":"};

    for(auto itr=ptid_to_wtid.begin(); itr!=ptid_to_wtid.end(); itr++)
      {
        index_type ptid = itr->first;
        index_type wtid = itr->second;

        auto& ptoken = pos_tokens.at(ptid);
        auto& wtoken = wtokens.at(wtid);

        //std::string pos = ptoken.prd;
        std::string pos = ptoken.pred_label;
        //double conf = ptoken.pred_conf;

        if(protected_pos.count(pos)==1)
          {}
        else if(pos.size()==1 or
                pos=="''" or
                pos.ends_with("-") or
                pos.starts_with("-"))
          {
            pos = "SYMBOL";
          }
        else if(pos.ends_with("$"))
          {
            pos = pos.substr(0, pos.size()-1);
          }
        else
          {}

        if(pos=="CD")
          {
            std::string word = wtoken.get_word();
            char c = word.front();

            if(numbers.count(word))
              {
                continue;
              }
            else if('A'<=c and c<='Z')
              {
                wtoken.set_pos("NNP");
              }
            else if('a'<=c and c<='z')
              {
                wtoken.set_pos("NN");
              }
            else
              {
                wtoken.set_pos(pos);
              }
          }
        else
          {
            wtoken.set_pos(pos);
          }
      }

  }

}

#endif
