//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_TERM_H_
#define ANDROMEDA_MODELS_ENTITIES_TERM_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, TERM>:
    public base_nlp_model,
    public base_pos_pattern
  {        
  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return TERM; }

    virtual bool apply(subject<TEXT>& subj);

    //virtual bool apply(subject<TABLE>& subj) { LOG_S(WARNING) << "apply nlp_model<ENT, TERM>"; return false; }

    virtual bool apply_on_table_data(subject<TABLE>& subj);
    
  private:

    std::vector<pcre2_expr> enum_exprs;
    
    std::vector<pcre2_expr> single_exprs;
  };

  nlp_model<ENT, TERM>::nlp_model():
    enum_exprs({}),
    single_exprs({})
  {

    /*
    {
      std::string term_1 = R"(((JJ(S)?\{(\d+)\})+(\,\{(\d+)\}))*)";
      std::string term_2 = R"(((JJ(S)?\{(\d+)\})+(\,\{(\d+)\})?(CC\{(\d+)\}))(\,\{(\d+)\})?)";
      std::string term_3 = R"(((JJ(S)?\{(\d+)\})+(NN(S|P)?\{(\d+)\})+))";

      std::string expr = term_1+term_2+term_3;
      exprs.emplace_back(this->get_key(), "enum-term", expr);
    }
    */
    
    /*
    {
      std::string term_1 = R"(((NN(S|P)?\{(\d+)\})+(\,\{(\d+)\}))*)";
      std::string term_2 = R"(((NN(S|P)?\{(\d+)\})+(\,\{(\d+)\})?(CC\{(\d+)\}))(\,\{(\d+)\})?)";
      std::string term_3 = R"(((NN(S|P)?\{(\d+)\})+))";

      std::string expr = term_1+term_2+term_3;
      exprs.emplace_back(this->get_key(), "enum-term", expr);
    }
    */

    // `thermal, magnetic, and, superconducting properties`, `magnetic and superconducting properties`
    {
      std::string term_0 = R"(((JJ(P|S)*\{(\d+)\})+(\,\{(\d+)\}))*)";
      std::string term_1 = R"((JJ(P|S)*\{(\d+)\})+)";
      std::string term_2 = R"((\,\{(\d+)\})?(CC\{(\d+)\})(\,\{(\d+)\})?)";
      std::string term_3 = R"((JJ(P|S)*\{(\d+)\})+(NN(S)?\{(\d+)\}))";

      std::string expr = term_0+term_1+term_2+term_3;
      enum_exprs.emplace_back(this->get_key(), "enum-term-mark-1", expr);
    }

    {
      std::string term_0 = R"(((NN\{(\d+)\})+(\,\{(\d+)\}))*)";
      std::string term_1 = R"((NN\{(\d+)\})+)";
      std::string term_2 = R"((\,\{(\d+)\})?(CC\{(\d+)\})(\,\{(\d+)\})?)";
      std::string term_3 = R"((NN\{(\d+)\})+)";

      std::string expr = term_0+term_1+term_2+term_3;
      enum_exprs.emplace_back(this->get_key(), "enum-term-mark-2", expr);
    }

    {
      std::string term_0 = R"(((NNS\{(\d+)\})+(\,\{(\d+)\}))*)";
      std::string term_1 = R"((NNS\{(\d+)\})+)";
      std::string term_2 = R"((\,\{(\d+)\})?(CC\{(\d+)\})(\,\{(\d+)\})?)";
      std::string term_3 = R"((NNS\{(\d+)\})+)";

      std::string expr = term_0+term_1+term_2+term_3;
      enum_exprs.emplace_back(this->get_key(), "enum-term-mark-3", expr);
    }

    {
      std::string term_0 = R"(((NNP\{(\d+)\})+(\,\{(\d+)\}))*)";
      std::string term_1 = R"((NNP\{(\d+)\})+)";
      std::string term_2 = R"((\,\{(\d+)\})?(CC\{(\d+)\})(\,\{(\d+)\})?)";
      std::string term_3 = R"((NNP\{(\d+)\})+)";

      std::string expr = term_0+term_1+term_2+term_3;
      enum_exprs.emplace_back(this->get_key(), "enum-term-mark-4", expr);
    }        
    
    {
      //single_exprs.emplace_back(this->get_key(), "single-term",
      //R"(((JJ|NN)(S|P|PS)?\{(\d+)\})+(NN(S|P|PS)?\{(\d+)\}))");

      single_exprs.emplace_back(this->get_key(), "single-term",
				R"(((JJ|NN)(S|P|PS)?\{(\d+)\})((JJ|NN|\:)(S|P|PS)?\{(\d+)\})*(NN(S|P|PS)?\{(\d+)\}))");
    }

    /*
    {
      exprs.emplace_back(this->get_key(), "single-term", 
      R"(((VBG|JJ|NN)(S|P)?\{(\d+)\})+(NN(S|P)?\{(\d+)\}))");
    }
    */
    
    {
      single_exprs.emplace_back(this->get_key(), "single-term",
				R"((NN(P|S|PS)?\{(\d+)\})+)");
    }
  }

  nlp_model<ENT, TERM>::~nlp_model()
  {}

  bool nlp_model<ENT, TERM>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj, text_dependencies))
      {
	//LOG_S(WARNING) << "skipping term ...";
	return false;
      }
    
    std::vector<typename base_nlp_model::range_type> ranges_01, ranges_02;
    get_ranges(subj, ranges_01, ranges_02);

    std::vector<pcre2_item> enum_chunks={}, single_chunks={};
    {
      get_chunks(subj, enum_exprs, enum_chunks);    

      get_chunks(subj, single_exprs, single_chunks);
    }

    {
      add_instances(get_name(), subj, ranges_01, ranges_02, enum_chunks);

      add_instances(get_name(), subj, ranges_01, ranges_02, single_chunks);
    }
    
    return update_applied_models(subj);
  }

  bool nlp_model<ENT, TERM>::apply_on_table_data(subject<TABLE>& subj)
  {
    for(std::size_t i=0; i<subj.num_rows(); i++)
      {
	for(std::size_t j=0; j<subj.num_cols(); j++)
	  {	   	    
	    if(subj(i,j).skip())
	      {
		//LOG_S(INFO) << "skipping: " << subj(i,j).get_text();
		continue;
	      }

	    std::vector<typename base_nlp_model::range_type> ranges_01={}, ranges_02={};
	    //get_ranges(subj, ranges_01, ranges_02);
	    
	    std::vector<pcre2_item> single_chunks={};
	    
	    get_chunks(subj(i,j), single_exprs, single_chunks);	    

	    //LOG_S(INFO) << andromeda::tabulate(subj(i,j).get_word_tokens(), subj(i,j).get_text());
	    //LOG_S(INFO) << "chunks: " << single_chunks.size();
	    
	    add_instances(get_name(), subj, subj(i,j).get_coor(),
			  subj(i,j).get_row_span(),
			  subj(i,j).get_col_span(),
			  ranges_01, ranges_02, single_chunks);
	  }
      }
    
    return update_applied_models(subj);
  }
  
}

#endif
