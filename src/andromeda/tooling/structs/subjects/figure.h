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
    subject(uint64_t dhash, std::shared_ptr<prov_element> prov);
    
    ~subject();

    std::string get_path() const { return (provs.size()>0? (provs.at(0)->path):"#"); }
    
    void clear();

    bool is_valid() { return (base_subject::valid); }
    
    nlohmann::json to_json();

    bool from_json(const nlohmann::json& data);
    
    bool set_data(const nlohmann::json& data) { return true; }

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
		    std::shared_ptr<utils::text_normaliser> text_normaliser);
    
  private:

    void set_hash();
    
  public:

    std::vector<std::shared_ptr<prov_element> > provs;
    
    std::vector<std::shared_ptr<subject<PARAGRAPH> > > captions;
    std::vector<std::shared_ptr<subject<PARAGRAPH> > > footnotes;
    std::vector<std::shared_ptr<subject<PARAGRAPH> > > mentions;
  };

  subject<FIGURE>::subject():
    base_subject(FIGURE),

    provs({}),
    
    captions({}),
    footnotes({}),
    mentions({})
  {}
  
  subject<FIGURE>::subject(uint64_t dhash, std::shared_ptr<prov_element> prov):
    base_subject(dhash, FIGURE),

    provs({prov}),
    
    captions({}),
    footnotes({}),
    mentions({})
  {}
  
  subject<FIGURE>::~subject()
  {}

  void subject<FIGURE>::clear()
  {
    base_subject::clear();

    provs.clear();
    
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
	  _.push_back(caption->to_json());
	}
    }

    {
      nlohmann::json& _ = result[base_subject::footnotes_lbl];
      _ = nlohmann::json::array({});
      
      for(auto& footnote:footnotes)
	{
	  _.push_back(footnote->to_json());
	}
    }

    {
      nlohmann::json& _ = result[base_subject::mentions_lbl];
      _ = nlohmann::json::array({});
      
      for(auto& mention:mentions)
	{
	  _.push_back(mention->to_json());
	}
    }        
    
    return result;
  }
  
  bool subject<FIGURE>::from_json(const nlohmann::json& data)
  {
    {
      set_data(data);
    }
    
    {
      captions.clear();
      for(const nlohmann::json& item:data.at(base_subject::captions_lbl))
	{
	  std::shared_ptr<subject<PARAGRAPH> > ptr
	    = std::make_shared<subject<PARAGRAPH> >();

	  ptr->from_json(item);
	  captions.push_back(ptr);
	}
    }

    {
      footnotes.clear();
      for(const nlohmann::json& item:data.at(base_subject::footnotes_lbl))
	{
	  std::shared_ptr<subject<PARAGRAPH> > ptr
	    = std::make_shared<subject<PARAGRAPH> >();

	  ptr->from_json(item);
	  footnotes.push_back(ptr);
	}
    }
    
    {
      mentions.clear();
      for(const nlohmann::json& item:data.at(base_subject::mentions_lbl))
	{
	  std::shared_ptr<subject<PARAGRAPH> > ptr
	    = std::make_shared<subject<PARAGRAPH> >();

	  ptr->from_json(item);
	  mentions.push_back(ptr);
	}
    }        
    
    return true;
  }
  
  bool subject<FIGURE>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
				   std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    return true;
  }
  
}

#endif
