//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_EXPRESSION_H_
#define ANDROMEDA_MODELS_ENTITIES_EXPRESSION_H_

namespace andromeda
{
  template<>
  class nlp_model<ENT, EXPRESSION>:
    public base_nlp_model
  {
  public:

    const static inline std::string CONTRACTION_LABEL = "contraction";

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return EXPRESSION; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply_on_table_data(subject<TABLE>& subj);

  private:

    bool initialise();

    std::string normalise(std::string orig);

    bool apply_normalisation_regexes(subject<TEXT>& subj);

    bool apply_common_regex(subject<TEXT>& subj);
    bool apply_apostrophe_regex(subject<TEXT>& subj);
    bool apply_abbr_regex(subject<TEXT>& subj);

    bool apply_regex(subject<TEXT>& subj);

    bool apply_concatenation_regex(subject<TEXT>& subj);
    bool apply_concatenation_regex(subject<TABLE>& subj);
    
    bool apply_latex_regex(subject<TEXT>& subj);

    bool find_concatenated_wtokens(subject<TEXT>& subj);
    void add_concatenated_expression(subject<TEXT>& subj,
                                     std::list<std::size_t> wtoken_inds);

    bool post_process(subject<TEXT>& subj);

  private:

    const static std::set<model_name> dependencies;

    pcre2_expr concat_normalisation, single_word, faulty_wtoken_pattern;

    std::vector<pcre2_expr> faulty_wtoken_patterns;

    std::vector<pcre2_expr> concat_exprs;
    std::vector<pcre2_expr> latex_exprs;
    std::vector<pcre2_expr> html_exprs;

    std::vector<pcre2_expr> common_exprs;
    std::vector<std::string> common_names;

    std::vector<pcre2_expr> apo_exprs;
    std::vector<pcre2_expr> abbr_exprs;
  };

  const std::set<model_name> nlp_model<ENT, EXPRESSION>::dependencies = {NAME, LINK, CITE, NUMVAL,
                                                                         QUOTE, PARENTHESIS};

  nlp_model<ENT, EXPRESSION>::nlp_model():
    concat_normalisation("normalisation", "normalisation", R"(\s*\-\s*)"),
    single_word("single-word", "single-word", R"((\s)[A-Za-z\-]+(\s|\.|\,))")
  {
    initialise();
  }

  nlp_model<ENT, EXPRESSION>::~nlp_model()
  {}

  bool nlp_model<ENT, EXPRESSION>::initialise()
  {
    // faulty patterns
    {
      // `model.Finally`
      {
        pcre2_expr expr("faulty_wtoken_pattern", "faulty_wtoken_pattern",
                        R"(^([a-z]+)(\.)[A-Z]?([a-z]+)$)");
        faulty_wtoken_patterns.push_back(expr);
      }
      // `Finally,the`
      {
        pcre2_expr expr("faulty_wtoken_pattern", "faulty_wtoken_pattern",
                        R"(^([A-Z])([a-z]+)(\,)([a-z]+)$)");
        faulty_wtoken_patterns.push_back(expr);
      }

      // `model.Finally,the`
      {
        pcre2_expr expr("faulty_wtoken_pattern", "faulty_wtoken_pattern",
                        R"(^([a-z]+)(\.)([A-Z])([a-z]+)(\,)([a-z]+)$)");
        faulty_wtoken_patterns.push_back(expr);
      }

      // `model.[1][2]` or `model.[1,2]` `model.[1-2]`
      {
        pcre2_expr expr("faulty_wtoken_pattern", "faulty_wtoken_pattern",
                        R"(^([a-z]+)(\.)(\[\d+((\,|\-)\d+)*\])+$)");
        faulty_wtoken_patterns.push_back(expr);
      }
    }

    // concatenations
    {
      // example: `$$d$$-wave`
      {
        pcre2_expr expr(this->get_key(), "latex-concatenation", R"((\s|^)(?P<expr>\${2}[^\$]+\${2}(\s*\-\s*)[a-z]+)(\s|\.|\,|\?))");
        concat_exprs.push_back(expr);
      }

      // example: `$d$-wave`
      {
        pcre2_expr expr(this->get_key(), "latex-concatenation", R"((\s|^)(?P<expr>\$[^\$]+\$(\s*\-\s*)[a-z]+)(\s|\.|\,|\?))");
        concat_exprs.push_back(expr);
      }

      // example: `high-$$T_c$$`
      {
        pcre2_expr expr(this->get_key(), "latex-concatenation", R"((\s|^)(?P<expr>[A-Za-z]+(\s*\-\s*)\${2}[^\$]+\${2})(\s|\.|\,|\?))");
        concat_exprs.push_back(expr);
      }

      // example: `high-$T_c$`
      {
        pcre2_expr expr(this->get_key(), "latex-concatenation", R"((\s|^)(?P<expr>[A-Za-z]+(\s*\-\s*)\${1}[^\$]+\${1})(\s|\.|\,|\?))");
        concat_exprs.push_back(expr);
      }

      // example `fine- tuning`, `in- situ`, `in - situ`
      {
        pcre2_expr expr(this->get_key(), "word-concatenation", R"((\s|^)(?<expr>(([A-Za-z]+)(\s*)(\-\s*)+([A-Za-z]+))+)(\s|\.|\,|\?))");
        concat_exprs.push_back(expr);
      }

      // example `randomization/sanitization`,
      {
        pcre2_expr expr(this->get_key(), "word-concatenation", R"((\s|^)(?<expr>(([A-Za-z]+)(\s*)(\/\s*)+([A-Za-z]+))+)(\s|\.|\,|\?))");
        concat_exprs.push_back(expr);
      }

      // example `(sub)structure`,
      {
        pcre2_expr expr(this->get_key(), "word-concatenation", R"((\s|^)(?<expr>(\(([A-Za-z]+)\)[A-Za-z\-]+))(\s|\.|\,|\?))");
        concat_exprs.push_back(expr);
      }
    }

    // latex expressions
    {
      // example $$D=4$$
      {
        pcre2_expr expr(this->get_key(), "latex", R"((?P<expr>\${2}[^\$]+\${2}))");
        latex_exprs.push_back(expr);
      }

      // example $D=4$
      {
        pcre2_expr expr(this->get_key(), "latex", R"((?P<expr>\$[^\$]+\$))");
        latex_exprs.push_back(expr);
      }

      // example: \\sum_{sss}^{dca}, \\frac{1}{3}, \cite{McMillan}
      {
        pcre2_expr expr(this->get_key(), "latex", R"((?P<expr>\\[a-z]+((\_|\^)?(\{[^\}\$]+\}))+))");
        latex_exprs.push_back(expr);
      }

      // example: {\cdot}
      {
        pcre2_expr expr(this->get_key(), "latex", R"((?P<expr>(\\)?\{(\s*)\\[^\}]+(\s*)(\\)?\}))");
        latex_exprs.push_back(expr);
      }

      // example: \[\cdot\]
      {
        pcre2_expr expr(this->get_key(), "latex", R"((?P<expr>(\\)\[\s*\\[^\]]+(\\)\]))");
        latex_exprs.push_back(expr);
      }

      // example: \begin{eqn} ... \end{eqn}
      {
        pcre2_expr expr(this->get_key(), "latex", R"((?P<expr>\\begin\{[^\}]+\}(.*)\\end\{[^\}]+\}))");
        latex_exprs.push_back(expr);
      }
    }

    // latex expressions
    {
      // example: `<a  href=cnn.com>CNN<\a>`
      {
        pcre2_expr expr(this->get_key(), "html", R"((?P<expr>\<[a-z][^\<\>]*\>(?P<content>([^\<\>])+)\<\\[a-z]\>))");
        html_exprs.push_back(expr);
      }
    }

    // common expressions
    {
      // example `--`
      {
        pcre2_expr expr(this->get_key(), "repetition", R"((?<expr>(\-){2,}))");
        common_exprs.push_back(expr);
        common_names.push_back("-");
      }

      // example `''` or `'''`
      {
        pcre2_expr expr(this->get_key(), "repetition", R"((?<expr>(\'){2,}))");
        common_exprs.push_back(expr);
        common_names.push_back("'");
      }

      // example `"` or `"`
      {
        pcre2_expr expr(this->get_key(), "repetition", R"((?<expr>(\"){2,}))");
        common_exprs.push_back(expr);
        common_names.push_back("\"");
      }

      // example `"` or `"`
      {
        pcre2_expr expr(this->get_key(), "repetition", R"((?<expr>(\=){2,}))");
        common_exprs.push_back(expr);
        common_names.push_back("=");
      }

      // example `i.e.`
      {
        pcre2_expr expr(this->get_key(), "common", R"((?<expr>(i\.?\s?e\.))(\s|\,))");
        common_exprs.push_back(expr);
        common_names.push_back("ie");
      }

      // example `e.g.`
      {
        pcre2_expr expr(this->get_key(), "common", R"((?<expr>(e(\.|\s)*g\.))(\s|\,))");
        common_exprs.push_back(expr);
        common_names.push_back("eg");
      }

      // example `et. al.`
      {
        pcre2_expr expr(this->get_key(), "common", R"((?<expr>(et\.?\s+al\.?)))");
        common_exprs.push_back(expr);
        common_names.push_back("et al");
      }

      // example `etc.`, `...`
      {
        pcre2_expr expr(this->get_key(), "common", R"((?<expr>((\.{3,})|(etc\.))))");
        common_exprs.push_back(expr);
        common_names.push_back("etc");
      }

      // example `...`
      {
        pcre2_expr expr(this->get_key(), "common", R"((?<expr>((\.\s*){2,}\.)))");
        common_exprs.push_back(expr);
        common_names.push_back("etc");
      }

      // example `Grant No. 1`
      {
        pcre2_expr expr(this->get_key(), "common", R"((?<expr>((N|n)o\.)))");
        common_exprs.push_back(expr);
        common_names.push_back("number");
      }      
    }

    {
      // example `viz.`, `no.`, `pp.`, `vol.`,
      {
        pcre2_expr expr(this->get_key(), "common-abbreviation", R"((\s)(?<expr>((?<abbr>(viz|no|pp|vol|ser))\.))(\s))");
        abbr_exprs.push_back(expr);
      }
    }

    // apostrophes
    {
      // example `model's`, `don't`
      {
        pcre2_expr expr(this->get_key(), "apostrophe", R"((\s|^)(?<expr>([A-Za-z]+\'[a-z]))(\s))");
        apo_exprs.push_back(expr);
      }

      // example `superconductors'`
      {
        pcre2_expr expr(this->get_key(), "apostrophe", R"((\s|^)(?<expr>([A-Za-z]+\'))(\s))");
        apo_exprs.push_back(expr);
      }
    }

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_on_table_data(subject<TABLE>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    apply_concatenation_regex(subj);
    
    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply(subject<TEXT>& subj)
  {
    //LOG_S(INFO) << "starting expression ...";

    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    //subj.show(true, false, false, true, false, true, false);

    apply_normalisation_regexes(subj);

    //subj.show(false, false, false, true, false, true, false);

    apply_regex(subj);

    find_concatenated_wtokens(subj);

    post_process(subj);

    // FIXME not sure ...
    //subj.contract_wtokens_from_instances(EXPRESSION);

    //subj.show(false, false, false, true, false, true, false);

    return update_applied_models(subj);
  }

  bool nlp_model<ENT, EXPRESSION>::apply_normalisation_regexes(subject<TEXT>& subj)
  {
    apply_common_regex(subj);

    apply_apostrophe_regex(subj);

    apply_abbr_regex(subj);

    // FIXME not sure ...
    //subj.contract_wtokens_from_instances(EXPRESSION);

    for(auto& ent:subj.instances)
      {
        if(ent.is_model(EXPRESSION) and ent.is_subtype("common") and ent.wtoken_len()==1)
          {
            //subj.word_tokens.at(ent.get_wtok_range(0)).set_word(ent.get_name());
	    subj.set_word(ent.get_wtok_range(0), ent.get_name());
          }
        else if(ent.is_model(EXPRESSION) and ent.is_subtype("apostrophe") and ent.wtoken_len()==1)
          {
            //subj.word_tokens.at(ent.get_wtok_range(0)).set_word(ent.get_name());
	    subj.set_word(ent.get_wtok_range(0), ent.get_name());
          }
        else
          {}
      }

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_common_regex(subject<TEXT>& subj)
  {
    //std::string orig = subj.get_text();
    std::string text = subj.get_text();

    //std::size_t max_id = subj.get_max_ent_hash();

    for(std::size_t l=0; l<common_exprs.size(); l++)
      {
        auto& expr = common_exprs.at(l);

        std::vector<pcre2_item> items;
        expr.find_all(text, items);

        for(auto& item:items)
          {
            for(auto& grp:item.groups)
              {
                if(grp.group_name=="expr")
                  {
                    range_type char_range = grp.rng;

                    range_type ctok_range = subj.get_char_token_range(char_range);
                    range_type wtok_range = subj.get_word_token_range(char_range);

                    std::string name="", orig="";

                    orig = subj.from_ctok_range(ctok_range);

                    if(expr.get_subtype()=="repetition")
                      {
                        name = orig;
                      }
                    else
                      {
                        name = common_names.at(l);
                      }
                    //LOG_S(INFO) << __FUNCTION__ << " " << l << ": " << orig;

                    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                               EXPRESSION, expr.get_subtype(),
                                               name, orig,
                                               char_range, ctok_range, wtok_range);
                  }
              }
          }
      }

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_apostrophe_regex(subject<TEXT>& subj)
  {
    std::string text = subj.get_text();

    //std::size_t max_id = subj.get_max_ent_hash();

    for(std::size_t l=0; l<apo_exprs.size(); l++)
      {
        auto& expr = apo_exprs.at(l);

        std::vector<pcre2_item> items;
        expr.find_all(text, items);

        for(auto& item:items)
          {
            //LOG_S(INFO) << item.to_json().dump();

            for(auto& grp:item.groups)
              {
                if(grp.group_name=="expr")
                  {
                    range_type char_range = grp.rng;

                    range_type ctok_range = subj.get_char_token_range(char_range);
                    range_type wtok_range = subj.get_word_token_range(char_range);

                    std::string orig="", name="";

                    orig = subj.from_ctok_range(ctok_range);
                    name = utils::replace(orig, "'", "");

                    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                               EXPRESSION, expr.get_subtype(),
                                               name, orig,
                                               char_range, ctok_range, wtok_range);
                  }
              }
          }
      }

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_abbr_regex(subject<TEXT>& subj)
  {
    std::string text = subj.get_text();

    for(std::size_t l=0; l<abbr_exprs.size(); l++)
      {
        auto& expr = abbr_exprs.at(l);

        std::vector<pcre2_item> items;
        expr.find_all(text, items);

        for(auto& item:items)
          {
            //LOG_S(INFO) << __FUNCTION__ << ": " << item.to_json().dump();

            for(auto& grp:item.groups)
              {
                if(grp.group_name=="expr")
                  {
                    range_type char_range = grp.rng;

                    range_type ctok_range = subj.get_char_token_range(char_range);
                    range_type wtok_range = subj.get_word_token_range(char_range);

                    std::string orig="", name="";

                    orig = subj.from_ctok_range(ctok_range);
                    name = utils::replace(orig, ".", "");

                    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                               EXPRESSION, expr.get_subtype(),
                                               name, orig,
                                               char_range, ctok_range, wtok_range);
                  }
              }
          }
      }

    return true;
  }

  std::string nlp_model<ENT, EXPRESSION>::normalise(std::string orig)
  {
    std::string name = orig;

    name = utils::replace(name, "$", "");
    name = utils::replace(name, "  ", " ");
    name = utils::replace(name, " _", "_");
    name = utils::replace(name, "_ ", "_");
    name = utils::strip(name);

    return name;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_regex(subject<TEXT>& subj)
  {
    apply_concatenation_regex(subj);

    apply_latex_regex(subj);

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_concatenation_regex(subject<TEXT>& subj)
  {
    std::string text = subj.get_text();

    // find all concat expressions
    for(auto& expr:concat_exprs)
      {
        std::vector<pcre2_item> items;
        expr.find_all(text, items);

        for(auto& item:items)
          {
            //LOG_S(INFO) << item.to_json().dump();

            for(auto& grp:item.groups)
              {
                if(grp.group_name=="expr")
                  {
                    range_type char_range = grp.rng;

                    range_type ctok_range = subj.get_char_token_range(grp.rng);
                    range_type wtok_range = subj.get_word_token_range(grp.rng);

                    std::string orig="", name="";

                    orig = subj.from_ctok_range(ctok_range);
                    name = normalise(orig);

                    // FIXME: it would be nice to have something more sophisticated ...
                    bool keep=true;
                    if(name.ends_with(" and") or
                       name.ends_with(" or"))
                      {
                        keep = false;
                      }

                    if(keep)
                      {
                        subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
						   EXPRESSION, expr.get_subtype(),
                                                   name, orig,
                                                   char_range, ctok_range, wtok_range);
                      }

                    utils::mask(text, char_range);
                  }
              }
          }
      }

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_concatenation_regex(subject<TABLE>& subj)
  {
    for(std::size_t i=0; i<subj.num_rows(); i++)
      {
        for(std::size_t j=0; j<subj.num_cols(); j++)
          {
            if(subj(i,j).get_text().size()==0)
              {
                continue;
              }

            std::string text = subj(i,j).get_text();

            // find all concat expressions
            for(auto& expr:concat_exprs)
              {
                std::vector<pcre2_item> items;
                expr.find_all(text, items);

                for(auto& item:items)
                  {
                    //LOG_S(INFO) << item.to_json().dump();

                    for(auto& grp:item.groups)
                      {
                        if(grp.group_name=="expr")
                          {
                            range_type char_range = grp.rng;

                            range_type ctok_range = subj(i,j).get_char_token_range(grp.rng);
                            range_type wtok_range = subj(i,j).get_word_token_range(grp.rng);

                            std::string orig="", name="";

                            orig = subj(i,j).from_ctok_range(ctok_range);
                            name = normalise(orig);

                            // FIXME: it would be nice to have something more sophisticated ...
                            bool keep=true;
                            if(name.ends_with(" and") or
                               name.ends_with(" or"))
                              {
                                keep = false;
                              }

                            if(keep)
                              {
                                subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
							    EXPRESSION, expr.get_subtype(),
							    name, orig,
							    subj(i,j).get_coor(),
							    subj(i,j).get_row_span(),
							    subj(i,j).get_col_span(),
							    char_range,
							    ctok_range,
							    wtok_range);
                              }

                            utils::mask(text, char_range);
                          }
                      }
                  }
              }
          }
      }

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::apply_latex_regex(subject<TEXT>& subj)
  {
    //std::string orig = subj.get_text();
    std::string text = subj.get_text();

    for(auto& ent:subj.instances)
      {
        if(ent.is_model(CITE))
          {
            utils::mask(text, ent.get_char_range());
          }
      }

    // find all latex expressions
    bool found_new = true;
    while(found_new)
      {
        found_new = false;
        for(auto& expr:latex_exprs)
          {
            std::vector<pcre2_item> items;
            expr.find_all(text, items);

            for(auto& item:items)
              {
                //LOG_S(INFO) << item.to_json().dump();

                for(auto& grp:item.groups)
                  {
                    if(grp.group_name=="expr")
                      {
                        range_type char_range = grp.rng;

                        range_type ctok_range = subj.get_char_token_range(char_range);
                        range_type wtok_range = subj.get_word_token_range(char_range);

                        std::string orig="", name="";

                        orig = subj.from_ctok_range(ctok_range);
                        name = normalise(orig);

                        subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                                   EXPRESSION, expr.get_subtype(),
                                                   name, orig,
                                                   char_range, ctok_range, wtok_range);

                        utils::mask(text, char_range);
                      }
                  }
              }
          }
      }

    return true;
  }

  bool nlp_model<ENT, EXPRESSION>::find_concatenated_wtokens(subject<TEXT>& subj)
  {
    std::set<std::size_t> forbidden_inds={};
    for(auto& ent:subj.instances)
      {
        if(ent.is_model(CITE))
          {
            for(std::size_t ind=ent.get_wtok_range(0); ind<ent.get_wtok_range(1); ind++)
              {
                forbidden_inds.insert(ind);
              }
          }
      }

    auto& wtokens = subj.get_word_tokens();

    std::list<std::size_t> wtoken_inds={};
    for(std::size_t l=0; l<wtokens.size(); l++)
      {
        if(wtoken_inds.size()==0)
          {
            wtoken_inds.push_back(l);
          }
        else if(wtoken_inds.size()==0 and forbidden_inds.count(l)==1)
          {
            wtoken_inds = {l};
          }
        else if(wtoken_inds.size()>0 and forbidden_inds.count(l)==1)
          {
            add_concatenated_expression(subj, wtoken_inds);

            wtoken_inds = {};
          }
        else if(wtoken_inds.size()>0 and
                wtokens.at(l-1).get_rng(1)==wtokens.at(l).get_rng(0))
          {
            wtoken_inds.push_back(l);
          }
        else
          {
            add_concatenated_expression(subj, wtoken_inds);

            wtoken_inds = {l};
          }
      }

    return true;
  }

  void nlp_model<ENT, EXPRESSION>::add_concatenated_expression(subject<TEXT>& subj,
                                                               std::list<std::size_t> wtoken_inds)
  {
    auto& wtokens = subj.get_word_tokens();

    std::set<std::string> special_begins  = {"\"", "'", "''", "{", "}", ".", ",", ";", "/"};
    std::set<std::string> special_endings = {".",",","?","!",":", ";", "\"", "'", "''"};

    while(wtoken_inds.size()>=2)
      {
        index_type wind = wtoken_inds.front();
        std::string word = wtokens.at(wind).get_word();

        if(special_begins.count(word)==1)
          {
            wtoken_inds.pop_front();
          }
        else
          {
            break;
          }
      }

    while(wtoken_inds.size()>=2)
      {
        index_type wind = wtoken_inds.back();
        std::string word = wtokens.at(wind).get_word();

        if(special_endings.count(word)==1)
          {
            wtoken_inds.pop_back();
          }
        else
          {
            break;
          }
      }

    if(wtoken_inds.size()>=2)
      {
        index_type i0 = wtoken_inds.front();
        index_type i1 = wtoken_inds.back();

        range_type char_range =
          {
           wtokens.at(i0).get_rng(0),
           wtokens.at(i1).get_rng(1)
          };

        range_type ctok_range = subj.get_char_token_range(char_range);
        range_type wtok_range = subj.get_word_token_range(char_range);

        std::string orig="", name="";

        orig = subj.from_wtok_range(wtok_range);
        name = normalise(orig);

        //LOG_S(INFO) << "wtoken-concatenation: " << name;

        bool keep=true;
        for(auto& expr:faulty_wtoken_patterns)
          {
            if(expr.match(orig))
              {
                keep = false;
              }
          }

        if(keep)
          {
            //std::size_t max_id = subj.get_max_ent_hash();

            subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                       EXPRESSION, "wtoken-concatenation",
                                       name, orig,
                                       char_range, ctok_range, wtok_range);
          }
      }
  }

  bool nlp_model<ENT, EXPRESSION>::post_process(subject<TEXT>& subj)
  {
    auto& insts = subj.instances;

    auto itr=insts.begin();

    while(itr!=insts.end())
      {
        auto& ent = *itr;

        if(ent.is_model(EXPRESSION) and ent.is_subtype("common"))
          {
            itr++;
          }
        else if(ent.is_model(EXPRESSION))
          {
            std::string orig = ent.get_orig();

            int cnt_$       = utils::count(orig, '$');
            int diff_cnt_rb = utils::count_imbalance(orig, '(', ')');
            int diff_cnt_cb = utils::count_imbalance(orig, '{', '}');
            int diff_cnt_sb = utils::count_imbalance(orig, '[', ']');

            std::vector<pcre2_item> words;
            std::string text = ent.get_orig();

            {
              while(true)
                {
                  //LOG_S(INFO) << "text: " << text;
                  std::size_t N = words.size();

                  single_word.find_all(text, words);

                  for(auto& word:words)
                    {
                      utils::mask(text, word.rng);
                    }

                  if(words.size()==0 or N==words.size())
                    {
                      break;
                    }
                }

              //LOG_S(INFO) << "text: " << text;
              text = normalise(text);
            }

            //LOG_S(WARNING) << ent.name << " ->" << words.size();
            //for(auto word:words)
            //{
            //LOG_S(WARNING) << "\t ->" << word.get_text();
            //}

            if(orig.starts_with("(") and orig.ends_with(")"))
              {
                //LOG_S(INFO) << " -> removing: " << ent.orig << "; " << ent.name;
                itr = insts.erase(itr);
              }
            else if(text.size()==0 or words.size()>=3)
              {
                //LOG_S(INFO) << " -> removing: " << ent.orig;
                itr = insts.erase(itr);
              }
            else if(((cnt_$%2)!=0 or
                     diff_cnt_rb!=0 or
                     diff_cnt_cb!=0 or
                     diff_cnt_sb!=0))
              {
                //LOG_S(INFO) << " -> removing: " << ent.orig << "; " << ent.name;
                itr = insts.erase(itr);
              }
            else // keep it ...
              {
                //LOG_S(INFO) << " -> keeping: " << ent.orig << "; " << ent.name;
                itr++;
              }
          }
        else
          {
            itr++;
          }
      }

    //LOG_S(INFO) << "removed inconsistent expressions ...";

    bool erasing=true;
    while(erasing)
      {
        erasing=false;

        //LOG_S(INFO) << "#-instances: " << insts.size();

        for(auto itr_i=insts.begin(); itr_i!=insts.end(); itr_i++)
          {
            for(auto itr_j=insts.begin(); itr_j!=insts.end(); itr_j++)
              {
                auto cr_i = itr_i->get_char_range();
                auto cr_j = itr_j->get_char_range();

                if(itr_i!=itr_j and
                   itr_i->is_model(EXPRESSION) and
                   itr_j->is_model(EXPRESSION) and
                   cr_i==cr_j and
		   itr_i->is_subtype("wtoken-concatenation"))
                  {
                    //LOG_S(INFO) << "removing: " << itr_i->orig << "; " << itr_i->name;

                    itr_i = insts.erase(itr_i);
                    erasing=true;
                  }
                if(itr_i!=itr_j and
                   itr_i->is_model(EXPRESSION) and
                   itr_j->is_model(EXPRESSION) and
                   ((cr_j[0]<=cr_i[0] and cr_i[1]<cr_j[1]) or
                    (cr_j[0]<cr_i[0] and cr_i[1]<=cr_j[1]) ) )
                  {
                    //LOG_S(INFO) << "removing: " << itr_i->orig << "; " << itr_i->name;

                    itr_i = insts.erase(itr_i);
                    erasing=true;
                  }
                else if(itr_i->is_model(EXPRESSION) and
                        (itr_j->is_model(NUMVAL) or itr_j->is_model(NAME)) and
                        cr_i==cr_j)
                  {
                    //LOG_S(INFO) << "removing: " << itr_i->orig << "; " << itr_i->name;

                    itr_i = insts.erase(itr_i);
                    erasing=true;
                  }
                else
                  {}

                if(erasing)
                  {
                    break;
                  }
              }

            if(erasing)
              {
                break;
              }
          }
      }

    return true;
  }

}

#endif
