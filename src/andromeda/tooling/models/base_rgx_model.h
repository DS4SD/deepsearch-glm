//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_RGX_MODEL_H_
#define ANDROMEDA_MODELS_BASE_RGX_MODEL_H_

namespace andromeda
{
  class base_rgx_model: public base_nlp_model
  {
  public:

    base_rgx_model();
    virtual ~base_rgx_model();

    /* add expressions */

    bool push_back(std::string type, std::string subtype,
		   std::string expr);
    
    /*   IO   */    
    virtual bool load(std::filesystem::path ifile, bool verbose);
    virtual bool save(std::filesystem::path ofile);

    /*   INFERENCE   */
    
    virtual bool apply(std::string& text, nlohmann::json& annots) { return false; }

    virtual bool apply(subject<TEXT>& subj);// = 0;// { return false; }
    virtual bool apply(subject<TABLE>& subj) { return false; }

    virtual bool apply(subject<DOCUMENT>& subj) { return false; }
    
    /*   TRAIN   */
    virtual bool is_trainable() { return false; }
    virtual bool train(nlohmann::json args) { return true; }

  private:

    nlohmann::json config;

    std::vector<pcre2_expr> exprs;
  };

  base_rgx_model::base_rgx_model():
    config({}),
    exprs({})
  {
    config = nlohmann::json::object();
    
    config["headers"] = {"type", "subtype", "expression"};
    config["data"] = nlohmann::json::array();
  }

  base_rgx_model::~base_rgx_model()
  {}

  bool base_rgx_model::load(std::filesystem::path ifile, bool verbose)
  {
    std::ifstream ifs(ifile.c_str());

    if(ifs)
      {
	ifs >> config;
      }
    else
      {
	LOG_S(ERROR) << __FILE__ << ":" << __LINE__ << "\t"
		     << "could not read from file " << ifile;

	return false;
      }

    exprs.clear();
    for(auto& row:config["data"])
      {
	this->push_back(row.at(0).get<std::string>(),
			row.at(1).get<std::string>(),
			row.at(2).get<std::string>());
      }
    
    return true;        
  }

  bool base_rgx_model::save(std::filesystem::path ofile)
  {
    std::ofstream ofs(ofile.c_str());

    if(ofs)
      {
	ofs << std::setw(2) << config;
      }
    else
      {
	LOG_S(ERROR) << __FILE__ << ":" << __LINE__ << "\t"
		     << "could not write to file " << ofile;

	return false;
      }

    return true;    
  }
  
  bool base_rgx_model::push_back(std::string type, std::string subtype,
				 std::string expr_)
  {
    try
      {
	pcre2_expr expr(type, subtype, expr_);
	exprs.push_back(expr);

	config["data"].push_back({type, subtype, expr_});	
      }
    catch(std::exception& exc)
      {
	LOG_S(ERROR) << __FILE__ << ":" << __LINE__ << "\n"
		     << "\t error-message: " << exc.what() << "\n"
		     << "\t regex-expr: " << expr_;
	return false;
      }

    return true;
  }

  bool base_rgx_model::apply(subject<TEXT>& subj)
  {
    std::string text = subj.get_text();

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
	    
	    subj.instances.emplace_back(subj.get_hash(), TEXT, subj.get_self_ref(),
					this->get_name(), expr.get_subtype(), 
					name, orig,
					char_range, ctok_range, wtok_range);
          }
      }

    return update_applied_models(subj);
  }

}

#endif
