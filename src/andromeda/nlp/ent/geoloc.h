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
    
    virtual bool apply(subject<PARAGRAPH>& subj);
    virtual bool apply(subject<TABLE>& subj);
    
  private:
    
    bool initialise();

    //bool apply_regex(subject<PARAGRAPH>& subj);    
    //bool contract_regex(subject<PARAGRAPH>& subj);
    
  private:

    const static std::set<model_name> dependencies;

    std::vector<pcre2_expr> exprs;
    
    std::filesystem::path asset_file;
    std::filesystem::path model_file;

    nlohmann::json assets;

    std::map<std::string, index_type> h2j; // headers to column index
    std::map<std::string, std::vector<index_type> > s2c={}; // subtype to row-indices

    //base_rgx_model rgx_model;
  };

  const std::set<model_name> nlp_model<ENT, GEOLOC>::dependencies = {};
  
  nlp_model<ENT, GEOLOC>::nlp_model():
    exprs({}),
    
    asset_file(get_crf_dir() / "geoloc/rgx_geoloc.json"),
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
    std::vector<std::vector<std::string> > data ={};

    headers = assets.value("headers", headers);
    data = assets.value("data", data);

    h2j={};
    for(index_type j=0; j<headers.size(); j++)
      {
	h2j[headers.at(j)] = j;
      }

    assert(h2j.count("type")==1);
    assert(h2j.count("subtype")==1);
    assert(h2j.count("expression")==1);

    s2c={};
    for(index_type i=0; i<data.size(); i++)
      {
	std::string subtype = data.at(i).at(h2j.at("subtype"));

	if(s2c.count(subtype))
	  {
	    s2c.at(subtype).push_back(i);
	  }
	else
	  {
	    s2c[subtype] = {i};
	  }
      }

    for(auto itr=s2c.begin(); itr!=s2c.end(); itr++)
      {
	auto subtype = itr->first;	
	auto& inds = itr->second;

	std::stringstream ss;

	ss << R"((^|\s))" << "(?<" << subtype << ">";
	for(index_type l=0; l<inds.size(); l++)
	  {
	    if(l+1==inds.size())
	      {
		ss << data.at(inds.at(l)).at(h2j.at("expression"));
	      }
	    else
	      {
		ss << data.at(inds.at(l)).at(h2j.at("expression")) << "|";
	      }
	  }
	ss << ")" << R"(($|\s|\,|.|\:|\;|\?))";

	LOG_S(INFO) << subtype << ": " << ss.str();
	
	pcre2_expr expr(this->get_key(), subtype, ss.str());
	exprs.push_back(expr);
      }

    return (exprs.size()>0);
  }

  bool nlp_model<ENT, GEOLOC>::apply(std::string& text, nlohmann::json& annots)
  {
    LOG_S(ERROR) << __FUNCTION__ << " on text ...";
    return false;
  }
  
  bool nlp_model<ENT, GEOLOC>::apply(subject<PARAGRAPH>& subj)
  {
    LOG_S(ERROR) << __FUNCTION__ << " on paragraph ...";
    
    std::string text = subj.get_text();

    for(auto& expr:exprs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    for(auto& grp:item.groups)
	      {
		if(s2c.count(grp.group_name)==1)
		  {
		    // NOTE: in future, we might need to have individual post-processing
		    // to determine the range.
		    auto char_range = grp.rng;

		    auto ctok_range = subj.get_char_token_range(char_range);
		    auto wtok_range = subj.get_word_token_range(char_range);

		    std::string orig = subj.from_char_range(char_range);
		    std::string name = subj.from_ctok_range(ctok_range);

		    LOG_S(INFO) << "found " << grp.group_name << ": " << name;
		    
		    subj.instances.emplace_back(subj.get_hash(),
						GEOLOC, expr.get_subtype(),
						name, orig, 
						char_range, ctok_range, wtok_range);

		    utils::mask(text, item.rng);
		  }
	      }
	  }
      }

    return update_applied_models(subj);
  }
  
  bool nlp_model<ENT, GEOLOC>::apply(subject<TABLE>& subj)
  {
    return false;
    //return rgx_model.apply(subj);    
  }
  
}

#endif
