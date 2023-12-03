//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_PARENTHESIS_H_
#define ANDROMEDA_MODELS_ENTITIES_PARENTHESIS_H_

namespace andromeda
{
  template<>
  class nlp_model<ENT, PARENTHESIS>:
    public base_nlp_model
  {

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return PARENTHESIS; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply_on_table_data(subject<TABLE>& subj);

  private:

    bool initialise();

  private:

    const static std::set<model_name> dependencies;

    std::vector<pcre2_expr> exprs;
  };

  const std::set<model_name> nlp_model<ENT, PARENTHESIS>::dependencies = {};

  nlp_model<ENT, PARENTHESIS>::nlp_model()
  {
    initialise();
  }

  nlp_model<ENT, PARENTHESIS>::~nlp_model()
  {}

  bool nlp_model<ENT, PARENTHESIS>::initialise()
  {
    {
      pcre2_expr expr(this->get_key(), "reference", R"(\((?P<content>\d+)\))");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "reference", R"(\[(?P<content>\d+)\])");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "reference", R"(\(\d+\-\d+\))");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "reference", R"(\[\d+\-\d+\])");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "reference", R"(\((\d+\,\s*)*\d+\-\d+\))");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "reference", R"(\[(\d+\,\s*)*\d+\-\d+\])");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "round brackets", R"(\([^\(\)]+\))");
      exprs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "square brackets", R"(\[[^\[\]]+\])");
      exprs.push_back(expr);
    }

    return true;
  }

  bool nlp_model<ENT, PARENTHESIS>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    std::string text = subj.get_text();

    bool updating=true;

    while(updating)
      {
        updating=false;

        for(auto& expr:exprs)
          {
            std::vector<pcre2_item> items;
            expr.find_all(text, items);

            for(auto& item:items)
              {
                auto char_range = item.rng;

                auto ctok_range = subj.get_char_token_range(char_range);
                auto wtok_range = subj.get_word_token_range(char_range);

                std::string orig = subj.from_char_range(char_range);
                std::string name = subj.from_ctok_range(ctok_range);

                subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                           PARENTHESIS, expr.get_subtype(),
                                           name, orig,
                                           char_range,
                                           ctok_range,
                                           wtok_range);

                utils::mask(text, char_range);
                updating = true;
              }
          }
      }

    return update_applied_models(subj);
  }

  bool nlp_model<ENT, PARENTHESIS>::apply_on_table_data(subject<TABLE>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    for(std::size_t i=0; i<subj.num_rows(); i++)
      {
        for(std::size_t j=0; j<subj.num_cols(); j++)
          {
	    std::string text = subj(i,j).get_text();
	    
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
                    auto char_range = item.rng;

                    auto ctok_range = subj(i,j).get_char_token_range(char_range);
                    auto wtok_range = subj(i,j).get_word_token_range(char_range);

                    std::string orig = subj(i,j).from_char_range(char_range);
                    std::string name = subj(i,j).from_ctok_range(ctok_range);

		    auto coor = subj(i,j).get_coor();
		    auto row_span = subj(i,j).get_row_span();
		    auto col_span = subj(i,j).get_col_span();
		    
                    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
						PARENTHESIS, expr.get_subtype(),
						name, orig,
						coor, row_span, col_span,
						char_range,
						ctok_range,
						wtok_range);

                    utils::mask(text, char_range);
                  }
              }
          }
      }
    
    return update_applied_models(subj);
  }

}

#endif
