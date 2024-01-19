//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_NLP_STRUCTS_TABLE_H
#define PYBIND_ANDROMEDA_NLP_STRUCTS_TABLE_H

namespace andromeda_py
{

  /*
   *
   *
   */
  class nlp_table
  {
    typedef andromeda::subject<andromeda::TABLE> subject_type;

  public:

    nlp_table();
    ~nlp_table();

    nlohmann::json to_json(const std::set<std::string>& filters={});
    bool from_json(nlohmann::json& data);

    std::shared_ptr<andromeda::subject<andromeda::TABLE> > get_ptr() { return subj_ptr; }
    
    void clear();
    
  private:

    std::shared_ptr<andromeda::subject<andromeda::TABLE> > subj_ptr;
  };

  nlp_table::nlp_table():
    subj_ptr(std::make_shared<subject_type>())
  {}

  nlp_table::~nlp_table()
  {}

  nlohmann::json nlp_table::to_json(const std::set<std::string>& filters)
  {
    if(subj_ptr==NULL)
      {
        return nlohmann::json::value_t::null;
      }
    
    return subj_ptr->to_json(filters);
  }

  bool nlp_table::from_json(nlohmann::json& data)
  {
    if(subj_ptr==NULL)
      {
        subj_ptr = std::make_shared<subject_type>();
      }

    if(not subj_ptr->from_json(data))
      {
        subj_ptr = NULL;
        return false;
      }

    return true;
  }

  void nlp_table::clear()
  {
    if(subj_ptr!=NULL)
      {
	subj_ptr->clear();
      }
  }

}

#endif
