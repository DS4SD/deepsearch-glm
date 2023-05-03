//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_VERB_H_
#define ANDROMEDA_MODELS_ENTITIES_VERB_H_

namespace andromeda
{

  template<>
  class nlp_model<ENT, VERB>:
    public base_nlp_model,
    public base_pos_pattern
  {
  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return VERB; }

    virtual bool apply(subject<PARAGRAPH>& subj);
    virtual bool apply(subject<TABLE>& subj) { return false; }

    //virtual bool apply(subject<WEBDOC>& subj) { return false; }

  private:
    
    std::vector<pcre2_expr> exprs;
  };

  nlp_model<ENT, VERB>::nlp_model():
    exprs({})
  {
    {
      exprs.emplace_back(this->get_key(), "compound-verb",
			 R"((MD\{(\d+)\})?(VB([A-Z])?\{(\d+)\})+(TO\{(\d+)\}|RB\{(\d+)\}|VB([A-Z])?\{(\d+)\})+)");
    }
    
    {
      exprs.emplace_back(this->get_key(), "single-verb", R"((MD\{(\d+)\})?(VB([A-Z])?\{(\d+)\})+)");
    }
  }

  nlp_model<ENT, VERB>::~nlp_model()
  {}

  bool nlp_model<ENT, VERB>::apply(subject<PARAGRAPH>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    std::vector<range_type > ranges_01, ranges_02;
    get_ranges(subj, ranges_01, ranges_02);

    std::vector<pcre2_item> chunks;
    get_chunks(subj, exprs, chunks);

    add_entities(get_name(), subj, ranges_01, ranges_02, chunks);
    
    return update_applied_models(subj);
  }

}

#endif
