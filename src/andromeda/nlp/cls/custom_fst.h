//-*-C++-*-

#ifndef ANDROMEDA_MODELS_CLASSIFIERS_CUSTOM_FST_H_
#define ANDROMEDA_MODELS_CLASSIFIERS_CUSTOM_FST_H_

namespace andromeda
{
  /*
    The goal of this class is to find ...
  */
  template<>
  class nlp_model<CLS, CUSTOM_FST>: public fasttext_supervised_model
  {
    const static model_type type = CLS;
    const static model_name name = CUSTOM_FST;

  public:

    nlp_model();
    
    nlp_model(std::string desc);

    nlp_model(std::string name, std::string file,
	      std::filesystem::path model_file);
    
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return CLS; }
    virtual model_name get_name() { return CUSTOM_FST; }

    virtual std::string get_key() {
      std::stringstream ss;
      ss << to_key(this->get_name()) << "(" << custom_name << ":" << custom_file << ")";
      
      return ss.str();
    }
    
    template<typename subject_type>
    bool get(subject_type& subj, base_property& prop);

    virtual bool preprocess(const subject<TEXT>& subj, std::string& text);
    //virtual bool preprocess(const subject<TABLE>& subj, std::string& text) { return false; }

    virtual bool apply(subject<TEXT>& subj);
    //virtual bool apply(subject<TABLE>& subj) { return false; }
    //virtual bool apply(subject<DOCUMENT>& subj) { return false; }

  private:

    bool initialise();

    void run_model(subject<TEXT>& subj);

  private:

    const static std::set<model_name> dependencies;

    std::string custom_name, custom_file;
    
    std::filesystem::path model_file;
  };

  const std::set<model_name> nlp_model<CLS, CUSTOM_FST>::dependencies = {};

  nlp_model<CLS, CUSTOM_FST>::nlp_model()
  {}

  nlp_model<CLS, CUSTOM_FST>::nlp_model(std::string desc):
    model_file()
  {
    pcre2_expr expr("custom-fst", "", R"(custom_fst\((?P<name>([a-zA-Z\-]+))\:(?P<file>(.+))\))");
    
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
	LOG_S(WARNING) << "could not initialise custom-fst with desc: " << desc;
      }
  }

  nlp_model<CLS, CUSTOM_FST>::nlp_model(std::string name,
					std::string file,
					std::filesystem::path model_file):
    custom_name(name),
    custom_file(file),
    model_file(model_file)
  {
    initialise();
  }
  
  nlp_model<CLS, CUSTOM_FST>::~nlp_model()
  {}
  
  bool nlp_model<CLS, CUSTOM_FST>::initialise()
  {
    if(not fasttext_supervised_model::load(model_file))
      {
        LOG_S(ERROR) << "could not load CUSTOM_FST model from " << model_file;
        return false;
      }

    return true;
  }

  bool nlp_model<CLS, CUSTOM_FST>::preprocess(const subject<TEXT>& subj, std::string& text)
  {
    text = subj.get_text();
    return (text.size()>0);
  }
  
  bool nlp_model<CLS, CUSTOM_FST>::apply(subject<TEXT>& subj)
  {
    std::string text="", label="null";
    double conf=0.0;

    if(not preprocess(subj, text))
    {
      return false; //continue; // skip continue; // skip
    }

    if(not classify(text, label, conf))
      {
        return false; //continue; // skip
      }

    //LOG_S(INFO) << label << ", " << conf << ": " << text.substr(0, 64);
    
    subj.properties.emplace_back(subj.get_hash(), TEXT, subj.get_self_ref(), 
                                 get_name(), label, conf);
    subj.applied_models.insert(get_key());
    
    return true;
  }
  
}

#endif
