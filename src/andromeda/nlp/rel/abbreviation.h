//-*-C++-*-

#ifndef ANDROMEDA_MODELS_RELATIONS_ABBREVIATION_H_
#define ANDROMEDA_MODELS_RELATIONS_ABBREVIATION_H_

namespace andromeda
{

  template<>
  class nlp_model<REL, ABBREVIATION>:
    public base_nlp_model
  {
    typedef nlp_model<REL, ABBREVIATION> this_type;
    
  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return ABBREVIATION; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply(subject<TABLE>& subj) { return false; }

  private:

    bool initialise();

    void find_abbreviation_instances(subject<TEXT>& subj);

    void find_abbreviation_relations(subject<TEXT>& subj);
    
  private:

    const static std::set<model_name> dependencies;

    pcre2_expr filter_01, filter_02;
  };

  const std::set<model_name> nlp_model<REL, ABBREVIATION>::dependencies = {NAME, TERM};

  nlp_model<REL, ABBREVIATION>::nlp_model():
    filter_01(this->get_key(), "lower-case term", R"([a-z\-\\]+)"),
    filter_02(this->get_key(), "number", R"([\d\-\.\,]+)")
  {
    initialise();
  }

  nlp_model<REL, ABBREVIATION>::~nlp_model()
  {}

  bool nlp_model<REL, ABBREVIATION>::initialise()
  {
    return true;
  }

  bool nlp_model<REL, ABBREVIATION>::apply(subject<TEXT>& subj)
  {
    //LOG_S(INFO) << __FUNCTION__;
    
    if(not satisfies_dependencies(subj))
      {
	return false;
      }
    
    find_abbreviation_instances(subj);

    find_abbreviation_relations(subj);
    
    return update_applied_models(subj);
  }

  void nlp_model<REL, ABBREVIATION>::find_abbreviation_instances(subject<TEXT>& subj)
  {  
    std::string text = subj.get_text();

    //std::size_t max_id = subj.get_max_ent_hash();
    
    for(auto& ent_j:subj.instances)
      {
	auto crng = ent_j.get_char_range();

	auto ctok_rng = ent_j.get_ctok_range();	    
	auto wtok_rng = ent_j.get_wtok_range();	    

	auto name = ent_j.get_name();
	auto orig = ent_j.get_orig();
	
	if(ent_j.is_model(TERM) and
	   0<crng[0] and crng[1]<text.size() and 
	   text[crng[0]-1]=='(' and text[crng[1]]==')' and // preceded and postceded by bracket
	   orig.find(" ")==std::string::npos and // no spaces
	   (not filter_01.match(orig)) and  // no all lower-case words
	   (not filter_02.match(orig))) // no numbers
	  {
	    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
					ABBREVIATION, ent_j.get_subtype(),
					//ent_j.get_name(), ent_j.get_orig(),
					name, orig, crng, ctok_rng, wtok_rng);
	  }
      }    
  }
  
  void nlp_model<REL, ABBREVIATION>::find_abbreviation_relations(subject<TEXT>& subj)
  {
    std::map<std::size_t, std::size_t> i1_to_name, i1_to_term, i0_to_abbr;
    
    for(std::size_t l=0; l<subj.instances.size(); l++)
      {
	auto& ent_j = subj.instances.at(l);
	
	if(ent_j.is_model(ABBREVIATION))
	  {
	    i0_to_abbr[ent_j.get_wtok_range(1)] = l;
	  }
	else if(ent_j.is_model(NAME))
	  {
	    i1_to_name[ent_j.get_wtok_range(1)] = l;
	  }
	else if(ent_j.is_model(TERM))
	  {
	    i1_to_term[ent_j.get_wtok_range(1)] = l;
	  }
      }

    for(auto itr=i0_to_abbr.begin(); itr!=i0_to_abbr.end(); itr++)
      {
	std::size_t i0 = itr->first;

	if(i1_to_name.count(i0-2)==1)
	  {
	    auto& ent_i = subj.instances.at(itr->second);
	    auto& ent_j = subj.instances.at(i1_to_name.at(i0-2));

	    subj.relations.emplace_back("from-abbreviation", 1.0, ent_i, ent_j);
	    subj.relations.emplace_back("to-abbreviation", 1.0, ent_j, ent_i);
	  }
	else if(i1_to_term.count(i0-2)==1)
	  {
	    auto& ent_i = subj.instances.at(itr->second);
	    auto& ent_j = subj.instances.at(i1_to_term.at(i0-2));

	    subj.relations.emplace_back("from-abbreviation", 1.0, ent_i, ent_j);
	    subj.relations.emplace_back("to-abbreviation", 1.0, ent_j, ent_i);	    
	  }
	else
	  {
	    // skip
	  }
      }
    
  }
  
}

#endif
