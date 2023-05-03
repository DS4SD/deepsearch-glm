//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_RGX_MODEL_H_
#define ANDROMEDA_MODELS_BASE_RGX_MODEL_H_

namespace andromeda
{
  class base_rgx_model: public base_nlp_model
  {
    typedef std::pair<std::string, std::string> key_type; // <label, sublabel>
    typedef std::vector<pcre2_expr>             val_type;
    
  public:

    base_rgx_model() {};
    virtual ~base_rgx_model() {};

    /*   IO   */    
    virtual bool load(std::filesystem::path ifile, bool verbose);
    virtual bool save(std::filesystem::path ofile);

    /*   INFERENCE   */
    
    virtual bool apply(std::string& text, nlohmann::json& annots);// { return false; }

    virtual bool apply(subject<PARAGRAPH>& subj);// = 0;// { return false; }
    virtual bool apply(subject<TABLE>& subj);// { return false; }

    virtual bool apply(subject<DOCUMENT>& subj);
    
    /*   TRAIN   */
    virtual bool is_trainable() { return true; }
    virtual bool train(nlohmann::json args);

  private:

    nlohmann::json config;
    
    std::map<key_type, val_type> exprs;
  };

  bool base_rgx_model::apply(std::string& text, nlohmann::json& annots)
  {
    return false;
  }

  bool base_rgx_model::apply(subject<PARAGRAPH>& subj)
  {
    return false;
  }
  
  bool base_rgx_model::apply(subject<TABLE>& subj)
  {
    return false;
  }

  bool base_rgx_model::apply(subject<DOCUMENT>& subj)
  {
    return false;
  }
  
  bool base_rgx_model::train(nlohmann::json args)
  {
    LOG_S(INFO) << "starting to train regex-model ...";
    
    return false;
  }
  

}

#endif
