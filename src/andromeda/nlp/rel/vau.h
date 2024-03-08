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
    
    //pcre2_expr filter_01, filter_02;
  };

  const std::set<model_name> nlp_model<REL, VAU>::dependencies = {NUMVAL};

  nlp_model<REL, VAU>::nlp_model():
    units_file(get_rgx_dir() / "vau/units.jsonl"),
    units({})
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
	std::string val = ""; // FIXME

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
	    auto& wtok = wtokens.at(wtok_rng.at(1));

	    range_type unit_wtok_range = {wtok_rng.at(1), wtok_rng.at(1)};
	    	      
	    std::string word = wtok.get_word();
	    auto itr = units.find(word);
	    
	    if(itr!=units.end() and itr->first==word)
	      {
		unit_wtok_range.at(1) += 1;
	      }

	    if((unit_wtok_range.at(1)-unit_wtok_range.at(0))>0)
	      {
		range_type unit_char_range = {
		  wtokens.at(unit_wtok_range.at(0)).get_rng(0),
		  wtokens.at(unit_wtok_range.at(1)).get_rng(1)
		};
		
		std::string orig = subj.from_char_range(unit_char_range);
		std::string name = subj.from_char_range(unit_char_range);
		
		instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				       VAU, "__unit__",
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
