//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_QUOTE_H_
#define ANDROMEDA_MODELS_ENTITIES_QUOTE_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, QUOTE>:
    public base_nlp_model
  {

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return QUOTE; }

    virtual bool apply(subject<TEXT>& subj);
    //virtual bool apply(subject<TABLE>& subj) { return false; }
    
  private:
    
    bool initialise();

    bool post_process(nlohmann::json& insts);
    
  private:

    const static std::set<model_name> dependencies;
    
    std::vector<pcre2_expr> exprs;
  };

  const std::set<model_name> nlp_model<ENT, QUOTE>::dependencies = {NUMVAL};
  
  nlp_model<ENT, QUOTE>::nlp_model()
  {
    initialise();
  }

  nlp_model<ENT, QUOTE>::~nlp_model()
  {}

  bool nlp_model<ENT, QUOTE>::initialise()
  {
    {
      pcre2_expr expr(this->get_key(), "quote", R"((?P<quote>(\")[A-Za-z][^\"\.]*[^\\](\.|\?|\!)?\"))");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "quote", R"((?P<quote>(\'{3})[A-Za-z][^\'\.]*[^\\](\.|\?|\!)?\'{3}))");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "quote", R"((?P<quote>(\'{2})[A-Za-z][^\"\.]*[^\\](\.|\?\!)?\"))");
      exprs.push_back(expr);
    }
    
    {
      pcre2_expr expr(this->get_key(), "quote", R"((?P<quote>(\'{2})[A-Za-z][^\'\.]*[^\\](\.|\?|\!)?\'{2}))");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "quote", R"([^A-Za-z](?P<quote>(\'{1})[A-Za-z][A-Za-z\-\s]+(\.|\?|\!)?\'{1}))");
      exprs.push_back(expr);
    }    
        
    return true;
  }

  bool nlp_model<ENT, QUOTE>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    std::string text = subj.get_text();
    for(auto& inst:subj.instances)
      {
	if(dependencies.count(inst.get_model())==1)
	  {
	    utils::mask(text, inst.get_char_range());
	  }
      }
    
    for(auto& expr:exprs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    for(auto& grp:item.groups)
	      {
		if(grp.group_name=="quote")
		  {
		    auto char_range = grp.rng;

		    auto ctok_range = subj.get_char_token_range(char_range);
		    auto wtok_range = subj.get_word_token_range(char_range);

		    std::string orig = subj.from_char_range(char_range);
		    std::string name = subj.from_ctok_range(ctok_range);
		    
		    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
					       QUOTE, expr.get_subtype(),
					       name, orig, 
					       char_range, ctok_range, wtok_range);

		    //LOG_S(INFO) << "quote: " << orig;
		    
		    utils::mask(text, item.rng);
		  }
	      }
	  }
      }
    //LOG_S(INFO) << " --> done quote!";
    
    return update_applied_models(subj);
  }
  
}

#endif
