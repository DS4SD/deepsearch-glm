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
    subject(uint64_t dhash, std::string dloc);
    subject(uint64_t dhash, std::string dloc,
	    std::shared_ptr<prov_element> prov);
    
    virtual ~subject();

    void clear();

    virtual nlohmann::json to_json(const std::set<std::string>& filters);

    virtual bool from_json(const nlohmann::json& data);
    virtual bool from_json(const nlohmann::json& item,
			   const std::vector<std::shared_ptr<prov_element> >& doc_provs);
    
    //std::string get_path() const { return (provs.size()>0? (provs.at(0)->get_item_ref()):"#"); }

    bool is_valid() { return (base_subject::valid); }
    
    bool set_data(const nlohmann::json& data);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
		    std::shared_ptr<utils::text_normaliser> text_normaliser);
    
  private:

    void set_hash();
    
  private:

    sval_type conf;
    std::string created_by;

  public:
    
    std::vector<std::shared_ptr<prov_element> > provs;
    
    std::vector<std::shared_ptr<subject<TEXT> > > captions;
    std::vector<std::shared_ptr<subject<TEXT> > > footnotes;
    std::vector<std::shared_ptr<subject<TEXT> > > mentions;
  };

  subject<FIGURE>::subject():
    base_subject(FIGURE),

    conf(0.0),
    created_by("unknown"),
    
    provs({}),
    
    captions({}),
    footnotes({}),
    mentions({})
  {}

  subject<FIGURE>::subject(uint64_t dhash, std::string dloc):
    base_subject(dhash, dloc, FIGURE),

    conf(0.0),
    created_by("unknown"),
    
    provs({}),
    
    captions({}),
    footnotes({}),
    mentions({})
  {}
  
  subject<FIGURE>::subject(uint64_t dhash, std::string dloc,
			   std::shared_ptr<prov_element> prov):
    base_subject(dhash, dloc, FIGURE),

    conf(0.0),
    created_by("unknown"),
    
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

  nlohmann::json subject<FIGURE>::to_json(const std::set<std::string>& filters)
  {
    nlohmann::json result = base_subject::_to_json(filters, provs);

    {
      result[base_subject::type_lbl] = "figure";

      result[base_subject::confidence_lbl] = utils::round_conf(conf);
      result[base_subject::created_by_lbl] = created_by;      
    }
    
    if(filters.size()==0 or filters.count(base_subject::captions_lbl))
      {
        base_subject::to_json(result, base_subject::captions_lbl, captions, filters);
      }

    if(filters.size()==0 or filters.count(base_subject::footnotes_lbl))
      {
        base_subject::to_json(result, base_subject::footnotes_lbl, footnotes, filters);
      }

    if(filters.size()==0 or filters.count(base_subject::mentions_lbl))
      {
        base_subject::to_json(result, base_subject::mentions_lbl, mentions, filters);
      }
    
    return result;
  }
  
  bool subject<FIGURE>::from_json(const nlohmann::json& json_figure)
  {
    {
      base_subject::valid = true;
      base_subject::_from_json(json_figure);
    }
    
    {
      conf = json_figure.value(base_subject::confidence_lbl, conf);
      created_by = json_figure.value(base_subject::created_by_lbl, created_by);
    }
    
    return base_subject::valid;
  }

  bool subject<FIGURE>::from_json(const nlohmann::json& json_figure,
				  const std::vector<std::shared_ptr<prov_element> >& doc_provs)
  {
    bool init_prov = base_subject::set_prov_refs(json_figure, doc_provs, provs);
    
    bool init_figure = this->from_json(json_figure);
    
    base_subject::from_json(json_figure, doc_provs, base_subject::captions_lbl, captions);
    base_subject::from_json(json_figure, doc_provs, base_subject::footnotes_lbl, footnotes);
    base_subject::from_json(json_figure, doc_provs, base_subject::mentions_lbl, mentions);
    
    return (init_figure and init_prov);
  }
  
  bool subject<FIGURE>::set_data(const nlohmann::json& data)
  {
    base_subject::valid = true;

    return base_subject::valid;
  }
  
  bool subject<FIGURE>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
				   std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    valid = true;
    
    for(auto& caption:captions)
      {
	caption->set_tokens(char_normaliser, text_normaliser);
      }

    for(auto& footnote:footnotes)
      {
	footnote->set_tokens(char_normaliser, text_normaliser);
      }

    return true;
  }
  
}

#endif
