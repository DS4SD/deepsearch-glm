//-*-C++-*-

#ifndef ANDROMEDA_MODELS_TMPL_MODEL_H_
#define ANDROMEDA_MODELS_TMPL_MODEL_H_

namespace andromeda
{
  template<model_type type, model_name name>
  class nlp_model: public base_nlp_model
  {
  public:

    nlp_model() {}
    ~nlp_model() {}

    virtual model_type get_type() { return type; }
    virtual model_name get_name() { return name; }
  };

}

#endif
