//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_NUMVAL_H_
#define ANDROMEDA_MODELS_ENTITIES_NUMVAL_H_

namespace andromeda
{
  template<>
  class nlp_model<ENT, NUMVAL>:
    public base_nlp_model
  {

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return NUMVAL; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply(subject<TABLE>& subj);
    
  private:
    
    bool initialise();

    bool apply_regex(subject<TEXT>& subj);
    
    bool contract_regex(subject<TEXT>& subj);
    
  private:

    const static std::set<model_name> dependencies;
    
    std::vector<pcre2_expr> exprs;
  };

  const std::set<model_name> nlp_model<ENT, NUMVAL>::dependencies = {};
  
  nlp_model<ENT, NUMVAL>::nlp_model()
  {
    initialise();
  }

  nlp_model<ENT, NUMVAL>::~nlp_model()
  {}

  bool nlp_model<ENT, NUMVAL>::initialise()
  {
    // `15th`
    {
      pcre2_expr expr(this->get_key(), "enum", R"((?P<value>(\d+))(st|rd|th))");
      exprs.push_back(expr);
    }

    // `2021`
    {
      pcre2_expr expr(this->get_key(), "year", R"((^|[^\d])(?P<value>((18|19|20)(\d{2}))(ies|\'?s)?)($|[^\d]))");
      exprs.push_back(expr);
    }

    // `40s`, `80ies`
    {
      pcre2_expr expr(this->get_key(), "year", R"((?P<value>((\d{1}0))(ies|\'?s)))");
      exprs.push_back(expr);
    }

    // scientific value `10^(12)` 
    {
      pcre2_expr expr(this->get_key(), "fexp", R"((?<value>(10\^\((\-)?\d+\))))");
      exprs.push_back(expr);
    }

    // scientific value `10^{5}` 
    {
      pcre2_expr expr(this->get_key(), "fexp", R"((?<value>(10\^\{(\-)?\d+\})))");
      exprs.push_back(expr);
    }

    // scientific value `10-$^{5}$` --> error from PDF conversion
    {
      pcre2_expr expr(this->get_key(), "fexp", R"((?<value>(10(\-)\^\{\d+\})))");
      exprs.push_back(expr);
    }

    // scientific value `10^5` 
    {
      pcre2_expr expr(this->get_key(), "fexp", R"((?<value>(10\^(\-)?\d+)))");
      exprs.push_back(expr);
    }    
    
    // scientific value `1.23e5` 
    {
      pcre2_expr expr(this->get_key(), "fsci", R"((?<value>((\-)?(\d+((\.|\,)\d+)?)\s*(e|E)\s*((\-)?\d+((\.|\,)\d+)?))))");
      exprs.push_back(expr);
    }
    
    // floating value range `4.5-5.6`, `4.5--5.6` 
    {
      pcre2_expr expr(this->get_key(), "fval", R"((?<value>((\-)?(\d+(\.|\,)\d+)\s*(\-+)?\s*((\-)?\d+(\.|\,)\d+))))");
      exprs.push_back(expr);
    }
    
    // floating value 3.14
    {
      pcre2_expr expr(this->get_key(), "fval", R"((?<value>((\-\s*)?(\d+(\.|\,)\d+))))");
      exprs.push_back(expr);
    }

    // integer range 12-34
    {
      pcre2_expr expr(this->get_key(), "irng", R"((?<value>((\d+)\s*(\-+)\s*(\d+))))");
      exprs.push_back(expr);
    }
    
    // integer value 12
    {
      pcre2_expr expr(this->get_key(), "ival", R"((?<value>((\-\s*)?(\d+))))");
      exprs.push_back(expr);
    }

    return true;
  }

  bool nlp_model<ENT, NUMVAL>::apply(subject<TEXT>& subj)
  {
    //LOG_S(INFO) << "starting numval ...";
    
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    //subj.show();
    
    apply_regex(subj);

    //subj.show();
    
    subj.contract_wtokens_from_instances(NUMVAL);
    
    //subj.show();
    
    return true;
  }
  
  bool nlp_model<ENT, NUMVAL>::apply_regex(subject<TEXT>& subj)
  {    
    std::string text = subj.text;
    for(auto& expr:exprs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    //LOG_S(INFO) << item.to_json().dump();
	    
	    for(auto& grp:item.groups)
	      {
		if(grp.group_name=="value")
		  {
		    // NOTE: in future, we might need to have individual post-processing
		    // to determine the range.
		    auto char_range = grp.rng;

		    auto ctok_range = subj.get_char_token_range(char_range);
		    auto wtok_range = subj.get_word_token_range(char_range);

		    std::string orig = subj.from_char_range(char_range);
		    std::string name = subj.from_ctok_range(ctok_range);
		    
		    subj.instances.emplace_back(subj.get_hash(),
						NUMVAL, expr.get_subtype(),
						name, orig, 
						char_range, ctok_range, wtok_range);

		    //LOG_S(INFO) << "subj-hash: " << subj.get_hash() << ", name: " << name;
		    
		    utils::mask(text, item.rng);
		  }
	      }
	  }
      }

    return update_applied_models(subj);
  }
  
  bool nlp_model<ENT, NUMVAL>::apply(subject<TABLE>& subj)
  {
    //LOG_S(INFO) << "starting numval ...";
    
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    //subj.show();

    for(std::size_t i=0; i<subj.num_rows(); i++)
      {
	for(std::size_t j=0; j<subj.num_cols(); j++)
	  {	    
	    std::string text = subj(i,j).text;

	    if(text.size()==0)
	      {
		continue;
	      }
	    
	    for(auto& expr:exprs)
	      {
		std::vector<pcre2_item> items;
		expr.find_all(text, items);
		
		for(auto& item:items)
		  {
		    //LOG_S(INFO) << item.to_json().dump();
		    
		    for(auto& grp:item.groups)
		      {
			if(grp.group_name=="value")
			  {
			    // NOTE: in future, we might need to have individual post-processing
			    // to determine the range.
			    auto char_range = grp.rng;

			    auto ctok_range = subj(i,j).get_char_token_range(char_range);
			    auto wtok_range = subj(i,j).get_word_token_range(char_range);
			    
			    std::string orig = subj(i,j).from_char_range(char_range);
			    std::string name = subj(i,j).from_ctok_range(ctok_range);
			    
			    subj.instances.emplace_back(subj.get_hash(),
							NUMVAL, expr.get_subtype(),
							name, orig, 
							subj(i,j).get_coor(),
							subj(i,j).get_row_span(),
							subj(i,j).get_col_span(),
							char_range, ctok_range, wtok_range);
			    
			    utils::mask(text, item.rng);
			  }
		      }
		  }
	      }

	    // FIXME: can be optimised!
	    text = utils::replace(text, "-", "");
	    text = utils::replace(text, "+", "");
	    text = utils::replace(text, "*", "");
	    text = utils::replace(text, ":", "");
	    text = utils::replace(text, "/", "");
	    
	    text = utils::replace(text, "(", "");
	    text = utils::replace(text, ")", "");
	    text = utils::replace(text, "{", "");
	    text = utils::replace(text, "}", "");
	    text = utils::replace(text, "[", "");
	    text = utils::replace(text, "]", "");

	    text = utils::replace(text, "_", "");	    
	    text = utils::replace(text, "^", "");

	    text = utils::replace(text, "$", "");
	    text = utils::replace(text, "%", "");

	    text = utils::replace(text, ".", "");
	    text = utils::replace(text, ",", "");
	    text = utils::replace(text, ";", "");
	    
	    text = utils::strip(text);

	    if(text.size()==0)
	      {
		subj(i,j).set_numeric(true);
	      }

	    //if((not subj(i,j).is_numeric()) and text.size()<3)
	    //{
	    //LOG_S(WARNING) << "table-text '" << text << "': " << subj(i,j).get_text();
	    //}
	  }
      }
    
    return update_applied_models(subj);	    
  }
  
}

#endif
