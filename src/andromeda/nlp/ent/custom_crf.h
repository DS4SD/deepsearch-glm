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

    const static inline std::set<std::string> ignored_labels
    = {"null", "none", "undef", "__undef__"};
    
  public:

    nlp_model();
    
    nlp_model(std::string desc);
    
    nlp_model(std::string name, std::string file,
	      std::filesystem::path model_file);

    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return CUSTOM_CRF; }

    virtual std::string get_key() {
      std::stringstream ss;
      ss << to_key(this->get_name()) << "(" << custom_name << ":" << custom_file << ")";
      
      return ss.str();
    }
    
    virtual bool apply(subject<TEXT>& subj);

  protected:

    bool is_null_label(std::string label);
    
    bool initialise();

    void run_model(subject<TEXT>& subj);

    void post_process(subject<TEXT>& subj);

    // if the tokens are annotated with B-/I-/O format
    void post_process_bio(subject<TEXT>& subj);

    void identify_missing_I(subject<TEXT>& subj);
    void identify_rogue_I(subject<TEXT>& subj);
    
  protected:

    const static inline std::set<model_name> dependencies = {};

    std::string custom_name, custom_file;
    std::filesystem::path model_file;
  };

  nlp_model<ENT, CUSTOM_CRF>::nlp_model()
  {}
  
  nlp_model<ENT, CUSTOM_CRF>::nlp_model(std::string desc):
    model_file()
  {
    pcre2_expr expr("custom-crf", "", R"(custom_crf\((?P<name>([a-zA-Z\-]+))\:(?P<file>(.+))\))");
    
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
	LOG_S(WARNING) << "could not initialise custom-crf with desc: " << desc;
      }
  }

  nlp_model<ENT, CUSTOM_CRF>::nlp_model(std::string name,
					std::string file,
					std::filesystem::path model_file):
    custom_name(name),
    custom_file(file),
    model_file(model_file)
  {
    initialise();
  }
  
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

  bool nlp_model<ENT, CUSTOM_CRF>::is_null_label(std::string label)
  {
    for(auto& ignored_label:ignored_labels)
      {
	if(utils::contains(label, ignored_label))
	  {
	    return true;
	  }
      }
    
    return false;
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
	//if(label=="null")
	if(ignored_labels.count(label))
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
					//CUSTOM_CRF, label,
					this->get_name(), label,

					name, orig,
                                        char_range, ctok_range, wtok_range);
          }
      }

  }

  void nlp_model<ENT, CUSTOM_CRF>::identify_missing_I(subject<TEXT>& subj)
  {
    auto& wtokens = subj.get_word_tokens();

    // update wrongly identified O (1 O between two I-)
    for(std::size_t l=1; l+1<wtokens.size(); l++)
      {
	auto& wtoken = wtokens.at(l);

	std::string tag_l = "";
	for(auto& wtoken_tag:wtoken.get_tags())
	  {
	    if(wtoken_tag.starts_with(TAG))
	      {
		tag_l = wtoken_tag;
	      }
	  }

	std::string tag_lm1 = "";
	for(auto& wtoken_tag:wtokens.at(l-1).get_tags())
	  {
	    if(wtoken_tag.starts_with(TAG))
	      {
		tag_lm1 = wtoken_tag;
	      }
	  }

	std::string tag_lp1 = "";
	for(auto& wtoken_tag:wtokens.at(l+1).get_tags())
	  {
	    if(wtoken_tag.starts_with(TAG))
	      {
		tag_lp1 = wtoken_tag;
	      }
	  }

	if((tag_lm1.starts_with(TAG+"B_") or tag_lm1.starts_with(TAG+"I_")) and
	   //((not tag_l.starts_with(TAG+"B_")) and (not tag_l.starts_with(TAG+"I_"))) and
	   (is_null_label(tag_l)) and
	   (tag_lp1.starts_with(TAG+"I_")))
	  {
	    //LOG_S(WARNING) << tag_l << "\t" << ": updating to " << tag_lp1;
	    
	    wtokens.at(l).remove_tag(tag_l);
	    wtokens.at(l).set_tag(tag_lp1);
	  }
	else
	  {
	    //LOG_S(INFO) << tag_l << "\t" << ": keep as is ";
	  }
      }
  }
  
  void nlp_model<ENT, CUSTOM_CRF>::identify_rogue_I(subject<TEXT>& subj)
  {
    auto& wtokens = subj.get_word_tokens();
    
    // remove rogue I- (that follow a __undef__)
    for(std::size_t l=0; l+1<wtokens.size(); l++)
      {
	auto& wtoken = wtokens.at(l);

	std::string tag_l = "";
	for(auto& wtoken_tag:wtoken.get_tags())
	  {
	    if(wtoken_tag.starts_with(TAG))
	      {
		tag_l = wtoken_tag;
	      }
	  }

	std::string tag_lp1 = "";
	for(auto& wtoken_tag:wtokens.at(l+1).get_tags())
	  {
	    if(wtoken_tag.starts_with(TAG))
	      {
		tag_lp1 = wtoken_tag;
	      }
	  }
	
	if((is_null_label(tag_l)) and
	   (tag_lp1.starts_with(TAG+"I_")))
	  {
	    //LOG_S(WARNING) << tag_l << "\t" << ": updating to " << tag_lp1;
	    
	    wtokens.at(l+1).remove_tag(tag_lp1);
	    wtokens.at(l+1).set_tag(tag_l);
	  }
	else
	  {
	    //LOG_S(INFO) << tag_l << "\t" << ": keep as is ";
	  }
      }
  }
  
  void nlp_model<ENT, CUSTOM_CRF>::post_process_bio(subject<TEXT>& subj)
  {
    identify_missing_I(subj);

    identify_rogue_I(subj);
    
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
                auto label = wtoken_tag;
		label = utils::replace(label, TAG+"B_", "");
		label = utils::replace(label, TAG+"I_", "");
		label = utils::replace(label, TAG, "");

                auto itr = labels_to_crng.find(label);

                if(itr==labels_to_crng.end())
                  {
                    labels_to_crng[label]={};
                  }

		if(not is_null_label(label))
		  {
		    labels_to_crng.at(label).push_back(wtoken.get_rng());
		  }
	      }
          }
      }
    
    for(auto itr=labels_to_crng.begin(); itr!=labels_to_crng.end(); itr++)
      {
	std::string label = itr->first;
        auto& ranges = itr->second;

	//LOG_S(INFO) << label << ": " << ranges.size();
	
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

	    //LOG_S(INFO) << "inserting " << label << "\t" << name;
	    
            subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
					this->get_name(), label,
					name, orig,
                                        char_range, ctok_range, wtok_range);
          }
      }

  }
  
}

#endif
