//-*-C++-*-

#ifndef ANDROMEDA_MODELS_RECORDS_METADATA_H_
#define ANDROMEDA_MODELS_RECORDS_METADATA_H_

namespace andromeda
{

  template<>
  class nlp_model<REC, METADATA>: public base_crf_model
  {
    typedef typename word_token::range_type range_type;

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return METADATA; }

    // you can only run metadata on full documents
    virtual bool apply(subject<TEXT>& subj) { return false; }
    virtual bool apply(subject<TABLE>& subj) { return false; }

    virtual bool apply(subject<DOCUMENT>& subj);

  private:

    //void initialise()

    bool is_scientific_paper(subject<DOCUMENT>& subj);

    bool find_title(subject<DOCUMENT>& subj);

    bool find_authors(subject<DOCUMENT>& subj);

    bool find_affiliations(subject<DOCUMENT>& subj);

    bool find_abstract(subject<DOCUMENT>& subj);

  private:

    const static std::set<model_name> dependencies;

    int title_ind, abstract_ind, introduction_ind;

    //std::filesystem::path model_file;
  };

  const std::set<model_name> nlp_model<REC, METADATA>::dependencies = {NAME/*, DATE*/, GEOLOC, SEMANTIC};

  nlp_model<REC, METADATA>::nlp_model():
    title_ind(-1),
    abstract_ind(-1),
    introduction_ind(-1)
  {}

  nlp_model<REC, METADATA>::~nlp_model()
  {}

  bool nlp_model<REC, METADATA>::apply(subject<DOCUMENT>& subj)
  {
    if(not is_scientific_paper(subj))
      {
        LOG_S(WARNING) << "document is NOT scientific report!";
        return false;
      }

    LOG_S(WARNING) << "document is scientific report!";

    find_title(subj);

    find_authors(subj);

    find_affiliations(subj);

    find_abstract(subj);

    return update_applied_models(subj);
  }

  bool nlp_model<REC, METADATA>::is_scientific_paper(subject<DOCUMENT>& subj)
  {
    bool detected_abstract=false;
    bool detected_introduction=false;

    //for(auto& tsubj:subj.texts)
    for(int tind=0; tind<subj.texts.size(); tind++)
      {
        auto& tsubj = subj.texts.at(tind);

        auto& provs = tsubj->get_provs();
        if(provs.size()>0 and provs.at(0)->get_page()>=2)
          {
            break;
          }

        std::string text = tsubj->get_text();

        text = utils::replace(text, " ", "");
        text = utils::to_lower(text);

        if(text.starts_with("abstract") or
           text.ends_with("abstract"))
          {
            abstract_ind = tind;
            detected_abstract=true;
          }

        if(text.starts_with("introduction") or
           text.ends_with("introduction"))
          {
            introduction_ind = tind;
            detected_introduction=true;
          }
      }

    return (detected_abstract or detected_introduction);
  }

  bool nlp_model<REC, METADATA>::find_title(subject<DOCUMENT>& subj)
  {
    std::string title="";

    bool first_metadata = true;

    for(int tind=0; tind<abstract_ind; tind++)
      {
        auto& tsubj = subj.texts.at(tind);

        //auto& provs = tsubj->get_provs();

        std::vector<std::string> semlabels = subj.texts.at(tind)->get_property_labels(SEMANTIC);

        //LOG_S(INFO) << "text: " << tind << "\n"
        //std::cout << provs.at(0)->get_type() << std::setw(24)
        //<< provs.at(0)->get_name() << std::setw(24)
        //<< subj.texts.at(tind)->get_property_labels(SEMANTIC).at(0) << std::setw(24)
        //<< subj.texts.at(tind)->get_text() << "\n";
        //std::cout << tabulate(tsubj->get_properties()) << "\n\n";
        //std::cout << tabulate(tsubj->get_instances()) << "\n\n";

        if(tind>0 and
           (std::find(semlabels.begin(), semlabels.end(), "meta-data")!=semlabels.end()) and
           first_metadata)
          {
            first_metadata = false;
            title = tsubj->get_text();

            range_type char_range = {0, title.size()};

            range_type ctok_range = tsubj->get_char_token_range(char_range);
            range_type wtok_range = tsubj->get_word_token_range(char_range);

            tsubj->instances.emplace_back(tsubj->get_hash(), tsubj->get_name(), tsubj->get_self_ref(),
                                          METADATA, "title",
                                          title, title,
                                          char_range, ctok_range, wtok_range);

	    
            range_type rng = {0, 1};
	    /*
	    base_instance inst(subj.get_hash(), DOCUMENT, subj.get_self_ref(),
			       METADATA, "title",
			       title, title,
			       rng, rng, rng);

	    subj.instances.push_back(inst);
	    */
	    
	    subj.instances.emplace_back(subj.get_hash(), DOCUMENT, subj.get_self_ref(),
                                        METADATA, "title",
                                        title, title,
                                        rng, rng, rng);
	    
	    
            title_ind = tind;

            break;
          }
      }

    return true;
  }

  bool nlp_model<REC, METADATA>::find_authors(subject<DOCUMENT>& subj)
  {
    for(int tind=title_ind+1; tind<introduction_ind; tind++)
      {
        auto& tsubj = subj.texts.at(tind);

        auto& insts = tsubj->get_instances();

        for(auto& inst:insts)
          {
            if(inst.is_model(NAME) and inst.is_subtype("person-name"))
              {
                std::string name = inst.get_name();
                std::string orig = inst.get_orig();
		
                range_type rng = {0, 1};
                subj.instances.emplace_back(subj.get_hash(), DOCUMENT, subj.get_self_ref(),
                                            METADATA, "author",
                                            name, orig,
                                            rng, rng, rng);
              }
          }
      }

    return true;
  }

  bool nlp_model<REC, METADATA>::find_affiliations(subject<DOCUMENT>& subj)
  {
    return true;
  }

  bool nlp_model<REC, METADATA>::find_abstract(subject<DOCUMENT>& subj)
  {
    std::string abstract="";
    for(int tind=abstract_ind; tind<introduction_ind; tind++)
      {
        abstract += subj.texts.at(tind-1)->get_text();
      }

    range_type rng = {0, 1};
    subj.instances.emplace_back(subj.get_hash(), DOCUMENT, subj.get_self_ref(),
                                METADATA, "abstract",
                                abstract, abstract,
                                rng, rng, rng);

    return true;
  }



}

#endif
