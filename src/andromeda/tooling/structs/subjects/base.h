//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_BASE_SUBJECT_H
#define ANDROMEDA_SUBJECTS_BASE_SUBJECT_H

namespace andromeda
{

  class base_subject: public base_types
  {
  public:

    base_subject();
    base_subject(uint64_t dhash, prov_element& prov);
    
    void clear();

    void clear_models();

  public:

    bool valid;

    uint64_t hash;
    uint64_t dhash;

    std::vector<prov_element> provs;

    std::set<std::string> applied_models;

    std::vector<base_property> properties;
    std::vector<base_entity> entities;
    std::vector<base_relation> relations;
  };

  base_subject::base_subject():
    valid(false),

    hash(-1),
    dhash(-1),
    
    provs({}),

    applied_models({}),

    properties({}),
    entities({}),
    relations({})
  {}

  base_subject::base_subject(uint64_t dhash, prov_element& prov):
    valid(true),

    hash(-1),
    dhash(dhash),
    
    provs({prov}),

    applied_models({}),

    properties({}),
    entities({}),
    relations({})
  {}

  void base_subject::clear()
  {
    valid = false;

    hash = -1;
    dhash = -1;

    provs.clear();

    clear_models();
  }
  
  void base_subject::clear_models()
  {    
    applied_models.clear();
    
    properties.clear();
    entities.clear();
    relations.clear();
  }
  
}

#endif
