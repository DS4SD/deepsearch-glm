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
    //nlp_model(std::filesystem::path resources_dir);
    
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return CLS; }
    virtual model_name get_name() { return LANGUAGE; }
    
    /*  PREDICT  */
    
    virtual bool preprocess(const subject<PARAGRAPH>& subj, std::string& text);
    virtual bool preprocess(const subject<TABLE>& subj, std::string& text);
    
    virtual bool apply(subject<PARAGRAPH>& subj);
    virtual bool apply(subject<TABLE>& subj);

    virtual bool apply(subject<DOCUMENT>& subj);
    
  private:

    //void initialise(std::filesystem::path resources_dir);
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

  /*
  nlp_model<CLS, LANGUAGE>::nlp_model(std::filesystem::path resources_dir):
    fasttext_supervised_model()
  {
    initialise(resources_dir);
  }
  */
  
  nlp_model<CLS, LANGUAGE>::~nlp_model()
  {}

  /*
  void nlp_model<CLS, LANGUAGE>::initialise(std::filesystem::path resources_dir)
  {
    //std::filesystem::path ifile = resources_dir / "models/fasttext/language/lid.176.bin";
    //std::filesystem::path ifile = glm_variables::get_resources_dir() / "models/fasttext/language/lid.176.bin";

    fasttext_supervised_model::load(model_file);
  }
  */

  void nlp_model<CLS, LANGUAGE>::initialise()
  {
    if(not fasttext_supervised_model::load(model_file))
      {
	LOG_S(FATAL) << "could not load `language` classifier model ...";
      }
  }
  
  bool nlp_model<CLS, LANGUAGE>::preprocess(const subject<PARAGRAPH>& subj, std::string& text)
  {
    text = subj.text;
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
	    ss << row.at(j).text << "; ";
	  }
      }

    text = ss.str();
    return true;
  }
    
  bool nlp_model<CLS, LANGUAGE>::apply(subject<PARAGRAPH>& subj)
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
  
  bool nlp_model<CLS, LANGUAGE>::apply(subject<DOCUMENT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }  

    for(auto& para:subj.paragraphs)
      {
	this->apply(*para);
      }

    for(auto& table:subj.tables)
      {
	this->apply(*table);
      }

    std::map<std::string, std::size_t> lang_mapping;

    std::size_t total=0;
    for(auto& para:subj.paragraphs)
      {
	this->apply(*para);

	base_property prop("null", "null", 0.0);
	if(get(*para, prop))
	  {
	    std::string key = prop.get_name();
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

    base_property prop(this->get_key(), "null", 0.0);
    for(auto itr=lang_mapping.begin(); itr!=lang_mapping.end(); itr++)
      {
	double confidence = std::round(1000*(itr->second)/(0.0+total))/1000.0;
	
	if(itr==lang_mapping.begin())
	  {
	    prop.set_name(itr->first);
	    prop.set_conf(confidence);
	  }
	else if(prop.get_conf()<confidence)
	  {
	    prop.set_name(itr->first);
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
