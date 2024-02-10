//-*-C++-*-

#ifndef ANDROMEDA_MODELS_TOKENIZERS_SPM_H
#define ANDROMEDA_MODELS_TOKENIZERS_SPM_H

namespace andromeda
{

  template<>
  class nlp_model<TOK, SPM>: public base_tok_model
  {
    typedef typename word_token::range_type range_type;

    const static inline std::string TAG = "__"+to_string(SPM)+"__";
    
  public:

    nlp_model();
    
    //~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return TOK; }
    virtual model_name get_name() { return SPM; }

    virtual bool apply(subject<TEXT>& subj);

  private:

    const static inline std::set<model_name> dependencies = {};
    
    std::filesystem::path model_file;
  };

  nlp_model<TOK, SPM>::nlp_model():
    base_tok_model(),
    model_file(get_resources_dir() / "models/tok/default-tokenizer.model")
  {
    this->load(model_file, false);
  }

  bool nlp_model<TOK, SPM>::apply(subject<TEXT>& subj)
  {
    LOG_S(WARNING) << "bool nlp_model<TOK, SPM>::apply(subject<TEXT>& subj)";
    
    auto text = subj.get_text();
    auto& wtokens = subj.get_word_tokens();

    std::string tmp="";
    for(index_type i=0; i<wtokens.size(); i++)
      {
	if(i==0)
	  {
	    auto crng = wtokens.at(i).get_rng();
	    tmp = text.substr(crng[0], crng[1]-crng[0]);
	  }
	else
	  {
	    auto crng_0 = wtokens.at(i-1).get_rng();
	    auto crng_1 = wtokens.at(i-0).get_rng();
	    
	    tmp = text.substr(crng_0[1], crng_1[1]-crng_0[1]);
	  }

	auto inds = this->encode(tmp);
	LOG_S(INFO) << tmp << " => " << utils::to_string(inds);
	
	wtokens.at(i).set_inds(inds);
      }

    return true;
  }
  
}

#endif
