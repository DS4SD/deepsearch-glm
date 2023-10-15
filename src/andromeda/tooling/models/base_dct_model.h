//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_DCT_MODEL_H_
#define ANDROMEDA_MODELS_BASE_DCT_MODEL_H_

namespace andromeda
{
  class base_dct_model: public base_nlp_model
  {
    typedef std::pair<std::string, std::string> key_type; // <label, sublabel>
    typedef std::vector<pcre2_expr>             val_type;
    
  public:

    base_dct_model() {};
    virtual ~base_dct_model() {};

    /*   IO   */    
    virtual bool load(std::filesystem::path ifile, bool verbose);
    virtual bool save(std::filesystem::path ofile);

    /*   INFERENCE   */
    
    virtual bool apply(std::string& text, nlohmann::json& annots);// { return false; }

    virtual bool apply(subject<TEXT>& subj);// = 0;// { return false; }
    virtual bool apply(subject<TABLE>& subj);// { return false; }

    virtual bool apply(subject<DOCUMENT>& subj);
    
    /*   TRAIN   */
    virtual bool is_trainable() { return false; }
    virtual bool train(nlohmann::json args);

  private:

    nlohmann::json config;
    
    std::map<key_type, val_type> exprs;
  };

  bool base_dct_model::apply(std::string& text, nlohmann::json& annots)
  {
    return false;
  }

  bool base_dct_model::apply(subject<TEXT>& subj)
  {
    return false;
  }
  
  bool base_dct_model::apply(subject<TABLE>& subj)
  {
    return false;
  }

  bool base_dct_model::apply(subject<DOCUMENT>& subj)
  {
    return false;
  }
  
  bool base_dct_model::train(nlohmann::json args)
  {
    LOG_S(INFO) << "starting to train regex-model ...";
    
    return false;
  }
  

}

#endif
