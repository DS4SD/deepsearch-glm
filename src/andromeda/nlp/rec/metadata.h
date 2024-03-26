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
    
  private:

    const static std::set<model_name> dependencies;

    int abstract_text_ind, intro_text_ind;
    
    //std::filesystem::path model_file;
  };

  const std::set<model_name> nlp_model<REC, METADATA>::dependencies = {SEMANTIC};

  nlp_model<REC, METADATA>::nlp_model():
    abstract_text_ind(-1),
    intro_text_ind(-1)
  {}

  nlp_model<REC, METADATA>::~nlp_model()
  {}

  bool nlp_model<REC, METADATA>::apply(subject<DOCUMENT>& subj)
  {
    if(not is_scientific_paper(subj))
      {
	return false;
      }

    LOG_S(WARNING) << "document is scientific report!";

    for(int tind=0; tind<abstract_text_ind; tind++)
      {
	LOG_S(INFO) << "text: " << tind << "\n"
		    << subj.texts.at(tind)->to_json({}).dump(2);
      }
    
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
	    abstract_text_ind = tind;
	    detected_abstract=true;
	  }

	if(text.starts_with("introduction") or
	   text.ends_with("introduction"))
	  {
	    intro_text_ind = tind;
	    detected_introduction=true;
	  }
      }

    return (detected_abstract and detected_introduction);
  }
  
}

#endif

