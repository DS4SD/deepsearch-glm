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
        //LOG_S(WARNING) << "document is NOT scientific report!";
        return false;
      }

    //LOG_S(WARNING) << "document is scientific report!";

    find_title(subj);

    find_authors(subj);

    find_affiliations(subj);

    find_abstract(subj);

    //LOG_S(INFO) << "done with metadata ...";

    return update_applied_models(subj);
  }

  bool nlp_model<REC, METADATA>::is_scientific_paper(subject<DOCUMENT>& subj)
  {
    //LOG_S(INFO);

    bool detected_abstract=false;
    bool detected_introduction=false;

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

    //LOG_S(INFO) << abstract_ind << ", " << introduction_ind << "\n";

    return (detected_abstract or detected_introduction);
  }

  bool nlp_model<REC, METADATA>::find_title(subject<DOCUMENT>& subj)
  {
    //LOG_S(INFO) << __FUNCTION__;

    std::string title="";

    bool first_metadata = true;

    int cut_off=0;
    if(abstract_ind!=-1)
      {
        cut_off = abstract_ind;
      }
    else if(introduction_ind!=-1)
      {
        cut_off = introduction_ind;
      }
    else
      {}

    title_ind = -1;
    for(int tind=0; tind<cut_off; tind++)
      {
        auto& tsubj = subj.texts.at(tind);

        std::vector<std::string> semlabels = subj.texts.at(tind)->get_property_labels(SEMANTIC);

        if(tind>0 and
           (std::find(semlabels.begin(), semlabels.end(), "meta-data")!=semlabels.end()) and
           first_metadata)
          {
            title_ind = tind-1;

            first_metadata = false;
            title = subj.texts.at(tind-1)->get_text();

            range_type char_range = {0, title.size()};

            range_type ctok_range = tsubj->get_char_token_range(char_range);
            range_type wtok_range = tsubj->get_word_token_range(char_range);

            tsubj->instances.emplace_back(tsubj->get_hash(), tsubj->get_name(), tsubj->get_self_ref(),
                                          METADATA, "title",
                                          title, title,
                                          char_range, ctok_range, wtok_range);

            subj.instances.emplace_back(subj.get_hash(), DOCUMENT, tsubj->get_self_ref(),
                                         METADATA, "title",
                                         title, title,
                                         char_range, ctok_range, wtok_range);

            //LOG_S(INFO) << "found title: " << title;

            break;
          }
      }

    return true;
  }

  bool nlp_model<REC, METADATA>::find_authors(subject<DOCUMENT>& subj)
  {
    //LOG_S(INFO) << __FUNCTION__;

    int cut_off=0;
    if(abstract_ind!=-1)
      {
        cut_off = abstract_ind;
      }
    else if(introduction_ind!=-1)
      {
        cut_off = introduction_ind;
      }
    else
      {}

    //LOG_S(INFO) << title_ind << "\t" << cut_off;

    for(int tind=title_ind+1; tind<cut_off; tind++)
      {
        auto& tsubj = subj.texts.at(tind);
        //LOG_S(INFO) << tind << "\t" << tsubj->get_text();
	
        auto& insts = tsubj->get_instances();
	
        for(auto& inst:insts)
          {
            //LOG_S(INFO) << inst.get_subtype() << "\t" << inst.get_name() << "\t";
            if(inst.is_model(NAME) and inst.is_subtype("person-name"))
              {
                std::string name = inst.get_name();
                std::string orig = inst.get_orig();

                //LOG_S(INFO) << " --> author: " << name;
                
                subj.instances.emplace_back(subj.get_hash(), DOCUMENT, tsubj->get_self_ref(), inst.get_conf(),
                                            METADATA, "author",
                                            name, orig,
                                            inst.get_char_range(),
					    inst.get_ctok_range(),
					    inst.get_wtok_range());
              }
          }
      }

    return true;
  }

  bool nlp_model<REC, METADATA>::find_affiliations(subject<DOCUMENT>& subj)
  {
    //LOG_S(INFO) << __FUNCTION__;

    return true;
  }

  bool nlp_model<REC, METADATA>::find_abstract(subject<DOCUMENT>& subj)
  {
    //LOG_S(INFO) << __FUNCTION__;

    std::string abstract="";
    if(abstract_ind!=-1 and introduction_ind==-1)
      {
        abstract = subj.texts.at(abstract_ind)->get_text();
      }
    else if(abstract_ind!=-1 and introduction_ind!=-1)
      {
        for(int tind=abstract_ind; tind<introduction_ind; tind++)
          {
	    std::string text = subj.texts.at(tind)->get_text();
	    
	    text = utils::replace(text, " ", "");
	    text = utils::to_lower(text);
	
	    if(not text.ends_with("abstract"))
	      {
		abstract += subj.texts.at(tind)->get_text();
		abstract += " ";
	      }
	  }
      }
    else
      {}

    if(abstract.size()>0)
      {
        range_type rng = {0, abstract.size()};
        subj.instances.emplace_back(subj.get_hash(), DOCUMENT, subj.get_self_ref(),
                                    METADATA, "abstract",
                                    abstract, abstract,
                                    rng, rng, rng);
      }
    
    return true;
  }

}

#endif
