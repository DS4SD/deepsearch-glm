//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_CITE_H_
#define ANDROMEDA_MODELS_ENTITIES_CITE_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, CITE>:
    public base_nlp_model
  {
    typedef nlp_model<ENT, CITE> this_type;

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return CITE; }

    virtual bool apply(subject<TEXT>& subj);
    //virtual bool apply(subject<TABLE>& subj) { return false; }

  private:

    bool initialise();

    bool apply_regex(subject<TEXT>& subj);
    
    bool post_process(nlohmann::json& insts);

  private:

    const static std::set<model_name> dependencies;

    std::vector<pcre2_expr> exprs;
  };

  const std::set<model_name> nlp_model<ENT, CITE>::dependencies = {};

  nlp_model<ENT, CITE>::nlp_model()
  {
    initialise();
  }

  nlp_model<ENT, CITE>::~nlp_model()
  {}

  bool nlp_model<ENT, CITE>::initialise()
  {
    // `~\cite{xxx}` (latex reference/citation)
    {
      pcre2_expr expr(this->get_key(), "latex-cite", R"((?P<cite>((\~)?(\\)(cite|ref)\{([^\}])+\})))");
      exprs.push_back(expr);
    }

    // `~\cite[xxx]`
    {
      pcre2_expr expr(this->get_key(), "latex-cite", R"((?P<cite>((\~)?(\\)(cite|ref)\[([^\]])+\])))");
      exprs.push_back(expr);
    }
    
    // [[4]], [[4-5]], [[4,5]], [[3,4-5]]
    {
      pcre2_expr expr(this->get_key(), "wiki-cite", R"((?P<cite>(\[\[\d+(\-\d+)?(\,\d+(\-\d+)?)*\]\])))");
      exprs.push_back(expr);
    }

    return true;
  }

  bool nlp_model<ENT, CITE>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    apply_regex(subj);

    //subj.contract_wtokens_from_entities(CITE);
    
    return true;
  }
  
  bool nlp_model<ENT, CITE>::apply_regex(subject<TEXT>& subj)
  {    
    std::string text = subj.get_text();

    for(auto& expr:exprs)
      {
        std::vector<pcre2_item> items;
        expr.find_all(text, items);

        for(auto& item:items)
          {
	    auto char_range = item.rng;

	    while(char_range[0]<char_range[1])
	      {
		char c = text[char_range[1]-1];

		if(c=='.' or c==')')
		  {
		    char_range[1] -= 1;
		  }
		else
		  {
		    break;
		  }
	      }

	    auto ctok_range = subj.get_char_token_range(char_range);
	    auto wtok_range = subj.get_word_token_range(char_range);
	    
	    std::string orig = subj.from_char_range(char_range);
	    std::string name = subj.from_ctok_range(ctok_range);
	    
	    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				       CITE, expr.get_subtype(),
				       name, orig,
				       char_range, ctok_range, wtok_range);
          }
      }

    return update_applied_models(subj);
  }

}

#endif
