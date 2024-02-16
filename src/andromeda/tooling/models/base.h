//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_MODEL_H_
#define ANDROMEDA_MODELS_BASE_MODEL_H_

namespace andromeda
{
  class base_nlp_model: public glm_variables
  {
  public:
    
    typedef typename word_token::index_type index_type;
    typedef typename word_token::range_type range_type;
    
  public:

    base_nlp_model() {};
    virtual ~base_nlp_model() {};

    virtual std::set<model_name> get_dependencies() = 0;

    virtual model_type get_type() = 0;
    virtual model_name get_name() = 0;

    virtual std::string get_key() { return to_key(this->get_name()); }

    /* INFERENCE */

    template<typename subject_type>
    bool is_applied(subject_type& subj);
    
    template<typename subject_type>
    bool satisfies_dependencies(subject_type& subj);

    template<typename subject_type>
    static bool satisfies_dependencies(subject_type& subj, const std::set<model_name>& deps);

    template<typename subject_type>
    bool update_applied_models(subject_type& subj);

    virtual bool match(std::string& text, nlohmann::json& annot) { return false; }
    virtual bool apply(std::string& text, nlohmann::json& annots) { return false; }

    virtual bool apply(subject<TEXT>& subj) = 0;// { return false; }
    //virtual bool apply(subject<TABLE>& subj) = 0;//{ return false; }

    virtual bool apply(subject<TABLE>& subj); //{ return false; }
    virtual bool apply_on_table_data(subject<TABLE>& subj) { return false; }
    
    virtual bool apply(subject<FIGURE>& subj);
    virtual bool apply_on_figure_data(subject<FIGURE>& subj) { return false; }

    virtual bool apply(subject<DOCUMENT>& subj);

    static bool finalise(subject<DOCUMENT>& subj) { return false; }

    /* TRAIN */

    virtual bool is_trainable() { return false; }

    virtual nlohmann::json create_train_config() { return nlohmann::json::object({}); }

    virtual bool prepare_data_for_train(nlohmann::json args,
					std::vector<std::shared_ptr<base_nlp_model> >& dep_models) { return false; }

    virtual bool train(nlohmann::json args) { return false; }

    virtual bool evaluate_model(nlohmann::json args,
				std::vector<std::shared_ptr<base_nlp_model> >& dep_models) { return false; }
  };

  template<typename subject_type>
  bool base_nlp_model::is_applied(subject_type& subj)
  {
    return (subj.applied_models.count(this->get_key())==1);
  }
  
  template<typename subject_type>
  bool base_nlp_model::satisfies_dependencies(subject_type& subj)
  {
    //if(subj.applied_models.count(this->get_key()))
    //{
	//LOG_S(WARNING) << "already applied " << this->get_key() << " ...";
    //return false; // already done ...
    //}

    if(is_applied(subj)) // already done ...
      {
	return false;
      }
    
    return satisfies_dependencies(subj, get_dependencies());
  }
  
  template<typename subject_type>
  bool base_nlp_model::satisfies_dependencies(subject_type& subj, const std::set<model_name>& deps)
  {
    bool result=true;
    for(auto dep:deps)
      {
        if(subj.applied_models.count(to_key(dep))==0)
          {
            result = false;
          }
      }

    return result;
  }
  
  template<typename subject_type>
  bool base_nlp_model::update_applied_models(subject_type& subj)
  {
    subj.applied_models.insert(this->get_key());
    return true;
  }

  bool base_nlp_model::apply(subject<TABLE>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }
    
    for(auto& caption:subj.captions)
      {
        this->apply(*caption);
      }

    this->apply_on_table_data(subj);

    return update_applied_models(subj);
  }
  
  bool base_nlp_model::apply(subject<FIGURE>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    for(auto& caption:subj.captions)
      {
        this->apply(*caption);
      }

    this->apply_on_figure_data(subj);

    return update_applied_models(subj);
  }
  
  bool base_nlp_model::apply(subject<DOCUMENT>& subj)
  {
    //LOG_S(INFO) << "applying " << to_string(get_name());
    
    if(not satisfies_dependencies(subj))
      {
	//LOG_S(INFO) << " -> not satisfied ...";
        return false;
      }

    //LOG_S(INFO) << " -> satisfied ...";
    
    for(auto& text_ptr:subj.texts)
      {
        this->apply(*text_ptr);
      }

    for(auto& table_ptr:subj.tables)
      {
        this->apply(*table_ptr);
      }

    for(auto& figure_ptr:subj.figures)
      {
        this->apply(*figure_ptr);
      }
    
    return update_applied_models(subj);
  }  

}

#endif
