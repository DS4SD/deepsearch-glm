//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_CUSTOM_CRF_H_
#define ANDROMEDA_MODELS_ENTITIES_CUSTOM_CRF_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, CUSTOM_CRF>: public base_crf_model
  {
    typedef typename word_token::range_type range_type;

    const static inline std::string TAG = "__"+to_string(CUSTOM_CRF)+"__";
    
  public:

    
    nlp_model(std::string desc);

    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return CUSTOM_CRF; }

    virtual bool apply(subject<TEXT>& subj);
    //virtual bool apply(subject<TABLE>& subj) { return false; }
    //virtual bool apply(subject<DOCUMENT>& subj);

  private:

    bool initialise();

    void run_model(subject<TEXT>& subj);

    void post_process(subject<TEXT>& subj);

    //std::string normalise_name(std::string orig);
    //void normalise_subject(subject<TEXT>& subj);

  private:

    const static inline std::set<model_name> dependencies = {};

    std::string custom_name, custom_file;
    
    std::filesystem::path model_file;
  };

  nlp_model<ENT, CUSTOM_CRF>::nlp_model(std::string desc):
    model_file()
  {
    pcre2_expr expr("custom-crf", "", R"(custom_crf\((?P<name>([a-zA-Z\-]+))\:(?P<file>(.+))\))");
    
    //std::vector<pcre2_item> items;
    pcre2_item item;
    if(expr.match(desc, item))
      {
	for(auto& grp:item.groups)
	  {
	    if(grp.group_name=="name")
	      {
		auto char_range = grp.rng;
		custom_name = desc.substr(char_range.at(0), char_range.at(1)-char_range.at(0));
	      }
	    else if(grp.group_name=="file")
	      {
		auto char_range = grp.rng;
		custom_file = desc.substr(char_range.at(0), char_range.at(1)-char_range.at(0));
	      }
	    else
	      {}
	  }

	model_file = custom_file.c_str();
	initialise();
      }
    else
      {
	LOG_S(ERROR) << "could not initialise custom-crf with desc: " << desc;
      }
  }

  /*
  nlp_model<ENT, CUSTOM_CRF>::nlp_model(std::filesystem::path model_file):
    model_file(model_file)
  {
    initialise();
  }
  */
  
  nlp_model<ENT, CUSTOM_CRF>::~nlp_model()
  {}

  bool nlp_model<ENT, CUSTOM_CRF>::initialise()
  {
    if(not base_crf_model::load(model_file, false))
      {
        LOG_S(ERROR) << "could not load CUSTOM_CRF model from " << model_file;
        return false;
      }

    return true;
  }

  bool nlp_model<ENT, CUSTOM_CRF>::apply(subject<TEXT>& subj)
  {
    //LOG_S(INFO) << __FILE__ << ":" << __LINE__ << "\t" << subj.get_text();

    if(not satisfies_dependencies(subj))
      {
        //LOG_S(WARNING) << "does not satisfy deps ... ";
        return false;
      }

    run_model(subj);

    post_process(subj);

    return true;
  }

  void nlp_model<ENT, CUSTOM_CRF>::run_model(subject<TEXT>& subj)
  {
    //LOG_S(WARNING) << __FILE__ << ":" << __LINE__ << "\t" << __FUNCTION__;

    std::vector<crf_token_type> crf_tokens={};
    std::map<std::size_t, std::size_t> ptid_to_wtid={};

    auto& wtokens = subj.get_word_tokens();
    //auto& entities = subj.entities;

    //pre_process(wtokens, ent.wtok_range, pos_tokens, ptid_to_wtid);

    for(std::size_t l=0; l<wtokens.size(); l++)
      {
        auto& wtoken = wtokens.at(l);

        crf_tokens.emplace_back(wtoken.get_word(),
                                wtoken.get_rng(0),
                                wtoken.get_rng(1));
      }

    base_crf_model::predict(crf_tokens);

    //andromeda_crf::tabulate(crf_tokens);
    
    for(std::size_t l=0; l<wtokens.size(); l++)
      {
        auto& wtoken = wtokens.at(l);
        auto& ptoken = crf_tokens.at(l);

        std::string label = TAG + ptoken.pred_label;
        wtoken.set_tag(label);
      }
    
  }

  void nlp_model<ENT, CUSTOM_CRF>::post_process(subject<TEXT>& subj)
  {
    auto& wtokens = subj.get_word_tokens();

    //std::map<std::string, std::vector<std::array<std::size_t, 2> > > labels_to_crng={};
    std::map<std::string, std::vector<typename word_token::range_type> > labels_to_crng={};

    for(std::size_t l=0; l<wtokens.size(); l++)
      {
        auto& wtoken = wtokens.at(l);

        for(auto& wtoken_tag:wtoken.get_tags())
          {
            if(wtoken_tag.starts_with(TAG))
              {
                auto label = utils::replace(wtoken_tag, TAG, "");

                auto itr = labels_to_crng.find(label);

                if(itr==labels_to_crng.end())
                  {
                    labels_to_crng[label]={};
                  }

                labels_to_crng.at(label).push_back(wtoken.get_rng());
              }
          }
      }

    std::vector<std::string> labels = this->get_labels();
    
    for(const auto& label:labels)
      {
	if(label=="null")
	  {
	    continue;
	  }
	
        if(labels_to_crng.count(label)==0)
          {
            continue;
          }

        auto& ranges = labels_to_crng.at(label);

        std::size_t ind=0;
        while(ind<ranges.size())
          {
            range_type char_range = ranges.at(ind++);

            while(ind<ranges.size())
              {
                auto rng = ranges.at(ind);
                if(rng[0]-char_range[1]<=1)
                  {
                    char_range[1] = rng[1];
                  }
                else
                  {
                    break;
                  }

                ind += 1;
              }

            auto ctok_range = subj.get_char_token_range(char_range);
            auto wtok_range = subj.get_word_token_range(char_range);

            std::string orig = subj.from_char_range(char_range);
            std::string name = subj.from_ctok_range(ctok_range);
            	    
            subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
					CUSTOM_CRF, label,
                                        name, orig,
                                        char_range, ctok_range, wtok_range);
          }
      }

  }
  
}

#endif