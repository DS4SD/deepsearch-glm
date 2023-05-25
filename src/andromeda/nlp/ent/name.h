//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_NAME_H_
#define ANDROMEDA_MODELS_ENTITIES_NAME_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, NAME>:
    public fasttext_supervised_model
  {

  public:

    nlp_model(std::filesystem::path resources_dir);
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return NAME; }

    virtual bool apply(subject<PARAGRAPH>& subj);
    virtual bool apply(subject<TABLE>& subj) { return false; }
    
  private:
    
    bool initialise(std::filesystem::path resources_dir);

    bool apply_regex(subject<PARAGRAPH>& subj);
    
    bool post_process(nlohmann::json& ents);
    
  private:

    const static std::set<model_name> dependencies;
    
    std::vector<pcre2_expr> exprs;
  };

  const std::set<model_name> nlp_model<ENT, NAME>::dependencies = {};
  
  nlp_model<ENT, NAME>::nlp_model(std::filesystem::path resources_dir)
  {
    initialise(resources_dir);
  }

  nlp_model<ENT, NAME>::~nlp_model()
  {}

  bool nlp_model<ENT, NAME>::initialise(std::filesystem::path resources_dir)
  {
    {
      std::filesystem::path file = resources_dir / "models/fasttext/person-name/person-classification.bin";
      fasttext_supervised_model::load(file);
    }

    // `Jan H. Wernick`
    {
      pcre2_expr expr(this->get_key(), "person-name",
		      R"((?P<name>(([A-Z][a-z]+)\s+)+(([A-Z]\.)\s+)+([A-Z][a-z]+)+))");
      exprs.push_back(expr);
    }

    // `J. E. Kunzler`
    {
      pcre2_expr expr(this->get_key(), "person-name",
		      R"((?P<name>(([A-Z][a-z]+)\s+)*(([A-Z]\.)\s+){1,}([A-Z][a-z]+)+))");
      exprs.push_back(expr);
    }

    // `Phys Rev B`
    {
      pcre2_expr expr(this->get_key(), "abbreviation-name",
		      R"((?P<name>(([A-Z][a-z]{0,3}\.\s+){1,2}([A-Z][a-z]{0,3}\.))))");
      exprs.push_back(expr);
    }
    
    // `Belle et al.`
    {
      pcre2_expr expr(this->get_key(), "person-group",
		      R"((?P<name>(([A-Z][a-z]+)\s+)(et\.?\s+al\.?)))");
      exprs.push_back(expr);
    }    

    // `Bose-Hubbard` or `Bose- Hubbard` or `Bose -Hubbard`
    {
      pcre2_expr expr(this->get_key(), "name-concatenation",
		      R"((?P<name>(([A-Z][a-z]+)(\-|\s+\-|\-\s+|\s+\-\s+)+)+([A-Z][a-z]+)))");
      exprs.push_back(expr);
    }
    
    return true;
  }

  bool nlp_model<ENT, NAME>::apply(subject<PARAGRAPH>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    apply_regex(subj);

    //subj.contract_wtokens_from_entities(NAME);

    return true;
  }
  
  bool nlp_model<ENT, NAME>::apply_regex(subject<PARAGRAPH>& subj)
  {    
    std::string text = subj.text;
    for(auto& expr:exprs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    for(auto& grp:item.groups)
	      {
		if(grp.group_name=="name")
		  {
		    // NOTE: in future, we might need to have individual post-processing
		    // to determine the range.

		    auto char_range = grp.rng;

		    auto ctok_range = subj.get_char_token_range(char_range);
		    auto wtok_range = subj.get_word_token_range(char_range);

		    std::string orig = subj.from_char_range(char_range);
		    std::string name = subj.from_ctok_range(ctok_range);
		    
		    if(expr.get_subtype()=="name-concatenation")
		      {
			name = utils::replace(name, " ", "");
			name = utils::replace(name, "--", "-");
		      }
		    else
		      {
			name = utils::replace(name, ".", " ");
			name = utils::replace(name, "  ", " ");			
		      }
		    
		    name = utils::strip(name);

		    bool keep=true;
		    if(expr.get_subtype()=="person-name")
		      {
			double conf=1.e-3;
			std::string label="undef";
			
			if(this->classify(name, label, conf))
			  {
			    if(label=="expr" or conf<0.9)
			      {
				keep = false;
			      }
			  }
			
			//LOG_S(WARNING) << name << " -> " << label << ", " << conf << " -> " << keep;
		      }

		    if(keep)
		      {
			subj.entities.emplace_back(subj.get_hash(),
						   NAME, expr.get_subtype(),
						   name, orig, 
						   char_range, ctok_range, wtok_range);
		      }
		    else
		      {
			//LOG_S(WARNING) << "skipping: " << name << " (" << orig << ")";
		      }
		    
		    utils::mask(text, item.rng);
		  }
	      }
	  }
      }
    
    return update_applied_models(subj);
  }
  
}

#endif
