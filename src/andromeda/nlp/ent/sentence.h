//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_SENTENCE_H_
#define ANDROMEDA_MODELS_ENTITIES_SENTENCE_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, SENTENCE>: public base_nlp_model
  {

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return SENTENCE; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply(subject<TABLE>& subj) { return false; }

    //virtual bool apply(subject<WEBDOC>& subj) { return false; }
    //virtual bool apply(subject<PDFDOC>& subj);
    
  private:

    void initialise();

  private:

    const static std::set<model_name> dependencies;
    
    std::vector<pcre2_expr> exprs;
  };

  const std::set<model_name> nlp_model<ENT, SENTENCE>::dependencies = {NAME, LINK, NUMVAL, CITE, QUOTE,
								       EXPRESSION, PARENTHESIS};
  
  nlp_model<ENT, SENTENCE>::nlp_model()
  {
    initialise();
  }

  nlp_model<ENT, SENTENCE>::~nlp_model()
  {}

  void nlp_model<ENT, SENTENCE>::initialise()
  {
    {
      pcre2_expr expr(this->get_key(), "sentence", R"([A-Z]([^\.\?\!]+)(\.|\?|\!))");
      exprs.push_back(expr);
    }
  }

  bool nlp_model<ENT, SENTENCE>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }
    
    std::string text = subj.text;
    
    for(auto& ent:subj.instances)
      {
	if(dependencies.count(ent.get_model())==1)
	  {
	    if(ent.is_model(NAME) or
	       ent.is_model(EXPRESSION) or
	       ent.is_model(QUOTE))
	      {
		for(std::size_t i=ent.get_char_range(0); i<ent.get_char_range(1); i++)
		  {
		    if(i==ent.get_char_range(0))
		      {
			text[i]='A';
		      }
		    else
		      {
			text[i]='a';
		      }
		  }
	      }
	    else
	      {
		utils::mask(text, ent.get_char_range());
	      }
	  }
      }
    
    std::string orig = subj.text;
    
    for(auto& expr:exprs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    range_type char_range = item.rng;

	    range_type ctok_range = subj.get_char_token_range(char_range);
	    range_type wtok_range = subj.get_word_token_range(char_range);

	    std::string sent = orig.substr(char_range[0], char_range[1]-char_range[0]); 
	    
	    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				       SENTENCE, "",
				       sent, sent,
				       char_range, ctok_range, wtok_range);
	  }
      }
    
    return update_applied_models(subj);
  }

}

#endif
