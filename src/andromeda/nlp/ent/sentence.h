//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_SENTENCE_H_
#define ANDROMEDA_MODELS_ENTITIES_SENTENCE_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, SENTENCE>: public base_nlp_model
  {

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return SENTENCE; }

    virtual bool apply(subject<TEXT>& subj);
    //virtual bool apply(subject<TABLE>& subj) { return false; }
    
  private:

    void initialise();

  private:

    const static std::set<model_name> dependencies;
    
    std::vector<pcre2_expr> exprs;
  };

  const std::set<model_name> nlp_model<ENT, SENTENCE>::dependencies = {NAME, LINK, NUMVAL, CITE, QUOTE,
								       EXPRESSION, PARENTHESIS};
  
  nlp_model<ENT, SENTENCE>::nlp_model()
  {
    initialise();
  }

  nlp_model<ENT, SENTENCE>::~nlp_model()
  {}

  void nlp_model<ENT, SENTENCE>::initialise()
  {
    {
      pcre2_expr expr(this->get_key(), "sentence", R"([A-Z]([^\.\?\!]+)(\.|\?|\!))");
      exprs.push_back(expr);
    }
  }

  bool nlp_model<ENT, SENTENCE>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }
    
    std::string text = subj.get_text();
    
    for(auto& ent:subj.instances)
      {
	if(dependencies.count(ent.get_model())==1)
	  {
	    if(ent.is_model(NAME) or
	       ent.is_model(EXPRESSION) or
	       ent.is_model(QUOTE))
	      {
		for(std::size_t i=ent.get_char_range(0); i<ent.get_char_range(1); i++)
		  {
		    if(i==ent.get_char_range(0))
		      {
			text[i]='A';
		      }
		    else
		      {
			text[i]='a';
		      }
		  }
	      }
	    else
	      {
		utils::mask(text, ent.get_char_range());
	      }
	  }
      }
    
    std::string orig = subj.get_text();

    std::vector<range_type> sent_ranges={};
    for(auto& expr:exprs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    range_type char_range = item.rng;

	    range_type ctok_range = subj.get_char_token_range(char_range);
	    range_type wtok_range = subj.get_word_token_range(char_range);

	    // skip sentences with 0 words ...
	    if(wtok_range[0]==wtok_range[1])
	      {
		continue;
	      }
	      
	    std::string sent = orig.substr(char_range[0], char_range[1]-char_range[0]); 
	    
	    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				       SENTENCE, "proper",
				       sent, sent,
				       char_range, ctok_range, wtok_range);

	    sent_ranges.push_back(char_range);
	  }
      }

    std::vector<range_type> ranges={};
    for(auto& rng:sent_ranges)
      {
        if(ranges.size()==0 and rng.at(0)==0)
          {
	    ranges.push_back(rng);
          }
	else if(ranges.size()==0 and rng.at(0)>0)
          {
	    ranges.push_back({0, rng.at(0)});
	    ranges.push_back(rng);
          }
	else if(ranges.back().at(1)==rng.at(0))
          {
	    ranges.push_back(rng);
          }
	else if(ranges.back().at(1)<rng.at(0))
          {
	    ranges.push_back({ranges.back().at(1), rng.at(0)});
	    ranges.push_back(rng);
          }	
      }

    if(ranges.size()>0 and ranges.back().at(1)<text.size())
      {
	ranges.push_back({ranges.back().at(1), text.size()});
      }
    else if(ranges.size()==0 and text.size()>0)
      {
	ranges.push_back({0, text.size()});
      }
	    
    for(auto itr=ranges.begin(); itr!=ranges.end(); )
      {
	bool updated=false;
	for(auto sent_rng:sent_ranges)
	  {
	    if(*itr==sent_rng)
	      {
		itr = ranges.erase(itr);
		updated=true;
	      }
	  }

	if(not updated)
	  {
	    itr++;
	  }
      }

    //LOG_S(WARNING) << "text: " << text;
    //LOG_S(WARNING) << "text-size: " << text.size() << "; subj.len: " << subj.get_len();
    
    for(auto rng:ranges)
      {
	range_type char_range = rng;

	//LOG_S(INFO) << "char (1): " << char_range.at(0) << "-" << char_range.at(1);
	
	while(char_range.at(0)<char_range.at(1) and
	      char_range.at(0)<text.size() and
	      text.at(char_range.at(0))==' ')
	  {
	    char_range.at(0) += 1;
	    //LOG_S(INFO) << " -> char: " << char_range.at(0) << "-" << char_range.at(1);
	  }

	//LOG_S(INFO) << "char (2): " << char_range.at(0) << "-" << char_range.at(1);
	
	while(char_range.at(0)<char_range.at(1) and
	      0<char_range.at(1) and
	      text.at(char_range.at(1)-1)==' ')
	  {
	    char_range.at(1) -= 1;
	    //LOG_S(INFO) << " -> char: " << char_range.at(0) << "-" << char_range.at(1);
	  }

	//LOG_S(INFO) << "char (3): " << char_range.at(0) << "-" << char_range.at(1);
	
	if(char_range.at(0)==char_range.at(1))
	  {
	    continue;
	  }
	
	range_type ctok_range = subj.get_char_token_range(char_range);
	range_type wtok_range = subj.get_word_token_range(char_range);

	// skip sentences with 0 words ...
	if(wtok_range[0]==wtok_range[1])
	  {
	    continue;
	  }
	
	std::string sent = orig.substr(char_range[0], char_range[1]-char_range[0]); 

	//LOG_S(WARNING) << " => sent: " << sent;
	
	subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				    SENTENCE, "improper",
				    sent, sent,
				    char_range, ctok_range, wtok_range);
      }
    
    return update_applied_models(subj);
  }

}

#endif
