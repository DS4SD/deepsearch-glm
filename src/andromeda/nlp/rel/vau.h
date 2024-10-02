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
    std::map<std::string, std::string> numbers;
    std::map<std::string, std::string> symbols;
    std::map<std::string, std::string> obrackets;
    std::map<std::string, std::string> cbrackets;
  };

  const std::set<model_name> nlp_model<REL, VAU>::dependencies = {NUMVAL};

  nlp_model<REL, VAU>::nlp_model():
    units_file(get_rgx_dir() / "vau/units.jsonl"),
    units({}),
    numbers({}),
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
	// LOG_S(ERROR) << "Failed to open file: " << units_file.c_str();
	LOG_S(ERROR) << "Failed to open file: " << units_file.string();

	
	return false;
      }

    symbols = {
      {"*", "*"},
      {"/", "/"},
      {"\\", "\\"},
      {"^", "^"},
      {"-", "-"},
      {"$", "$"}
    };

    numbers = {};
    for(int l=0; l<10; l++)
      {
	std::string key = std::to_string(l);
	std::string val = std::to_string(l);
	
	numbers[key] = val;
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
	    // LOG_S(ERROR) << "Failed to parse JSON: " << e.what();
		LOG_S(ERROR) << "Failed to parse JSON: " << std::string(e.what());
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
    //subj.show();

    std::string text = subj.get_text();
    
    auto& wtokens = subj.get_word_tokens();

    auto& instances = subj.get_instances();
    auto& relations = subj.get_relations();
    
    std::vector<base_instance> unit_instances={};
    
    for(std::size_t l=0; l<instances.size(); l++)
      {
	auto& inst = instances.at(l);

	auto wtok_rng = inst.get_wtok_range();	    

	bool keep = true;
	auto i0 = wtok_rng.at(0);
		
	if(i0>0 and
	   'a'<=text.at(i0-1) and text.at(i0-1)<='z' and
	   'A'<=text.at(i0-1) and text.at(i0-1)<='Z')
	  {
	    keep = false;
	  }
	
	if(keep and inst.is_model(NUMVAL) and wtok_rng.at(1)<wtokens.size())
	  {
	    range_type unit_wtok_range = {
	      wtok_rng.at(1),
	      wtok_rng.at(1)
	    };

	    bool found_unit=false;
	    int balance=0;
	    while(unit_wtok_range.at(1)<wtokens.size())
	      {
		auto& wtok = wtokens.at(unit_wtok_range.at(1));
		
		std::string word = wtok.get_word();

		auto unit_itr = units.find(word);
		auto n_itr = numbers.find(word);
		auto s_itr = symbols.find(word);
		auto o_itr = obrackets.find(word);
		auto c_itr = cbrackets.find(word);
		
		if(unit_itr!=units.end() and unit_itr->first==word)
		  {
		    found_unit = true;
		    unit_wtok_range.at(1) += 1;
		  }
		else if(n_itr!=numbers.end() and n_itr->first==word)
		  {
		    unit_wtok_range.at(1) += 1;
		  }
		else if(s_itr!=symbols.end() and s_itr->first==word)
		  {
		    unit_wtok_range.at(1) += 1;
		  }
		else if(o_itr!=obrackets.end() and o_itr->first==word)
		  {
		    balance += 1;
		    unit_wtok_range.at(1) += 1;
		  }
		else if(c_itr!=cbrackets.end() and c_itr->first==word and balance>0)
		  {
		    balance -= 1;
		    unit_wtok_range.at(1) += 1;
		  }
		else
		  {
		    break;
		  }

		//LOG_S(INFO) << "adding " << word;
	      }
	    
	    // omit trailing open brackets ...
	    while((unit_wtok_range.at(1)-unit_wtok_range.at(0))>0 and balance>0)
	      {
		auto& wtok = wtokens.at(unit_wtok_range.at(1)-1);

		std::string word = wtok.get_word();
		auto o_itr = obrackets.find(word);
		
		if(o_itr!=obrackets.end() and o_itr->first==word)
		  {
		    balance -= 1;
		    unit_wtok_range.at(1) -= 1;
		  }
		else
		  {
		    break;
		  }
	      }
	    
	    // omit trailing symbols ...
	    while((unit_wtok_range.at(1)-unit_wtok_range.at(0))>0)
	      {
		auto& wtok = wtokens.at(unit_wtok_range.at(1)-1);

		std::string word = wtok.get_word();
		auto s_itr = symbols.find(word);
		
		if(s_itr!=symbols.end() and s_itr->first==word and s_itr->first!="$")
		  {
		    unit_wtok_range.at(1) -= 1;
		  }
		else
		  {
		    break;
		  }
	      }

	    if((unit_wtok_range.at(1)-unit_wtok_range.at(0))>0 and
	       found_unit and balance==0)
	      {
		range_type unit_char_range = {
		  wtokens.at(unit_wtok_range.at(0)+0).get_rng(0),
		  wtokens.at(unit_wtok_range.at(1)-1).get_rng(1)
		};
		
		std::string orig = subj.from_char_range(unit_char_range);
		std::string name = subj.from_char_range(unit_char_range);

		//LOG_S(WARNING) << "keeping unit: " << orig;

		// normalisation
		{
		  name = utils::replace(name, "{", "(");
		  name = utils::replace(name, "}", ")");
		  name = utils::replace(name, "\\", "");

		  std::vector<std::string> start = {"-", "$"};
		  while(true)
		    {
		      bool updated=false;
		      for(std::string c:start)
			{
			  if(name.starts_with(c))
			    {
			      name = name.substr(c.size(), name.size()-c.size());
			      name = utils::strip(name);

			      updated = true;
			    }
			}

		      if(updated)
			{
			  break;
			}
		    }

		  std::vector<std::string> endings = {"-", "$"};
		  while(true)
		    {
		      bool updated=false;
		      for(std::string c:endings)
			{
			  if(name.ends_with(c))
			    {
			      name = name.substr(0, name.size()-c.size());
			      name = utils::strip(name);

			      updated = true;
			    }
			}

		      if(updated)
			{
			  break;
			}
		    }
		}
		
		auto& unit_inst = unit_instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
							      VAU, "unit",
							      name, orig,
							      unit_char_range, unit_char_range, unit_wtok_range);
		
		relations.emplace_back("has-unit", 1.0, inst, unit_inst);
	      }
	    /*
	    else if((unit_wtok_range.at(1)-unit_wtok_range.at(0))>0)
	      {
		range_type unit_char_range = {
		  wtokens.at(unit_wtok_range.at(0)+0).get_rng(0),
		  wtokens.at(unit_wtok_range.at(1)-1).get_rng(1)
		};
		
		std::string orig = subj.from_char_range(unit_char_range);

		LOG_S(WARNING) << "ignoring unit: " << orig;
	      }
	    */
	    else
	      {}
	  }
      }

    //LOG_S(INFO) << "adding relations";
    for(auto& unit_inst:unit_instances)
      {
	instances.push_back(unit_inst);
      }
    
    return true;
  }

  bool nlp_model<REL, VAU>::apply(subject<TABLE>& subj)
  {
    
    return false;
  }
  
}

#endif
