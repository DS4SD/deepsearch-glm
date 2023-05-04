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
    nlp_model(std::filesystem::path resources_dir, bool verbose);
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return POS; }
    virtual model_name get_name() { return LAPOS; }

    virtual bool apply(subject<PARAGRAPH>& subj);
    virtual bool apply(subject<TABLE>& subj);

  private:

    bool initialise(std::filesystem::path resources_dir, bool verbose);

    bool check_dependency(subject<PARAGRAPH>& subj,
                          std::string& lang);

    bool contract_word_tokens(subject<PARAGRAPH>& subj);

    /*
    template<typename wtoken_type, typename index_type>
    void pre_process(std::vector<wtoken_type>& wtokens,
                     std::array<index_type, 2>& rng,
                     std::vector<pos_token_type>& pos_tokens,
                     std::map<index_type, index_type>& ptid_to_wtid);
    */

    void pre_process(std::vector<word_token>& wtokens,
                     //std::array<index_type, 2>& rng,
		     range_type& rng,
                     std::vector<pos_token_type>& pos_tokens,
                     std::map<index_type, index_type>& ptid_to_wtid);
    
    void run_pos_tagger(subject<PARAGRAPH>& subj,
                        std::shared_ptr<pos_model_type> pos_model);

    /*
    template<typename wtoken_type, typename index_type>
    void post_process(std::vector<wtoken_type>& wtokens,
                      std::vector<pos_token_type>& pos_tokens,
                      std::map<index_type, index_type>& ptid_to_wtid);
    */

    void post_process(std::vector<word_token>& wtokens,
                      std::vector<pos_token_type>& pos_tokens,
                      std::map<index_type, index_type>& ptid_to_wtid);
    
    bool post_process(nlohmann::json& ents);

  private:

    const static std::set<model_name> dependencies;

    std::filesystem::path model_file;

    std::map<std::string, std::shared_ptr<pos_model_type> > pos_models;
    std::set<std::string> numbers;
  };

  const std::set<model_name> nlp_model<POS, LAPOS>::dependencies = {LANGUAGE, SENTENCE};

  /*
    nlp_model<POS, LAPOS>::nlp_model()
    {
    initialise(andromeda::RESOURCES_DIR);
    }
  */

  nlp_model<POS, LAPOS>::nlp_model(std::filesystem::path resources_dir, bool verbose)
  {
    initialise(resources_dir, verbose);
  }

  nlp_model<POS, LAPOS>::~nlp_model()
  {}

  bool nlp_model<POS, LAPOS>::initialise(std::filesystem::path resources_dir, bool verbose)
  {
    {
      model_file = resources_dir / "models/crf/part-of-speech/en/model_wsj02-21/model.la";
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

  bool nlp_model<POS, LAPOS>::check_dependency(subject<PARAGRAPH>& subj, std::string& lang)
  {
    bool static_dependency = satisfies_dependencies(subj);

    bool dyn_dependency=false;

    for(auto& prop:subj.properties)
      {
        if(prop.get_type()==to_key(LANGUAGE) and
           pos_models.count(prop.get_name())==1)
          {
            lang = prop.get_name();
            dyn_dependency=true;
          }
      }

    return (static_dependency and dyn_dependency);
  }

  bool nlp_model<POS, LAPOS>::apply(subject<PARAGRAPH>& subj)
  {
    // initialise
    for(auto& token:subj.word_tokens)
      {
        token.set_pos(word_token::UNDEF_POS);
      }

    std::string lang="null";
    if(not check_dependency(subj, lang))
      {
        return false;
      }

    std::shared_ptr<pos_model_type> pos_model = pos_models.at(lang);

    run_pos_tagger(subj, pos_model);

    return update_applied_models(subj);
  }

  bool nlp_model<POS, LAPOS>::apply(subject<TABLE>& subj)
  {
    return false;
  }

  void nlp_model<POS, LAPOS>::run_pos_tagger(subject<PARAGRAPH>& subj,
                                             std::shared_ptr<pos_model_type> pos_model)
  {
    std::vector<pos_token_type> pos_tokens={};
    //std::map<std::size_t, std::size_t> ptid_to_wtid={};
    std::map<index_type, index_type> ptid_to_wtid={};

    auto& wtokens = subj.word_tokens;
    auto& entities = subj.entities;

    // iterate over the sentences ...
    for(auto& ent:entities)
      {
        if(ent.model_type!=SENTENCE)
          {
            continue;
          }

        pre_process(wtokens, ent.wtok_range, pos_tokens, ptid_to_wtid);

        pos_model->predict(pos_tokens);

        post_process(wtokens, pos_tokens, ptid_to_wtid);
      }
  }

  /*
  template<typename wtoken_type, typename index_type>
  void nlp_model<POS, LAPOS>::pre_process(std::vector<wtoken_type>& wtokens,
                                          std::array<index_type, 2>& rng,
                                          std::vector<pos_token_type>& pos_tokens,
                                          std::map<index_type, index_type>& ptid_to_wtid)
  */
  void nlp_model<POS, LAPOS>::pre_process(std::vector<word_token>& wtokens,
                                          //std::array<index_type, 2>& rng,
					  range_type& rng,
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

  /*
  template<typename wtoken_type, typename index_type>
  void nlp_model<POS, LAPOS>::post_process(std::vector<wtoken_type>& wtokens,
                                           std::vector<pos_token_type>& pos_tokens,
                                           std::map<index_type, index_type>& ptid_to_wtid)
  */
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
