//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_GEOLOC_H_
#define ANDROMEDA_MODELS_ENTITIES_GEOLOC_H_

namespace andromeda
{
  template<>
  class nlp_model<ENT, GEOLOC>: public base_nlp_model
  {
  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return GEOLOC; }

    virtual bool apply(std::string& text, nlohmann::json& annots);

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply_on_table_data(subject<TABLE>& subj);

  private:

    bool initialise();

    //bool apply_regex(subject<TEXT>& subj);
    //bool contract_regex(subject<TEXT>& subj);

  private:

    const static inline std::set<std::string> allowed_subtypes={"continent", "country", "aquatic-region"};

    const static std::set<model_name> dependencies;

    std::vector<pcre2_expr> exprs;

    std::filesystem::path asset_file;
    std::filesystem::path model_file;

    nlohmann::json assets;

    std::map<std::string, std::string> l2s; // label to subtype (subtypes might have `-` in the name, which are not accepted as regex named groups)

    std::map<std::string, index_type> h2j; // headers to column index
    std::map<std::string, std::vector<index_type> > l2inds={}; // label to row-indices

    //base_rgx_model rgx_model;
  };

  const std::set<model_name> nlp_model<ENT, GEOLOC>::dependencies = {};

  nlp_model<ENT, GEOLOC>::nlp_model():
    exprs({}),

    asset_file(get_rgx_dir() / "geoloc/rgx_geoloc.json"),
    model_file(get_crf_dir() / "geoloc/crf_geoloc.bin"),

    assets(nlohmann::json::value_t::null)
  {
    initialise();
  }

  nlp_model<ENT, GEOLOC>::~nlp_model()
  {}

  bool nlp_model<ENT, GEOLOC>::initialise()
  {
    std::ifstream ifs(asset_file.c_str());

    if(ifs)
      {
        ifs >> assets;
      }
    else
      {
        LOG_S(ERROR) << "could not find " << asset_file;

        return false;
      }

    std::vector<std::string> headers = {};
    headers = assets.value("headers", headers);

    auto& data = assets.at("data");

    h2j={};
    for(index_type j=0; j<headers.size(); j++)
      {
        h2j[headers.at(j)] = j;
      }

    //index_type type_cind = h2j.at("type");
    index_type subtype_cind = h2j.at("subtype");
    index_type expr_cind = h2j.at("expression");

    l2s={};
    l2inds={};

    for(index_type i=0; i<data.size(); i++)
      {
        std::string subtype = data.at(i).at(subtype_cind).get<std::string>();
        std::string label = utils::replace(subtype, "-", "_");

        if(l2inds.count(label))
          {
            l2inds.at(label).push_back(i);
          }
        else if(allowed_subtypes.count(subtype))
          {
            l2inds[label] = {i};
            l2s[label] = subtype;
          }
      }

    index_type delta=128;
    for(auto itr=l2inds.begin(); itr!=l2inds.end(); itr++)
      {
        auto label = itr->first;
        auto& inds = itr->second;

        //LOG_S(INFO) << "init geoloc subtype " << l2s.at(label) << ": " << l2inds.at(label).size();

        index_type len = inds.size();
        for(index_type i0=0; i0<inds.size(); i0+=delta)
          {
            index_type lb = i0;
            index_type ub = std::min(i0+delta, len);

            std::stringstream ss;

            ss << R"((^|\s))" << "(?<" << label << ">";
            for(index_type i1=lb; i1<ub; i1++)
              {
                std::string cexpr = data.at(inds.at(i1)).at(expr_cind).get<std::string>();

                ss << cexpr;

                if(i1+1<ub)
                  {
                    ss << "|";
                  }
              }
            ss << ")" << R"(($|\s|\,|.|\:|\;|\?))";

            //LOG_S(INFO) << l2s.at(label) << "(" << lb << "," << ub << "): " << ss.str();

            pcre2_expr expr(this->get_key(), l2s.at(label), ss.str());
            exprs.push_back(expr);
          }
      }

    return (exprs.size()>0);
  }

  bool nlp_model<ENT, GEOLOC>::apply(std::string& text, nlohmann::json& annots)
  {
    LOG_S(ERROR) << __FUNCTION__ << " on text not implemented ...";
    return false;
  }

  bool nlp_model<ENT, GEOLOC>::apply(subject<TEXT>& subj)
  {
    //LOG_S(ERROR) << __FUNCTION__ << " on paragraph ...";

    std::string text = subj.get_text();

    for(auto& expr:exprs)
      {
        std::vector<pcre2_item> items;
        expr.find_all(text, items);

        for(auto& item:items)
          {
            for(auto& grp:item.groups)
              {
                if(l2inds.count(grp.group_name)==1)
                  {
                    // NOTE: in future, we might need to have individual post-processing
                    // to determine the range.
                    auto char_range = grp.rng;

                    auto ctok_range = subj.get_char_token_range(char_range);
                    auto wtok_range = subj.get_word_token_range(char_range);

                    std::string orig = subj.from_char_range(char_range);
                    std::string name = subj.from_ctok_range(ctok_range);

                    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                                GEOLOC, expr.get_subtype(),
                                                name, orig,
                                                char_range, ctok_range, wtok_range);

                    //utils::mask(text, item.rng);
                  }
              }
          }
      }

    for(auto itr=subj.instances.begin(); itr!=subj.instances.end(); )
      {
        if(not itr->is_wtok_range_match())
          {
            itr = subj.instances.erase(itr);
          }
        else
          {
            itr++;
          }
      }

    return update_applied_models(subj);
  }

  bool nlp_model<ENT, GEOLOC>::apply_on_table_data(subject<TABLE>& subj)
  {
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
                    for(auto& grp:item.groups)
                      {
                        if(l2inds.count(grp.group_name)==1)
                          {
                            // NOTE: in future, we might need to have individual post-processing
                            // to determine the range.
                            auto char_range = grp.rng;

                            auto ctok_range = subj(i,j).get_char_token_range(char_range);
                            auto wtok_range = subj(i,j).get_word_token_range(char_range);

                            std::string orig = subj(i,j).from_char_range(char_range);
                            std::string name = subj(i,j).from_ctok_range(ctok_range);

                            subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
                                                        GEOLOC, expr.get_subtype(),
                                                        name, orig,
							subj(i,j).get_coor(),
							subj(i,j).get_row_span(),
							subj(i,j).get_col_span(),	
                                                        char_range, ctok_range, wtok_range);
                          }
                      }
                  }
              }
          }
      }

    return true;
  }

}

#endif
