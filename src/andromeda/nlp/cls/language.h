//-*-C++-*-

#ifndef ANDROMEDA_MODELS_CLASSIFIERS_LANGUAGE_H_
#define ANDROMEDA_MODELS_CLASSIFIERS_LANGUAGE_H_

namespace andromeda
{

  template<>
  class nlp_model<CLS, LANGUAGE>: public fasttext_supervised_model  
  {
    const static model_type type = CLS;
    const static model_name name = LANGUAGE;
    
  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return CLS; }
    virtual model_name get_name() { return LANGUAGE; }
    
    /*  PREDICT  */
    
    virtual bool preprocess(const subject<TEXT>& subj, std::string& text);
    virtual bool preprocess(const subject<TABLE>& subj, std::string& text);
    
    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply(subject<TABLE>& subj);

    virtual bool apply(subject<DOCUMENT>& subj);
    
  private:

    void initialise();
    
  private:

    const static inline std::set<model_name> dependencies = {};

    std::filesystem::path model_file;
  };

  nlp_model<CLS, LANGUAGE>::nlp_model():
    fasttext_supervised_model(),
    model_file(get_fst_dir() / "language/fst_language.bin")
  {
    initialise();
  }

  nlp_model<CLS, LANGUAGE>::~nlp_model()
  {}

  void nlp_model<CLS, LANGUAGE>::initialise()
  {
    if(not fasttext_supervised_model::load(model_file))
      {
	LOG_S(ERROR) << "could not load `language` classifier model ...";
      }
  }
  
  bool nlp_model<CLS, LANGUAGE>::preprocess(const subject<TEXT>& subj, std::string& text)
  {
    text = subj.get_text();
    return true;
  }
  
  bool nlp_model<CLS, LANGUAGE>::preprocess(const subject<TABLE>& subj, std::string& text)
  {
    std::stringstream ss;
    for(std::size_t i=0; i<subj.data.size(); i++)
      {
	auto& row = subj.data.at(i);	
	for(std::size_t j=0; j<row.size(); j++)
	  {
	    ss << row.at(j).get_text() << "; ";
	  }
      }

    text = ss.str();
    return true;
  }
    
  bool nlp_model<CLS, LANGUAGE>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    bool classified = (subj.applied_models.count(get_key())>0);
    
    if(not classified)
      {
	classified = fasttext_supervised_model::classify(subj);	
      }
    
    return classified;    
  }

  bool nlp_model<CLS, LANGUAGE>::apply(subject<TABLE>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }
    
    bool classified = (subj.applied_models.count(get_key())>0);
    
    if(not classified)
      {
	classified = fasttext_supervised_model::classify(subj);	
      }
    
    return classified;        
  }
  
  /*
  bool nlp_model<CLS, LANGUAGE>::apply(subject<DOCUMENT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }  

    for(auto& para:subj.texts)
      {
	this->apply(*para);
      }

    for(auto& table:subj.tables)
      {
	this->apply(*table);
      }

    std::map<std::string, std::size_t> lang_mapping;

    std::size_t total=0;
    for(auto& para:subj.texts)
      {
	this->apply(*para);

	base_property prop(para->get_hash(), TEXT, para->get_sref(),
			   "null", "null", 0.0);
	
	if(get(*para, prop))
	  {
	    std::string key = prop.get_label();
	    std::size_t dst = para->dst;

	    if(lang_mapping.count(key)==1)
	      {
		lang_mapping[key] += dst;
		total += dst;
	      }
	    else
	      {
		lang_mapping[key] = dst;
		total += dst;
	      }
	  }
      }

    base_property prop(subj.get_hash(), DOCUMENT, "#",
		       this->get_key(), "null", 0.0);
    for(auto itr=lang_mapping.begin(); itr!=lang_mapping.end(); itr++)
      {
	double confidence = std::round(1000*(itr->second)/(0.0+total))/1000.0;
	
	if(itr==lang_mapping.begin())
	  {
	    prop.set_label(itr->first);
	    prop.set_conf(confidence);
	  }
	else if(prop.get_conf()<confidence)
	  {
	    prop.set_label(itr->first);
	    prop.set_conf(confidence);	    
	  }
	else
	  {}
      }

    subj.properties.push_back(prop);
    
    return update_applied_models(subj);
  }
  */

  bool nlp_model<CLS, LANGUAGE>::apply(subject<DOCUMENT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }
    
    std::string text="", label="null";
    double conf=0.0;

    std::map<std::string, std::size_t> lang_mapping;
    std::size_t total=0;
    
    for(uint64_t ind=0; ind<subj.texts.size(); ind++)
      {
	auto& para = subj.texts.at(ind);
	
	if(not preprocess(*para, text))
	  {
	    continue; // skip
	  }
	
	if(not classify(text, label, conf))
	  {
	    continue; // skip
	  }

	{
	  if(lang_mapping.count(label)==1)
	    {
	      lang_mapping[label] += para->get_len();
	      total += para->get_len();
	    }
	  else
	    {
	      lang_mapping[label] = para->get_len();
	      total += para->get_len();
	    }
	}
	
	para->properties.emplace_back(para->get_hash(), TEXT, para->get_self_ref(),
				      get_name(), label, conf);
	para->applied_models.insert(get_key());
      }

    base_property prop(subj.get_hash(), DOCUMENT, "#",
		       get_name(), "null", 0.0);
    for(auto itr=lang_mapping.begin(); itr!=lang_mapping.end(); itr++)
      {
	double confidence = std::round(1000*(itr->second)/(0.0+total))/1000.0;
	
	if(itr==lang_mapping.begin())
	  {
	    prop.set_label(itr->first);
	    prop.set_conf(confidence);
	  }
	else if(prop.get_conf()<confidence)
	  {
	    prop.set_label(itr->first);
	    prop.set_conf(confidence);	    
	  }
	else
	  {}
      }
    subj.properties.push_back(prop);
    
    return update_applied_models(subj);    
  }
  
}

#endif
