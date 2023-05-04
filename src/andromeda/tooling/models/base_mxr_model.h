//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_MXR_MODEL_H_
#define ANDROMEDA_MODELS_BASE_MXR_MODEL_H_

namespace andromeda
{
  class base_mxr_model: public base_nlp_model
  {
  public:

    base_mxr_model() {};
    virtual ~base_mxr_model() {};

    /*   TRAIN   */
    virtual bool is_trainable() { return true; }
    virtual bool train(nlohmann::json args);

  private:
    
  };

  bool base_mxr_model::train(nlohmann::json args)
  {
    LOG_S(INFO) << "starting to train NLP-mixer ...";
    
    return false;
  }
  
}

#endif
