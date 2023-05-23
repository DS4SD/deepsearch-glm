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

    std::vector<subject<PARAGRAPH> > captions;
    std::vector<subject<PARAGRAPH> > footnotes;
    std::vector<subject<PARAGRAPH> > mentions;
  };

  subject<FIGURE>::subject():
    base_subject(FIGURE),

    captions({}),
    footnotes({}),
    mentions({})
  {}
  
  subject<FIGURE>::subject(uint64_t dhash, prov_element& prov):
    base_subject(dhash, FIGURE, prov),

    captions({}),
    footnotes({}),
    mentions({})
  {}
  
  subject<FIGURE>::~subject()
  {}

  void subject<FIGURE>::clear()
  {
    base_subject::clear();

    captions.clear();
    footnotes.clear();
    mentions.clear();
  }

  nlohmann::json subject<FIGURE>::to_json()
  {
    nlohmann::json result = nlohmann::json::object({});

    {
      nlohmann::json& _ = result[base_subject::captions_lbl];
      _ = nlohmann::json::array({});
      
      for(auto& caption:captions)
	{
	  _.push_back(caption.to_json());
	}
    }

    {
      nlohmann::json& _ = result[base_subject::footnotes_lbl];
      _ = nlohmann::json::array({});
      
      for(auto& footnote:footnotes)
	{
	  _.push_back(footnote.to_json());
	}
    }

    {
      nlohmann::json& _ = result[base_subject::mentions_lbl];
      _ = nlohmann::json::array({});
      
      for(auto& mention:mentions)
	{
	  _.push_back(mention.to_json());
	}
    }        
    
    return result;
  }
  
}

#endif
