//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_LINK_H_
#define ANDROMEDA_MODELS_ENTITIES_LINK_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, LINK>:
    public base_nlp_model
  {
    typedef nlp_model<ENT, LINK> this_type;

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return LINK; }

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

  const std::set<model_name> nlp_model<ENT, LINK>::dependencies = {};

  nlp_model<ENT, LINK>::nlp_model()
  {
    initialise();
  }

  nlp_model<ENT, LINK>::~nlp_model()
  {}

  bool nlp_model<ENT, LINK>::initialise()
  {
    // NOTE: to avoid that the link ends with a `.` or a `)`, we add
    // the `(\.|\))?` at the end.

    // `https://yahoo.com`
    {
      pcre2_expr expr(this->get_key(), "url", R"((?P<link>(https?([^\s\(\)\[\]]|\s\.\s)+)))");
      exprs.push_back(expr);
    }

    // `www.yahoo.com`
    {
      pcre2_expr expr(this->get_key(), "url", R"((?P<link>(www\.?[^\s\(\)\[\]]+)))");
      exprs.push_back(expr);
    }

    // `doi:10.1000/182`
    {
      pcre2_expr expr(this->get_key(), "doi", R"((?P<link>((doi|DOI)\:?\.?\s?[^\s\(\)\[\]]+)))");
      exprs.push_back(expr);
    }

    // `name@gmail.com`
    {
      pcre2_expr expr(this->get_key(), "email", R"((?P<link>([^\s\(\)\[\]]+\@[^\s\(\)\[\]]+)))");
      exprs.push_back(expr);
    }

    // Arxiv:  "arXiv:2201.08390v1 [gr-qc] 20 Jan 2022"
    {
      pcre2_expr expr(this->get_key(), "arxiv", R"((?P<link>(arXiv:(\d+).(\d+)(v\d*)? \[.+\] (\d+) [A-Za-z]+ \d+)))");
      exprs.push_back(expr);
    }
    
    return true;
  }

  bool nlp_model<ENT, LINK>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    apply_regex(subj);

    //subj.contract_wtokens_from_entities(LINK);
    
    return true;
  }

  bool nlp_model<ENT, LINK>::apply_regex(subject<TEXT>& subj)
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

	    // remove spaces
	    name = utils::replace(name, " ", "");
	    
	    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				       LINK, expr.get_subtype(),
				       name, orig,
				       char_range, ctok_range, wtok_range);
          }
      }

    return update_applied_models(subj);
  }

}

#endif
