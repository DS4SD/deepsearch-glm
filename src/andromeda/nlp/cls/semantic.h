//-*-C++-*-

#ifndef ANDROMEDA_MODELS_CLASSIFIERS_SEMANTIC_H_
#define ANDROMEDA_MODELS_CLASSIFIERS_SEMANTIC_H_

namespace andromeda
{

  template<>
  class nlp_model<CLS, SEMANTIC>: public fasttext_supervised_model
  {
    const static model_type type = CLS;
    const static model_name name = SEMANTIC;
    
  public:

    nlp_model();
    nlp_model(std::filesystem::path resources_dir);
    
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return CLS; }
    virtual model_name get_name() { return SEMANTIC; }
    
    template<typename subject_type>  
    bool get(subject_type& subj, base_property& prop);
    
    virtual bool preprocess(const subject<PARAGRAPH>& subj, std::string& text);
    virtual bool preprocess(const subject<TABLE>& subj, std::string& text);

    //virtual bool classify(const std::string& orig, std::string& label, double& conf);
    
    virtual bool apply(subject<PARAGRAPH>& subj);
    virtual bool apply(subject<TABLE>& subj);

    //virtual bool apply(subject<DOCUMENT>& subj);
    
  private:

    void initialise();

    void initialise_regex();
    void initialise_model();

    void get_semantic_mapping();
    
  private:

    const static std::set<model_name> dependencies;

    std::filesystem::path resources_dir;
    std::filesystem::path model_path;    
    
    std::vector<pcre2_expr> author_list, authors;
    //std::vector<pcre2_expr> table_refs, figure_refs;
    std::vector<pcre2_expr> caption_refs;
  };

  const std::set<model_name> nlp_model<CLS, SEMANTIC>::dependencies = {LINK,NUMVAL};

  nlp_model<CLS, SEMANTIC>::nlp_model():
    fasttext_supervised_model(),
    
    resources_dir(andromeda::RESOURCES_DIR / "models/fasttext/semantic"),
    model_path("semantic-latest.bin")
    //model_path("semantic-v02.ftz")
  {
    initialise();    
  }
  
  nlp_model<CLS, SEMANTIC>::nlp_model(std::filesystem::path resources_dir):
    fasttext_supervised_model(),    

    resources_dir(resources_dir / "models/fasttext/semantic"),
    model_path("semantic-latest.bin")
    //model_path("semantic-v02.ftz")
  {
    initialise();
  }

  nlp_model<CLS, SEMANTIC>::~nlp_model()
  {}

  void nlp_model<CLS, SEMANTIC>::initialise()
  {
    initialise_regex();

    initialise_model();
  }
  
  void nlp_model<CLS, SEMANTIC>::initialise_regex()
  {
    // Yinhan Liu , Myle Ott , Naman Goyal , J . S . - A . Du , Mandar Joshi , Danqi Chen , Omer Levy , Mike Lewis , Luke Zettlemoyer , and Veselin Stoyanov 
    {
      std::string authors_str = R"(((?P<author>(([A-Z][a-z]+\s)(([A-Z][a-z]+|[A-Z]\s\.|\-|\')\s)*([A-Z][a-z]+)))\s((\,|and|\&)\s)+)+(?P<lauthor>(([A-Z][a-z]+\s)(([A-Z][a-z]+|[A-Z]\s\.|\-|\')\s)*([A-Z][a-z]+))))";
      
      pcre2_expr expr(this->get_key(), "__author_list__", authors_str);
      author_list.push_back(expr);
    }

    // Y . Liu , M . Ott , N . Goyal , J . S . - A . Du , M . Joshi , D . Chen , O . Levy , M . Lewis , L . Zettlemoyer 
    {
      std::string authors_str = R"(((?P<author>((([A-Z]\s\.|\-)\s)+([A-Z][a-z]+)))\s((\,|\&|and)\s)+)+(?P<lauthor>((([A-Z]\s\.|\-)\s)+([A-Z][a-z]+))))";

      pcre2_expr expr(this->get_key(), "__author_list__", authors_str);
      author_list.push_back(expr);
    }
    
    // __ival__ . Srivastava , R . - K . , Greff , K . & Schmidhuber , J . Highway networks . CoRR e - prints ( __year__ ) . arXiv : __fval__ .
    {
      std::string authors_str = R"((((?P<author>(([A-Z][a-z]+\s)(\,\s)(([A-Z]\s\.|\-)\s)+)))((\,|\&|and)\s))+(?P<lauthor>(([A-Z][a-z]+\s)(\,\s)(([A-Z]\s\.|\-)\s)+)))";

      pcre2_expr expr(this->get_key(), "__author_list__", authors_str);
      author_list.push_back(expr);
    }    
    
    {
      pcre2_expr expr(this->get_key(), "__author__",
		      R"((?P<author>([A-Z][a-z]+)\s\,(\s[A-Z\-]\s\.)+)\s(\,|and|\&))");
      authors.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "__author__",
		      R"((and|\&)\s(?P<author>([A-Z][a-z]+)\s\,\s([A-Z\-]\s\.)+)\s)");
      authors.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "__author__",
		      R"((?P<author>((\s[A-Z\-]\s\.)+\s([A-Z][a-z]+)))\s(\,|and|\&)+)");
      authors.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "__table__",
		      R"(^(?P<table>(Table|TABLE|Tab|TAB))(\s*\.)?(\s*)(?P<index>(__(i|f)val__|[A-Z]))?)");
      //table_refs.push_back(expr);
      caption_refs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "__table__",
		      R"(^(?P<table>(Table|TABLE|Tab|TAB)))");
      //table_refs.push_back(expr);
      caption_refs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "__figure__",
		      R"(^(?P<figure>(Figure|FIGURE|Fig|FIG))(\s*\.)?(\s*)(?P<index>(__(i|f)val__|[A-Z])))");
      //figure_refs.push_back(expr);
      caption_refs.push_back(expr);
    }

    {
      pcre2_expr expr(this->get_key(), "__figure__",
		      R"(^(?P<figure>(Figure|FIGURE|Fig|FIG))(\s*\.)?)");
      //figure_refs.push_back(expr);
      caption_refs.push_back(expr);
    }    
  }

  void nlp_model<CLS, SEMANTIC>::initialise_model()
  {
    std::filesystem::path path = resources_dir / model_path;

    if(not fasttext_supervised_model::load(path))
      {
	LOG_S(FATAL) << "could not load semantic model ...";
      }
  }

  template<typename subject_type>  
  bool nlp_model<CLS, SEMANTIC>::get(subject_type& subj, base_property& property)
  {
    for(auto& prop:subj.properties)
      {
	if(prop.get_type()==get_key())
	  {
	    property = prop; 
	    return true;
	  }
      }
    
    return false;
  }

  bool nlp_model<CLS, SEMANTIC>::preprocess(const subject<PARAGRAPH>& subj, std::string& text)
  {
    assert(authors.size()>0);
    assert(author_list.size()>0);
    //assert(table_refs.size()>0);
    //assert(figure_refs.size()>0);
    assert(caption_refs.size()>0);
    
    auto& wtokens = subj.word_tokens;
    
    std::stringstream ss;
    
    std::size_t MAXLEN = 256;
    for(std::size_t l=0; l<std::min(wtokens.size(), MAXLEN); l++)
      {
	auto& token = wtokens.at(l);	    
	auto tags = token.get_tags();
	
	if(tags.size()>0)
	  {
	    ss << "__" << *(tags.begin()) << "__";		
	  }
	else
	  {
	    std::string text = token.get_word();
	    //
	    
	    ss << text;
	  }
	
	ss << " ";
      }
    
    text = ss.str();
    //LOG_S(INFO) << __FUNCTION__ << " orig: " << text; 
    
    for(auto& expr:author_list)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    //LOG_S(WARNING) << "\t" << item.text;
	    text = utils::replace(text, item.text, "__author_list__");		    		    	    
	  }
	//LOG_S(INFO) << "text: " << text << "\n";
      }

    if(text.find("__author_list__")==std::string::npos)
      {
	//bool found=false;
	for(auto& expr:authors)
	  {
	    std::vector<pcre2_item> items;
	    expr.find_all(text, items);
	    
	    for(auto& item:items)
	      {
		for(auto& grp:item.groups)
		  {
		    if(grp.group_name=="author")
		      {
			//LOG_S(WARNING) << "\t" << grp.text;
			text = utils::replace(text, grp.text, "__author__");		    		    
		      }
		  }
	      }
	  }
      }

    /*
    if(subj.text.starts_with("Tab") or
       subj.text.starts_with("Fig") or
       subj.text.starts_with("TAB") or
       subj.text.starts_with("FIG"))
      {
	if(text.size()<64)
	  std::cout << text << "\n";
	else
	  std::cout << text.substr(0, 64) << "\n";
      }
    */

    for(auto& expr:caption_refs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	for(auto& item:items)
	  {
	    //LOG_S(WARNING) << "\t" << item.text;
	    text = utils::replace(text, item.text, "__caption_ref__");		    		    	    
	  }
	
	//if(items.size()>0)
	///{
	//text = "__caption_ref__";
	//}
	
      }

    /*
    for(auto& expr:table_refs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	if(items.size()>0)
	  {
	    text = "__table_ref__";
	  }

	//for(auto& item:items)
	  {
	    //LOG_S(WARNING) << "detected: " << item.text;
	    //text = utils::replace(text, item.text, "__table_ref__");		    		    	     
	    //text = "__table_ref__";
	      

	    for(auto& grp:item.groups)
	      {
		if(grp.group_name=="table")
		  {	    
		    text = utils::replace(text, grp.text, "__table__");		    		    	   
		  }

		if(grp.group_name=="index")
		  {	    
		    text = utils::replace(text, grp.text, "__table_index__");		    		    	   
		  }		
	      }
	    LOG_S(INFO) << " -> text: " << text << "\n";

	  }
      }
    */
	    
    /*
    for(auto& expr:figure_refs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	if(items.size()>0)
	  {
	    text = "__figure_ref__";
	  }
	
	//for(auto& item:items)
	  {
	    //LOG_S(WARNING) << "detected: " << item.text;
	    //text = utils::replace(text, item.text, "__figure_ref__");		    		    	     
	    
	      

	    for(auto& grp:item.groups)
	      {
		if(grp.group_name=="figure")
		  {	    
		    text = utils::replace(text, grp.text, "__figure__");		    		    	   
		  }

		if(grp.group_name=="index")
		  {	    
		    text = utils::replace(text, grp.text, "__figure_index__");		    		    	   
		  }		
	      }	    
	    LOG_S(INFO) << " -> text: " << text << "\n";

	  }
      }        
    */
    
    //LOG_S(INFO) << __FUNCTION__ << " -> " << text;

    /*
    if(subj.text.starts_with("Tab") or
       subj.text.starts_with("Fig") or
       subj.text.starts_with("TAB") or
       subj.text.starts_with("FIG"))
      {
	if(text.size()<64)
	  std::cout << text << "\n";
	else
	  std::cout << text.substr(0, 64) << "\n";
      }
    */
    
    text = utils::to_lower(text);
    
    return true;
  }

  bool nlp_model<CLS, SEMANTIC>::preprocess(const subject<TABLE>& subj, std::string& text)
  {
    std::stringstream ss;
    for(std::size_t i=0; i<subj.data.size(); i++)
      {
	auto& row = subj.data.at(i);	
	for(std::size_t j=0; j<row.size(); j++)
	  {
	    ss << row.at(j).text << "; ";
	  }
      }

    text = ss.str();

    return true;
  }

  /*
  bool nlp_model<CLS, SEMANTIC>::classify(const std::string& orig, std::string& label, double& conf)
  {
    std::string text = preprocess(orig);
    
    if(text.starts_with("__table_ref__") or
       text.starts_with("__figure_ref__") )
      {
	label = "caption";
	conf = 1.0;

	LOG_S(WARNING) << "hard update for caption ....";
      }    


    std::string text = orig;
    
    for(auto& expr:table_refs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	if(items.size()>0)
	  {
	    label = "caption";
	    conf = 1.0;

	    return true;
	  }
      }
    
    for(auto& expr:figure_refs)
      {
	std::vector<pcre2_item> items;
	expr.find_all(text, items);

	if(items.size()>0)
	  {
	    label = "caption";
	    conf = 1.0;

	    return true;
	  }
      }
    
    return fasttext_supervised_model::classify(orig, label, conf);
  }
  */
  
  bool nlp_model<CLS, SEMANTIC>::apply(subject<PARAGRAPH>& subj)
  {       
    return fasttext_supervised_model::classify(subj);
  }

  bool nlp_model<CLS, SEMANTIC>::apply(subject<TABLE>& subj)
  {
    return fasttext_supervised_model::classify(subj);
  }

  /*
  bool nlp_model<CLS, SEMANTIC>::apply(subject<DOCUMENT>& subj)
  {
    LOG_S(WARNING) << __FUNCTION__ << " on document";
    return false;
  }
  */
  
}

#endif
