//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_NLP_STRUCTS_DOCUMENT_H
#define PYBIND_ANDROMEDA_NLP_STRUCTS_DOCUMENT_H

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
  class nlp_document
  {
    typedef andromeda::subject<andromeda::DOCUMENT> subject_type;

  public:

    nlp_document();
    ~nlp_document();

    nlohmann::json to_json(std::set<std::string> filters={});
    bool from_json(nlohmann::json& data);

    std::shared_ptr<subject_type> get_ptr() { return subj_ptr; }
    
    void clear();

    bool append_text(nlp_text& subj);
    bool append_table(nlp_table& subj) { return false; }

  private:

    static std::string generate_random_name(int length=64);
    
  private:

    std::shared_ptr<andromeda::subject<andromeda::DOCUMENT> > subj_ptr;
  };

  nlp_document::nlp_document():
    subj_ptr(std::make_shared<subject_type>())
  {
    std::string name = generate_random_name(64);
    subj_ptr->set_name(name);
  }

  nlp_document::~nlp_document()
  {}

  std::string nlp_document::generate_random_name(int length)
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
  
  nlohmann::json nlp_document::to_json(std::set<std::string> filters)
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
