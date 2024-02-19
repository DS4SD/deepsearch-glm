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

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return TOK; }
    virtual model_name get_name() { return SPM; }

    virtual bool apply(subject<TEXT>& subj);

  private:

    const static inline std::set<model_name> dependencies = {};
    
    std::filesystem::path model_file;

    int SPACE_IND;
    std::map<int, int> ind_without_space;
  };

  nlp_model<TOK, SPM>::nlp_model():
    base_tok_model(),
    model_file(get_resources_dir() / "models/tok/default-tokenizer.model"),

    SPACE_IND(-1),
    ind_without_space({})
  {
    this->load(model_file, false);
    //model->SetEncodeExtraOptions("bos:eos");
    
    int _ind, ind;
    std::string _tok, tok;
    for(_ind=0; _ind<get_num_tokens(); _ind++)
      {
	_tok = to_token(_ind);

	if(_tok=="▁")
	  {
	    tok = "";
	    ind = -1;
	    
	    SPACE_IND = _ind;
	  }
	else if(_tok.starts_with("▁"))
	  {
	    tok = utils::replace(_tok, "▁", "");
	    ind = to_ind(tok);

	    if(ind==0)
	      {
		ind = -1;
	      }
	    else
	      {
		//LOG_S(INFO) << _ind << "\t" << ind << "\t" << _tok << "\t" << to_token(ind);
	      }
	  }
	else
	  {
	    tok = "";
	    ind = -1;
	  }
	
	ind_without_space[_ind] = ind;
      }
  }

  bool nlp_model<TOK, SPM>::apply(subject<TEXT>& subj)
  {
    //LOG_S(WARNING) << "bool nlp_model<TOK, SPM>::apply(subject<TEXT>& subj)";
    
    auto text = subj.get_text();
    auto& wtokens = subj.get_word_tokens();

    std::vector<int> tinds={};
        
    std::string tmp="";
    for(index_type i=0; i<wtokens.size(); i++)
      {
	/*
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
	*/

	auto crng = wtokens.at(i).get_rng();
	tmp = text.substr(crng[0], crng[1]-crng[0]);
	
	//tmp = utils::replace(tmp, " ", "▁");
	
	auto inds = this->encode(tmp);

	//LOG_S(INFO) << tmp << " => " << utils::to_string(inds);
	
	// no preceeding space
	if(i>0 and wtokens.at(i-1).get_rng().at(1)==wtokens.at(i-0).get_rng().at(0)) 
	  {
	    int ind_0 = ind_without_space.at(inds.at(0));
	    
	    if(SPACE_IND!=-1 and inds.at(0)==SPACE_IND and inds.size()>1)
	      {
		inds.erase(inds.begin());
	      }
	    else if(ind_0!=-1)
	      {
		inds.at(0) = ind_0;
	      }
	    else
	      {
		
	      }
	  }
	
	//LOG_S(INFO) << tmp << " => " << utils::to_string(inds);
	
	wtokens.at(i).set_inds(inds);

	std::vector<std::string> subws={};
	for(auto ind:inds)
	  {
	    subws.push_back(this->to_token(ind));
	  }
	
	wtokens.at(i).set_subws(subws);

	for(auto ind:inds)
	  {
	    tinds.push_back(ind);
	  }
      }

    //{
    //LOG_S(INFO) << "original: " << text;
    //}

    /*
    {
      for(auto ind:tinds)
	{
	  LOG_S(INFO) << ind << "\t" << this->to_token(ind);
	}
      
      auto text_ = this->decode(tinds);
      LOG_S(INFO) << "reconstructed: " << text_;
    }
    
    {
      auto inds = this->encode(text);
      for(auto ind:inds)
	{
	  LOG_S(INFO) << ind << "\t" << this->to_token(ind);
	}
      
      auto text_ = this->decode(inds);
      LOG_S(INFO) << "spm: " << text_;
    }
    */
    
    return true;
  }
  
}

#endif
