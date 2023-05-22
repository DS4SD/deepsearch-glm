//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_FIGURE_H_
#define ANDROMEDA_SUBJECTS_FIGURE_H_

namespace andromeda
{
  
  template<>
  class subject<FIGURE>: public base_subject
  {
  public:

    subject();
    subject(uint64_t dhash, prov_element& prov);
    
    ~subject();

    void clear();

    bool is_valid() { return (base_subject::valid); }
    
    nlohmann::json to_json();
    bool from_json(const nlohmann::json& data);
    
    bool set_data(nlohmann::json& data) { return true; }

  private:

    void set_hash();
    
  public:

    //bool valid;
    //uint64_t hash;
    
    //uint64_t dhash;
    //uint64_t index;

    std::vector<subject<PARAGRAPH> > captions;
    std::vector<subject<PARAGRAPH> > footnotes;
    
    //std::set<std::string> applied_models;
    
    //std::vector<base_property> properties;
    //std::vector<base_entity> entities;
    //std::vector<base_relation> relations;
    
  };

  subject<FIGURE>::subject():
    base_subject(),
    //valid(false),

    //hash(-1),
    //dhash(-1),
    //index(-1),

    captions({}),
    footnotes({})//,

    //applied_models(),
    
    //properties({}),
    //entities({}),
    //relations({})
  {}
  
  subject<FIGURE>::subject(uint64_t dhash, prov_element& prov):
    base_subject(dhash, prov),
    //valid(false),

    //hash(-1),
    //dhash(dhash),
    //index(index),

    captions({}),
    footnotes({})//,
    
    //applied_models(),
    
    //properties({}),
    //entities({}),
    //relations({})
  {}
  
  subject<FIGURE>::~subject()
  {}

  void subject<FIGURE>::clear()
  {
    base_subject::clear();
    //valid = false;

    //hash=-1;
    //dhash=-1;
    //index=-1;

    captions.clear();
    footnotes.clear();

    //applied_models.clear();
    
    //properties.clear();
    //entities.clear();
    //relations.clear();
  }
  
}

#endif
