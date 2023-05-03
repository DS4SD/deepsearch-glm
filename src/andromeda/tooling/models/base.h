//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_MODEL_H_
#define ANDROMEDA_MODELS_BASE_MODEL_H_

namespace andromeda
{
  class base_nlp_model
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

    /*   INFERENCE   */
    
    template<typename subject_type>
    bool satisfies_dependencies(subject_type& subj);

    template<typename subject_type>
    bool update_applied_models(subject_type& subj);

    virtual bool match(std::string& text, nlohmann::json& annot) { return false; }
    virtual bool apply(std::string& text, nlohmann::json& annots) { return false; }

    virtual bool apply(subject<PARAGRAPH>& subj) = 0;// { return false; }
    virtual bool apply(subject<TABLE>& subj) { return false; }

    virtual bool apply(subject<DOCUMENT>& subj);

    static bool finalise(subject<DOCUMENT>& subj) { return false; }

    /*   TRAIN   */

    virtual bool is_trainable() { return false; }

    virtual nlohmann::json create_train_config() { return nlohmann::json::object({}); }
    
    virtual bool train(nlohmann::json args) { return false; }
  };

  template<typename subject_type>
  bool base_nlp_model::satisfies_dependencies(subject_type& subj)
  {
    bool result=true;
    for(auto dep:get_dependencies())
      {
        if(subj.applied_models.count(to_key(dep))==0)
          {
            //LOG_S(WARNING) << "model-dependency " << to_key(dep)
            //<< " is not satisfied for " << this->get_key();
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

  bool base_nlp_model::apply(subject<DOCUMENT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    for(subject<PARAGRAPH>& paragraph:subj.paragraphs)
      {
        this->apply(paragraph);
      }

    for(subject<TABLE>& table:subj.tables)
      {
        this->apply(table);
      }

    return update_applied_models(subj);
  }  

}

#endif
