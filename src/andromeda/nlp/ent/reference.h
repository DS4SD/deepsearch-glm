//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_REFERENCE_H_
#define ANDROMEDA_MODELS_ENTITIES_REFERENCE_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, REFERENCE>: public base_crf_model
  {
    typedef typename word_token::range_type range_type;
    
    const static inline std::string TAG = "__"+to_string(REFERENCE)+"__";

  public:

    nlp_model();
    nlp_model(std::filesystem::path resources_dir);

    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return REFERENCE; }

    virtual bool apply(subject<PARAGRAPH>& subj);
    virtual bool apply(subject<TABLE>& subj) { return false; }
    virtual bool apply(subject<DOCUMENT>& subj);
    
  private:

    void initialise(std::filesystem::path resources_dir);

    void run_model(subject<PARAGRAPH>& subj);
    
    void post_process(subject<PARAGRAPH>& subj);
    
  private:

    const static std::set<model_name> dependencies;
  };

  const std::set<model_name> nlp_model<ENT, REFERENCE>::dependencies = { SEMANTIC, LINK, NUMVAL};

  nlp_model<ENT, REFERENCE>::nlp_model()
  {}
  
  nlp_model<ENT, REFERENCE>::nlp_model(std::filesystem::path resources_dir)
  {
    initialise(resources_dir);
  }

  nlp_model<ENT, REFERENCE>::~nlp_model()
  {}

  void nlp_model<ENT, REFERENCE>::initialise(std::filesystem::path resources_dir)
  {
    if(not base_crf_model::load(resources_dir / "models/crf/reference/reference-latest.bin", false))
      {
	LOG_S(FATAL) << "could not load REFERENCE model from " << resources_dir;
      }
  }

  bool nlp_model<ENT, REFERENCE>::apply(subject<DOCUMENT>& doc)
  {
    for(auto& paragraph:doc.paragraphs)
      {
	this->apply(paragraph);
      }

    return true;
  }
  
  bool nlp_model<ENT, REFERENCE>::apply(subject<PARAGRAPH>& subj)
  {
    //LOG_S(WARNING) << "reference parsing started ...";
    
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    bool is_ref=false;
    for(auto& cls:subj.properties)
      {
	//LOG_S(INFO) << cls.type << " -> " << (cls.type==to_key(SEMANTIC));
	//LOG_S(INFO) << cls.name << " -> " << (cls.name=="reference");
	
	if(cls.get_type()==to_key(SEMANTIC) and
	   cls.get_name()=="reference")
	  {
	    is_ref = true;
	  }
      }
    
    // text in subject is not a reference and we do not apply the reference parser
    if(not is_ref) 
      {
	//LOG_S(WARNING) << "is not a reference ...";
	return true;
      }
       
    run_model(subj);

    post_process(subj);
    
    return true;
  }

  void nlp_model<ENT, REFERENCE>::run_model(subject<PARAGRAPH>& subj)
  {
    //LOG_S(WARNING) << __FILE__ << ":" << __LINE__;
    
    std::vector<crf_token_type> crf_tokens={};
    std::map<std::size_t, std::size_t> ptid_to_wtid={};

    auto& wtokens = subj.word_tokens;
    //auto& entities = subj.entities;
    
    //pre_process(wtokens, ent.wtok_range, pos_tokens, ptid_to_wtid);

    for(std::size_t l=0; l<wtokens.size(); l++)
      {
	auto& wtoken = wtokens.at(l);

	crf_tokens.emplace_back(wtoken.get_word(),
				wtoken.get_rng(0),
				wtoken.get_rng(1));
      }
    
    base_crf_model::predict(crf_tokens);

    //andromeda_crf::tabulate(crf_tokens);

    /*
    for(std::size_t l=0; l<crf_tokens.size(); l++)
      {
	crf_tokens.at(l).true_label = crf_tokens.at(l).pred_label;
      }

    std::set<std::string> texts={".",",","and"};
    for(std::size_t l=1; l<crf_tokens.size()-1; l++)
      {
	if((crf_tokens.at(l-1).true_label==crf_tokens.at(l+1).true_label) and
	   texts.count(crf_tokens.at(l).text))
	  {
	    crf_tokens.at(l).true_label = crf_tokens.at(l-1).true_label;
	  }
      }
    */
    
    for(std::size_t l=0; l<wtokens.size(); l++)
      {
	auto& wtoken = wtokens.at(l);
	auto& ptoken = crf_tokens.at(l);

	std::string label = TAG + ptoken.pred_label;
	wtoken.set_tag(label);
      }
  }
  
  void nlp_model<ENT, REFERENCE>::post_process(subject<PARAGRAPH>& subj)
  {
    auto& wtokens = subj.word_tokens;

    //std::map<std::string, std::vector<std::array<std::size_t, 2> > > labels_to_crng={};
    std::map<std::string, std::vector<typename word_token::range_type> > labels_to_crng={};
    
    for(std::size_t l=0; l<wtokens.size(); l++)
      {
	auto& wtoken = wtokens.at(l);

	for(auto& wtoken_tag:wtoken.get_tags())
	  {
	    if(wtoken_tag.starts_with(TAG))
	      {
		auto label = utils::replace(wtoken_tag, TAG, "");

		auto itr = labels_to_crng.find(label);

		if(itr==labels_to_crng.end())
		  {
		    labels_to_crng[label]={};
		  }

		labels_to_crng.at(label).push_back(wtoken.get_rng());		    
	      }
	  }
      }

    /*
    for(auto itr=labels_to_crng.begin(); itr!=labels_to_crng.end(); itr++)
      {
	LOG_S(INFO) << itr->first << ": " << (itr->second).size();
	for(auto jtr=(itr->second).begin(); jtr!=(itr->second).end(); jtr++)
	  {
	    LOG_S(INFO) << " -> " << (*jtr)[0] << ", " << (*jtr)[1];
	  }
      }
    */
    
    std::set<std::string> labels
      = { "citation-number",
	  "author", "title",
	  "publisher", "editor",
	  "journal", "container-title",
	  "location", "date",
	  "volume", "pages",
	  "url", "doi"};

    for(const auto& label:labels)
      {       	
	if(labels_to_crng.count(label)==0)
	  {
	    continue;
	  }
	
	auto& ranges = labels_to_crng.at(label);

	std::size_t ind=0;
	while(ind<ranges.size())
	  {
	    range_type char_range = ranges.at(ind++);

	    while(ind<ranges.size())
	      {
		auto rng = ranges.at(ind);
		if(rng[0]-char_range[1]<=1)
		  {
		    char_range[1] = rng[1];
		  }
		else
		  {
		    break;
		  }

		ind += 1;
	      }
	
	    auto ctok_range = subj.get_char_token_range(char_range);
	    auto wtok_range = subj.get_word_token_range(char_range);
	    
	    std::string orig = subj.from_char_range(char_range);
	    std::string name = subj.from_ctok_range(ctok_range);
	    
	    subj.entities.emplace_back(//utils::to_hash(name),
				       REFERENCE, label,
				       name, orig, 
				       char_range, ctok_range, wtok_range);	
	  }
      }

    // delete all non-reference entities
    {
      auto itr=subj.entities.begin();
      while(itr!=subj.entities.end())
	{
	  if(itr->model_type!=REFERENCE)
	    {
	      itr = subj.entities.erase(itr);
	    }
	  else
	    {
	      itr++;
	    }
	}
    }
    
  }
  
}

#endif
