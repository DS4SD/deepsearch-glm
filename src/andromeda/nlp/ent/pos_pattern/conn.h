//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_CONN_H_
#define ANDROMEDA_MODELS_ENTITIES_CONN_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, CONN>:
    public base_nlp_model,
    public base_pos_pattern
  {        
  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }
    
    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return CONN; }

    virtual bool apply(subject<TEXT>& subj);
    virtual bool apply_on_table_data(subject<TABLE>& subj) { return false; }

  private:

    std::vector<pcre2_expr> exprs;
  };
  
  nlp_model<ENT, CONN>::nlp_model():
    exprs({})
  {
    {
      exprs.emplace_back(this->get_key(), "single-conn", R"(((JJ)\{(\d+)\})((IN)\{(\d+)\}))");
    }
    
    {
      exprs.emplace_back(this->get_key(), "single-conn", R"(((DT)\{(\d+)\})*((IN)\{(\d+)\})((DT)\{(\d+)\})*)");
    }

    {
      exprs.emplace_back(this->get_key(), "single-conn", R"(((DT)\{(\d+)\})*((OF)\{(\d+)\})((DT)\{(\d+)\})*)");
    }

    {
      exprs.emplace_back(this->get_key(), "single-conn", R"(((DT)\{(\d+)\})*((TO)\{(\d+)\})((DT)\{(\d+)\})*)");
    }
  }

  nlp_model<ENT, CONN>::~nlp_model()
  {}

  bool nlp_model<ENT, CONN>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    std::vector<typename base_nlp_model::range_type> ranges_01, ranges_02;
    get_ranges(subj, ranges_01, ranges_02);

    std::vector<pcre2_item> chunks;
    get_chunks(subj, exprs, chunks);    

    add_instances(get_name(), subj, ranges_01, ranges_02, chunks);
    
    return update_applied_models(subj);
  }

}

#endif
