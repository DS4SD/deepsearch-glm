//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_NLP_STRUCTS_H
#define PYBIND_ANDROMEDA_NLP_STRUCTS_H

namespace andromeda_py
{
  /*
   *
   *
   */
  class nlp_text
  {
    typedef andromeda::subject<andromeda::TEXT> subject_type;

  public:

    nlp_text();
    ~nlp_text();

    nlohmann::json to_json(std::set<std::string> filters);
    bool from_json(nlohmann::json& data);

    std::shared_ptr<andromeda::subject<andromeda::TEXT> > get_ptr() { return subj_ptr; }

    void clear();
    
    bool set_text(const std::string& ctext);
    
  private:

    std::shared_ptr<andromeda::subject<andromeda::TEXT> > subj_ptr;
  };

  nlp_text::nlp_text():
    subj_ptr(std::make_shared<subject_type>())
  {}

  nlp_text::~nlp_text()
  {}

  nlohmann::json nlp_text::to_json(std::set<std::string> filters)
  {
    if(subj_ptr==NULL)
      {
        return nlohmann::json::value_t::null;
      }
    
    return subj_ptr->to_json(filters);
  }

  bool nlp_text::from_json(nlohmann::json& data)
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

  void nlp_text::clear()
  {
    if(subj_ptr!=NULL)
      {
	subj_ptr->clear();
      }
  }
  
  bool nlp_text::set_text(const std::string& ctext)
  {
    if(subj_ptr!=NULL)
      {
	subj_ptr->set_text(ctext);
      }

    return false;
  }
  
  
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
  
  /*
   *
   *
   */
  class nlp_document
  {
    typedef andromeda::subject<andromeda::DOCUMENT> subject_type;

  public:

    nlp_document();
    ~nlp_document();

    nlohmann::json to_json(const std::set<std::string>& filters={});
    bool from_json(nlohmann::json& data);

    std::shared_ptr<subject_type> get_ptr() { return subj_ptr; }
    
    void clear();

    bool append_text(nlp_text& subj);
    bool append_table(nlp_table& subj) { return false; }
    
  private:

    std::shared_ptr<andromeda::subject<andromeda::DOCUMENT> > subj_ptr;
  };

  nlp_document::nlp_document():
    subj_ptr(std::make_shared<subject_type>())
  {}

  nlp_document::~nlp_document()
  {}

  nlohmann::json nlp_document::to_json(const std::set<std::string>& filters)
  {
    if(subj_ptr==NULL)
      {
        return nlohmann::json::value_t::null;
      }
    
    return subj_ptr->to_json(filters);
  }

  bool nlp_document::from_json(nlohmann::json& data)
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

  void nlp_document::clear()
  {
    if(subj_ptr!=NULL)
      {
	subj_ptr->clear();
      }
  }

  bool nlp_document::append_text(nlp_text& subj)
  {
    return subj_ptr->push_back(subj.get_ptr());
  }
  
}

#endif
