//-*-C++-*-

#ifndef ANDROMEDA_MODELS_RELATIONS_VAU_H_
#define ANDROMEDA_MODELS_RELATIONS_VAU_H_

namespace andromeda
{

  template<>
  class nlp_model<REL, VAU>:
    public base_nlp_model
  {
    typedef nlp_model<REL, VAU> this_type;

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return VAU; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply(subject<TABLE>& subj);

  private:

    bool initialise();

  private:

    const static std::set<model_name> dependencies;

    std::filesystem::path units_file;

    std::map<std::string, std::string> units;
    std::map<std::string, std::string> symbols;
    std::map<std::string, std::string> obrackets;
    std::map<std::string, std::string> cbrackets;
    
    //pcre2_expr filter_01, filter_02;
  };

  const std::set<model_name> nlp_model<REL, VAU>::dependencies = {NUMVAL};

  nlp_model<REL, VAU>::nlp_model():
    units_file(get_rgx_dir() / "vau/units.jsonl"),
    units({}),
    symbols({}),
    obrackets({}),
    cbrackets({})
  {
    initialise();
  }

  nlp_model<REL, VAU>::~nlp_model()
  {}

  bool nlp_model<REL, VAU>::initialise()
  {
    std::ifstream ifs(units_file.c_str());
    if (!ifs.is_open())
      {
	LOG_S(ERROR) << "Failed to open file: " << units_file.c_str();
	return false;
      }

    symbols = {
      {"*", "*"},
      {"/", "/"},
      {"^", "^"},
      {"-", "-"}
    };

    for(int l=0; l<10; l++)
      {
	std::string key = std::to_string(l);
	std::string val = std::to_string(l);
	
	symbols[key] = val;
      }

    obrackets = {
      {"{", "("},
      {"(", "("},
    };

    cbrackets = {
      {"}", ")"},
      {")", ")"},
    };
    
    std::string line;
    while (std::getline(ifs, line))
      {
        // Parse JSON from each line
	nlohmann::json data;
        try
	  {
            data = nlohmann::json::parse(line);
	  }
	catch (const std::exception& e)
	  {
	    LOG_S(ERROR) << "Failed to parse JSON: " << e.what();
	    continue;
	  }

	std::string key = data["unit"];
	std::string val = data["unit"];

	units[key] = val;
      }
    ifs.close();
        
    return true;
  }

  bool nlp_model<REL, VAU>::apply(subject<TEXT>& subj)
  {
    auto& wtokens = subj.get_word_tokens();

    //std::vector<base_instances> unit_instances = {};

    auto& instances = subj.get_instances();
    auto& relations = subj.get_relations();
    
    std::size_t instances_len = instances.size();

    for(std::size_t l=0; l<instances_len; l++)
      {
	auto& inst = instances.at(l);
	//auto crng = inst.get_char_range();

	//auto ctok_rng = inst.get_ctok_range();	    
	auto wtok_rng = inst.get_wtok_range();	    

	auto name = inst.get_name();
	auto orig = inst.get_orig();

	if(inst.is_model(NUMVAL) and wtok_rng.at(1)<wtokens.size())
	  {
	    range_type unit_wtok_range = {
	      wtok_rng.at(1),
	      wtok_rng.at(1)
	    };

	    bool found_unit=false;
	    int balance=0;
	    while(true)
	      {
		auto& wtok = wtokens.at(unit_wtok_range.at(1));
		
		std::string word = wtok.get_word();

		auto unit_itr = units.find(word);
		auto s_itr = symbols.find(word);
		auto o_itr = obrackets.find(word);
		auto c_itr = cbrackets.find(word);
		
		if(unit_itr!=units.end() and unit_itr->first==word)
		  {
		    found_unit = true;
		    unit_wtok_range.at(1) += 1;
		  }
		else if(s_itr!=units.end() and s_itr->first==word)
		  {
		    unit_wtok_range.at(1) += 1;
		  }
		else if(o_itr!=units.end() and o_itr->first==word)
		  {
		    balance += 1;
		    unit_wtok_range.at(1) += 1;
		  }
		else if(c_itr!=units.end() and c_itr->first==word and balance>0)
		  {
		    balance -= 1;
		    unit_wtok_range.at(1) += 1;
		  }
		else
		  {
		    break;
		  }
	      }

	    /*
	    while((unit_wtok_range.at(1)-unit_wtok_range.at(0))>0 and balance<0 and found_unit)
	      {
		auto& wtok = wtokens.at(unit_wtok_range.at(1)-1);

		std::string word = wtok.get_word();

		auto c_itr = cbrackets.find(word);
		if(c_itr!=units.end() and c_itr->first==word)
		  {
		    balance += 1;
		    unit_wtok_range.at(1) -= 1;
		  }
		else
		  {
		    break;
		  }
	      }
	    */
	    
	    if((unit_wtok_range.at(1)-unit_wtok_range.at(0))>0 and
	       found_unit and balance==0)
	      {
		range_type unit_char_range = {
		  wtokens.at(unit_wtok_range.at(0)+0).get_rng(0),
		  wtokens.at(unit_wtok_range.at(1)-1).get_rng(1)
		};
		
		std::string orig = subj.from_char_range(unit_char_range);
		std::string name = subj.from_char_range(unit_char_range);
		
		instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				       VAU, "unit",
				       name, orig,
				       unit_char_range, unit_char_range, unit_wtok_range);
		
		auto& unit_inst = instances.back();
		
		relations.emplace_back("has-unit", 1.0, inst, unit_inst);
	      }
	  }
      }
    
    return false;
  }

  bool nlp_model<REL, VAU>::apply(subject<TABLE>& subj)
  {
    
    return false;
  }
  
}

#endif
