//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_DS_STRUCTS_DOCUMENT_H
#define PYBIND_ANDROMEDA_DS_STRUCTS_DOCUMENT_H

#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>

namespace andromeda_py
{

  /*
   *
   *
   */
  class ds_document
  {
    typedef andromeda::subject<andromeda::DOCUMENT> subject_type;

  public:

    ds_document();
    ~ds_document();

    nlohmann::json to_json(std::set<std::string> filters={});
    bool from_json(nlohmann::json& data);

    std::shared_ptr<subject_type> get_ptr() { return subj_ptr; }

    void clear();

    void set_title(std::string title);
    void set_abstract(std::vector<std::string> abstract);
    void set_date(std::string date);
    void set_authors(std::vector<std::string>& authors);
    void set_affiliations(std::vector<std::string>& affils);

    void set_advanced(nlohmann::json& advanced);

    bool append_text(ds_text& subj);
    bool append_table(ds_table& subj);

  private:

    static std::string generate_random_name(int length=64);

  private:

    std::shared_ptr<andromeda::subject<andromeda::DOCUMENT> > subj_ptr;
  };

  ds_document::ds_document():
    subj_ptr(std::make_shared<subject_type>())
  {
    std::string name = generate_random_name(64);
    subj_ptr->set_name(name);
  }

  ds_document::~ds_document()
  {}

  std::string ds_document::generate_random_name(int length)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < length; ++i)
      {
        int randomNibble = dis(gen);
        ss << std::hex << randomNibble;
      }

    return ss.str();
  }

  nlohmann::json ds_document::to_json(std::set<std::string> filters)
  {
    if(subj_ptr==NULL)
      {
        return nlohmann::json::value_t::null;
      }

    return subj_ptr->to_json(filters);
  }

  bool ds_document::from_json(nlohmann::json& data)
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

  void ds_document::clear()
  {
    if(subj_ptr!=NULL)
      {
        subj_ptr->clear();
      }
  }

  void ds_document::set_title(std::string title)
  {
    if(subj_ptr==NULL)
      {
        LOG_S(ERROR) << "document is not initialized";
      }

    subj_ptr->set_title(title);
  }

  void ds_document::set_abstract(std::vector<std::string> abstract)
  {
    if(subj_ptr==NULL)
      {
        LOG_S(ERROR) << "document is not initialized";
      }

    subj_ptr->set_abstract(abstract);
  }

  void ds_document::set_date(std::string date)
  {
    if(subj_ptr==NULL)
      {
        LOG_S(ERROR) << "document is not initialized";
      }

    subj_ptr->set_date(date);

  }

  void ds_document::set_authors(std::vector<std::string>& authors)
  {
    if(subj_ptr==NULL)
      {
        LOG_S(ERROR) << "document is not initialized";
      }

    subj_ptr->set_authors(authors);

  }

  void ds_document::set_affiliations(std::vector<std::string>& affils)
  {
    if(subj_ptr==NULL)
      {
        LOG_S(ERROR) << "document is not initialized";
      }

    subj_ptr->set_affiliations(affils);

  }

  void ds_document::set_advanced(nlohmann::json& advanced)
  {
    if(subj_ptr==NULL)
      {
        LOG_S(ERROR) << "document is not initialized";
      }

    subj_ptr->set_advanced(advanced);

  }

  bool ds_document::append_text(ds_text& subj)
  {
    return subj_ptr->push_back(subj.get_ptr());
  }

  bool ds_document::append_table(ds_table& subj)
  {
    return subj_ptr->push_back(subj.get_ptr());
  }

}

#endif
