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
    
  private:

    const static std::set<model_name> dependencies;

    //std::filesystem::path model_file;
  };

  const std::set<model_name> nlp_model<REC, METADATA>::dependencies = {SEMANTIC};

  nlp_model<REC, METADATA>::nlp_model()
  {
      //initialise();
  }

  nlp_model<REC, METADATA>::~nlp_model()
  {}

  bool nlp_model<REC, METADATA>::apply(subject<DOCUMENT>& subj)
  {
    
    return update_applied_models(subj);
  }
  
}

#endif

