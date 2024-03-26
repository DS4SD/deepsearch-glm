//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_REFERENCE_H_
#define ANDROMEDA_MODELS_ENTITIES_REFERENCE_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, REFERENCE>: public base_crf_model
  {
    typedef typename word_token::range_type range_type;

    const static inline std::string TAG = "__"+to_string(REFERENCE)+"__";

    const static inline std::set<std::string> LABELS = { "reference-number",
                                                         "authors", "title",
                                                         "publisher",
                                                         "journal", "conference",
                                                         "date",
                                                         "volume", "pages",
                                                         "url", "doi", "isbn",
							 "note"};

  public:

    nlp_model();
    //nlp_model(std::filesystem::path resources_dir);

    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return REFERENCE; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply(subject<TABLE>& subj) { return false; }
    virtual bool apply(subject<DOCUMENT>& subj);

  private:

    //void initialise(std::filesystem::path resources_dir);
    bool initialise();

    void run_model(subject<TEXT>& subj);

    void post_process(subject<TEXT>& subj);

    std::string normalise_name(std::string orig);
    void normalise_subject(subject<TEXT>& subj);

  private:

    const static std::set<model_name> dependencies;

    std::filesystem::path model_file;
  };

  const std::set<model_name> nlp_model<ENT, REFERENCE>::dependencies = { LINK, NUMVAL, SEMANTIC };

  nlp_model<ENT, REFERENCE>::nlp_model():
    model_file(get_crf_dir() / "reference/crf_reference.bin")
  {
    initialise();
  }

  nlp_model<ENT, REFERENCE>::~nlp_model()
  {}

  bool nlp_model<ENT, REFERENCE>::initialise()
  {
    if(not base_crf_model::load(model_file, false))
      {
        LOG_S(ERROR) << "could not load REFERENCE model from " << model_file;
        return false;
      }

    return true;
  }

  bool nlp_model<ENT, REFERENCE>::apply(subject<DOCUMENT>& doc)
  {
    if(not satisfies_dependencies(doc))
      {
        return false;
      }

    //LOG_S(INFO) << "#-texts: " << doc.texts.size();
    for(auto& paragraph:doc.texts)
      {
        this->apply(*paragraph);
      }

    return update_applied_models(doc);
  }

  bool nlp_model<ENT, REFERENCE>::apply(subject<TEXT>& subj)
  {
    //LOG_S(INFO) << __FILE__ << ":" << __LINE__ << "\t" << subj.get_text();

    if(not satisfies_dependencies(subj))
      {
        //LOG_S(WARNING) << "does not satisfy deps ... ";
        return false;
      }

    bool is_ref=false;
    for(auto& cls:subj.properties)
      {
        if((cls.get_type()==to_key(SEMANTIC)) and (cls.is_label("reference")))
          {
            is_ref = true;
            //LOG_S(WARNING) << " => " << cls.get_type() << "\t" << cls.get_label();
          }
        else
          {
            //LOG_S(INFO) << " => " << cls.get_type() << "\t" << cls.get_label();
          }
      }

    // text in subject is not a reference and we do not apply the reference parser
    if(not is_ref)
      {
        return true;
      }

    run_model(subj);

    post_process(subj);

    return update_applied_models(subj);
  }

  void nlp_model<ENT, REFERENCE>::run_model(subject<TEXT>& subj)
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

    /*
      for(std::size_t l=0; l<crf_tokens.size(); l++)
      {
      crf_tokens.at(l).true_label = crf_tokens.at(l).pred_label;
      }

      std::set<std::string> texts={".",",","and"};
      for(std::size_t l=1; l<crf_tokens.size()-1; l++)
      {
      if((crf_tokens.at(l-1).true_label==crf_tokens.at(l+1).true_label) and
      texts.count(crf_tokens.at(l).text))
      {
      crf_tokens.at(l).true_label = crf_tokens.at(l-1).true_label;
      }
      }
    */

    for(std::size_t l=0; l<wtokens.size(); l++)
      {
        auto& wtoken = wtokens.at(l);
        auto& ptoken = crf_tokens.at(l);

        std::string label = TAG + ptoken.pred_label;
        wtoken.set_tag(label);
      }
  }

  void nlp_model<ENT, REFERENCE>::post_process(subject<TEXT>& subj)
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

    /*
      for(auto itr=labels_to_crng.begin(); itr!=labels_to_crng.end(); itr++)
      {
      LOG_S(INFO) << itr->first << ": " << (itr->second).size();
      for(auto jtr=(itr->second).begin(); jtr!=(itr->second).end(); jtr++)
      {
      LOG_S(INFO) << " -> " << (*jtr)[0] << ", " << (*jtr)[1];
      }
      }
    */

    /*
      std::set<std::string> labels
      = { "citation-number",
      "author", "title",
      //"publisher", "editor",
      "journal", "container-title",
      "location", "date",
      //"volume", "pages",
      "url", "doi", "isbn"};
    */

    for(const auto& label:LABELS)
      {
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
            //std::string name = subj.from_ctok_range(ctok_range);
            std::string name = normalise_name(orig);

	    if(label=="doi" or label=="url")
	      {
		name = utils::replace(name, " ", "");
	      }
	    
            subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                        REFERENCE, label,
                                        name, orig,
                                        char_range, ctok_range, wtok_range);
          }
      }

    normalise_subject(subj);
  }

  std::string nlp_model<ENT, REFERENCE>::normalise_name(std::string orig)
  {
    const static std::vector<std::string> endings
      = {")", "]", ".", ",", " "};
    const static std::vector<std::string> startings
      = {"(", "[",
	 "doi:", "DOI:", "isbn:", "ISBN:",
	 "arXiv preprint",
	 " "};
    
    std::string name = orig;

    bool updating=true;
    while(updating)
      {
        updating=false;
        for(auto end:endings)
          {
            if(name.ends_with(end))
              {
                name = name.substr(0, name.size()-end.size());
                updating=true;
              }
          }
      }

    updating = true;
    while(updating)
      {
        updating=false;
        for(auto strt:startings)
          {
            if(name.starts_with(strt))
              {
                name = name.substr(strt.size(), name.size()-strt.size());
                updating=true;
              }
          }
      }

    return name;
  }

  void nlp_model<ENT, REFERENCE>::normalise_subject(subject<TEXT>& subj)
  {
    auto itr=subj.instances.begin();
    while(itr!=subj.instances.end())
      {
        if(not (itr->is_model(REFERENCE)))
          {
            itr = subj.instances.erase(itr);
          }
        else
          {
            itr++;
          }
      }
  }

}

#endif
