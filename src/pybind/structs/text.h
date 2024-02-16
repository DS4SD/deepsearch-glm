//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_DS_STRUCTS_TEXT_H
#define PYBIND_ANDROMEDA_DS_STRUCTS_TEXT_H

namespace andromeda_py
{
  /*
   *
   *
   */
  class ds_text
  {
    typedef andromeda::subject<andromeda::TEXT> subject_type;

  public:

    ds_text();
    ~ds_text();

    nlohmann::json to_json(std::set<std::string> filters);
    bool from_json(nlohmann::json& data);

    std::shared_ptr<andromeda::subject<andromeda::TEXT> > get_ptr() { return subj_ptr; }

    void clear();
    
    bool set_text(const std::string& ctext);
    bool set_type(const std::string& ctype);
    
  private:

    std::shared_ptr<andromeda::subject<andromeda::TEXT> > subj_ptr;
  };

  ds_text::ds_text():
    subj_ptr(std::make_shared<subject_type>())
  {}

  ds_text::~ds_text()
  {}

  nlohmann::json ds_text::to_json(std::set<std::string> filters)
  {
    if(subj_ptr==NULL)
      {
        return nlohmann::json::value_t::null;
      }
    
    return subj_ptr->to_json(filters);
  }

  bool ds_text::from_json(nlohmann::json& data)
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

  void ds_text::clear()
  {
    if(subj_ptr!=NULL)
      {
	subj_ptr->clear();
      }
  }
  
  bool ds_text::set_text(const std::string& ctext)
  {
    if(subj_ptr!=NULL)
      {
	subj_ptr->set_text(ctext);
      }

    return false;
  }

  bool ds_text::set_type(const std::string& ctype)
  {
    return subj_ptr->set_type(ctype);
  }
  
}

#endif
