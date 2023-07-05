//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_H_

namespace andromeda
{

  template<>
  class subject<DOCUMENT>: public base_subject
  {
  public:

    const static inline std::string provs_lbl = "page-items";    

    const static inline std::string maintext_lbl = "main-text";
    const static inline std::string other_lbl = "other-text";

    const static inline std::string texts_lbl = "texts";
    const static inline std::string tables_lbl = "tables";
    const static inline std::string figures_lbl = "figures";

    const static inline std::string pdforder_lbl = "pdf-order";

    const static inline std::string prov_lbl = "prov";
    const static inline std::string text_lbl = "text";
    const static inline std::string data_lbl = "data";

    const static inline std::string maintext_name_lbl = "name";
    const static inline std::string maintext_type_lbl = "type";

    const static inline std::set<std::string> texts_types = {"title",
							     "subtitle-level-1", "paragraph",
							     "formula", "equation"};
    
    const static inline std::set<std::string> maintext_types = {"title",
								"subtitle-level-1", "paragraph",
								"formula", "equation", 
								"table", "figure"};
    
  public:

    subject();
    ~subject();

    void show();

    void clear();

    nlohmann::json to_json();

    uint64_t get_hash() const { return doc_hash; }

    void show(bool txt=true, bool mdls=false,
              bool ctokens=false, bool wtokens=true,
              bool prps=true, bool insts=true, bool rels=true);

    bool set_data(nlohmann::json& data, bool order_maintext);

    bool set_data(std::filesystem::path filepath,
                  nlohmann::json& data, bool order_maintext);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool finalise();

    void init_provs();
    void show_provs();

  private:

    void set_meta(nlohmann::json& data);
    void set_orig(nlohmann::json& data);
       
    bool is_preprocessed();

    void set_provs();
    void set_paragraphs();
    void set_other();
    void set_tables();
    void set_figures();
    
    bool finalise_properties();
    bool finalise_instances();
    bool finalise_relations();

  public:

    std::filesystem::path filepath;

    std::string doc_name;
    uint64_t doc_hash;

    nlohmann::json orig, dscr;

    std::vector<std::shared_ptr<prov_element> > provs;
    
    std::vector<std::shared_ptr<subject<PARAGRAPH> > > paragraphs;
    std::vector<std::shared_ptr<subject<PARAGRAPH> > > other;

    std::vector<std::shared_ptr<subject<TABLE> > > tables;
    std::vector<std::shared_ptr<subject<FIGURE> > > figures;
  };

  subject<DOCUMENT>::subject():
    base_subject(DOCUMENT),

    filepath("<undef>"),
    
    doc_name(""),
    doc_hash(-1),

    provs(),
    
    paragraphs(),
    other(),
    
    tables(),
    figures()
  {}

  subject<DOCUMENT>::~subject()
  {}

  nlohmann::json subject<DOCUMENT>::to_json()
  {
    nlohmann::json result = orig;
    
    {
      nlohmann::json base = base_subject::to_json();

      for(auto& elem:base.items())
        {
          result[elem.key()] = elem.value();
        }
    }

    // page-items contain everything on the document pages including page-header/footer,
    // title, subtitle, paragraph, table, figure, etc
    {
      nlohmann::json& page_items = result[provs_lbl];
      page_items = nlohmann::json::array({});

      for(auto& prov:provs)
	{
	  //std::string ref = to_dref(*prov);
	  page_items.push_back(prov->to_json());
	}
    }
    
    // the main-text will only have page-item prov and references to other
    // other parts of the document (text, table, figure, page-header/footer, footnotes)
    {
      nlohmann::json& main_text = result[maintext_lbl];
      nlohmann::json& other_text = result[other_lbl];

      main_text = nlohmann::json::array({});
      other_text = nlohmann::json::array({});

      std::set<std::string> paths={};

      for(auto& prov:provs)
	{
	  std::string path = prov->path;
	  
	  if(paths.count(path)==1) // skip items that have already been included
	    {
	      continue;
	    }
	  paths.insert(path);

	  auto item = prov->to_json();
	  {
	    item.erase("page");
	    item.erase("bbox");
	    item.erase("span");
	  }
	  
	  if(maintext_types.count(prov->type))
	    {	      
	      main_text.push_back(item);
	    }
	  else
	    {
	      other_text.push_back(item);
	    }
	}
    }
    
    {
      std::set<std::string> keys
	= { "hash", "orig", "text", "prov", "properties", "word-tokens"};
      //"instances", "relations"};

      auto& texts = result[texts_lbl];
      texts = nlohmann::json::array({});
      
      for(std::size_t l=0; l<paragraphs.size(); l++)
	{
	  if(not paragraphs.at(l)->is_valid())
	    {
	      continue;
	    }

	  nlohmann::json item = nlohmann::json::object({});
	  
	  auto& paragraph = paragraphs.at(l);	  
	  
	  auto _ = paragraph->to_json();
	  for(auto& elem:_.items())
	    {
	      if(keys.count(elem.key()))
		{
		  item[elem.key()] = elem.value();
		}
	    }

	  texts.push_back(item);
	}
    }
    
    {
      std::set<std::string> keys
	= { "hash", "captions", "footnotes", "mentions", "properties"};
      //"instances", "relations"};
      
      for(std::size_t l=0; l<tables.size(); l++)
	{
	  auto& prov = tables.at(l)->provs.at(0);	  

	  std::vector<std::string> parts = utils::split(prov->path, "/");	  
	  
	  std::string base = parts.at(1);
	  std::size_t index = std::stoi(parts.at(2));
	  
	  auto& item = result.at(base).at(index);
	  
	  auto json_tab = tables.at(l)->to_json();
	  for(auto& elem:json_tab.items())
	    {
	      if(keys.count(elem.key()))
		{
		  item[elem.key()] = elem.value();
		}
	    }
	}
    }

    {
      std::set<std::string> keys
	= { "hash",  "captions", "footnotes", "mentions", "properties"};
      //, "instances", "relations"};
      
      for(std::size_t l=0; l<figures.size(); l++)
	{
	  auto& prov = figures.at(l)->provs.at(0);	  

	  std::vector<std::string> parts = utils::split(prov->path, "/");
	  
	  std::string base = parts.at(1);
	  std::size_t index = std::stoi(parts.at(2));
	  
	  auto& item = result.at(base).at(index);
	      
	  auto json_fig = figures.at(l)->to_json();
	  for(auto& elem:json_fig.items())
	    {
	      if(keys.count(elem.key()))
		{
		  item[elem.key()] = elem.value();
		}
	    }
	}
    }
    
    return result;
  }

  void subject<DOCUMENT>::clear()
  {
    base_subject::clear();

    dscr = nlohmann::json::object({});
    orig = nlohmann::json::object({});

    paragraphs.clear();
    other.clear();
    
    tables.clear();
    figures.clear();
  }
  
  void subject<DOCUMENT>::show(bool txt, bool mdls,
                               bool ctok, bool wtok,
                               bool prps, bool insts, bool rels)
  {
    for(auto paragraph:paragraphs)
      {
        paragraph->show(txt,mdls, ctok,wtok, prps,insts,rels);
      }
  }

  bool subject<DOCUMENT>::set_data(std::filesystem::path filepath,
				   nlohmann::json& data,
                                   bool update_maintext)
  {
    this->filepath = filepath;

    return set_data(data, update_maintext);
  }

  bool subject<DOCUMENT>::set_data(nlohmann::json& data, bool update_maintext)
  {
    clear();

    {
      set_meta(data);      
      set_orig(data);
    }

    if(is_preprocessed())
      {
	//LOG_S(WARNING) << "set document ...";

	set_provs();

	set_paragraphs();
	set_other();

	set_tables();
	set_figures();
      }    
    else
      {
	//LOG_S(WARNING) << "pre-processing document ...";

	doc_normalisation<subject<DOCUMENT> > normaliser(*this);
	normaliser.execute();
      }
    
    return true;
  }

  void subject<DOCUMENT>::set_meta(nlohmann::json& data)
  {
    if(data.count("file-info") and
       data["file-info"].count("document-hash"))
      {
        doc_name = data["file-info"].value("document-hash", doc_name);
        doc_hash = utils::to_hash(doc_name);
      }
    else
      {
        LOG_S(WARNING) << "no `file-info.document-hash detected ...`";

        doc_name = filepath.c_str();
        doc_hash = utils::to_hash(doc_name);
      }

    if(data.count("description"))
      {
	dscr = data.at("description");
      }    
  }
  
  void subject<DOCUMENT>::set_orig(nlohmann::json& data)
  {
    orig = data;
  }

  bool subject<DOCUMENT>::is_preprocessed()
  {
    if(orig.count(provs_lbl) and
       orig.count(maintext_lbl) and
       orig.count(other_lbl) and
       orig.count(texts_lbl) and
       orig.count(tables_lbl) and
       orig.count(figures_lbl))
      {
	return true;
      }

    return false;
  }

  void subject<DOCUMENT>::set_provs()
  {
    provs.clear();

    for(ind_type l=0; l<orig.at(provs_lbl).size(); l++)
      {
	const nlohmann::json& item = orig.at(provs_lbl).at(l);
	
	std::shared_ptr<prov_element> ptr
	  = std::make_shared<prov_element>();

	ptr->from_json(item);
	
	provs.push_back(ptr);
      }
  }

  void subject<DOCUMENT>::set_paragraphs()
  {
    paragraphs.clear();

    for(ind_type l=0; l<orig.at(texts_lbl).size(); l++)
      {
	const nlohmann::json& item = orig.at(texts_lbl).at(l);
	
	std::shared_ptr<subject<PARAGRAPH> > ptr
	  = std::make_shared<subject<PARAGRAPH> >();

	ptr->from_json(item);
	
	paragraphs.push_back(ptr);
      }
  }

  void subject<DOCUMENT>::set_other()
  {
    other.clear();

    for(ind_type l=0; l<orig.at(other_lbl).size(); l++)
      {
	const nlohmann::json& item = orig.at(other_lbl).at(l);
	
	std::shared_ptr<subject<PARAGRAPH> > ptr
	  = std::make_shared<subject<PARAGRAPH> >();

	ptr->from_json(item);
	
	other.push_back(ptr);
      }
  }

  void subject<DOCUMENT>::set_tables()
  {
    tables.clear();

    for(ind_type l=0; l<orig.at(tables_lbl).size(); l++)
      {
	const nlohmann::json& item = orig.at(tables_lbl).at(l);
	
	std::shared_ptr<subject<TABLE> > ptr
	  = std::make_shared<subject<TABLE> >();

	ptr->from_json(item);
	
	tables.push_back(ptr);
      }
  }

  void subject<DOCUMENT>::set_figures()
  {
    figures.clear();

    for(ind_type l=0; l<orig.at(figures_lbl).size(); l++)
      {
	const nlohmann::json& item = orig.at(figures_lbl).at(l);
	
	std::shared_ptr<subject<FIGURE> > ptr
	  = std::make_shared<subject<FIGURE> >();

	ptr->from_json(item);
	
	figures.push_back(ptr);
      }
  }

  void subject<DOCUMENT>::show_provs()
  {
    //LOG_S(INFO) << __FUNCTION__;
    std::vector<std::string> headers = prov_element::get_headers();

    std::vector<std::vector<std::string> > rows={};
    for(auto& prov:provs)
      {
        rows.push_back(prov->to_row());
      }

    LOG_S(INFO) << "filepath: " << filepath << "\n\n"
                << utils::to_string(headers, rows, -1);
  }

  bool subject<DOCUMENT>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                                     std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    for(auto& paragraph:paragraphs)
      {
        bool valid = paragraph->set_tokens(char_normaliser, text_normaliser);

        if(not valid)
          {
            LOG_S(WARNING) << __FILE__ << ":" << __FUNCTION__
                           << " --> unvalid text detected in main-text";
          }
      }

    for(auto& table:tables)
      {
        bool valid = table->set_tokens(char_normaliser, text_normaliser);

        if(not valid)
          {
            LOG_S(WARNING) << __FILE__ << ":" << __FUNCTION__
                           << " --> unvalid text detected in table";
          }
      }

    for(auto& figure:figures)
      {
        bool valid = figure->set_tokens(char_normaliser, text_normaliser);

        if(not valid)
          {
            LOG_S(WARNING) << __FILE__ << ":" << __FUNCTION__
                           << " --> unvalid text detected in figure";
          }
      }

    return true;
  }

  bool subject<DOCUMENT>::finalise()
  {
    bool valid_props = finalise_properties();

    bool valid_insts = finalise_instances();

    bool valid_rels = finalise_relations();

    return (valid_props and valid_insts and valid_rels);
  }

  bool subject<DOCUMENT>::finalise_properties()
  {
    std::map<std::string, val_type>                         property_total;
    std::map<std::pair<std::string, std::string>, val_type> property_label_mapping;

    for(auto& paragraph:paragraphs)
      {
        for(auto& prop:paragraph->properties)
          {
            std::string mdl = prop.get_type();
            std::string lbl = prop.get_name();

            val_type conf = prop.get_conf();
            val_type dst = paragraph->dst;

            if(property_total.count(mdl)==1)
              {
                property_total[mdl] += dst;
              }
            else
              {
                property_total[mdl] = dst;
              }

            std::pair<std::string, std::string> key={mdl,lbl};
            if(property_label_mapping.count(key)==1)
              {
                property_label_mapping[key] += dst*conf;
              }
            else
              {
                property_label_mapping[key] = dst*conf;
              }
          }
      }

    properties.clear();
    for(auto itr=property_label_mapping.begin(); itr!=property_label_mapping.end(); itr++)
      {
        std::string mdl = (itr->first).first;
        itr->second /= (property_total.at(mdl));

        base_property prop((itr->first).first, (itr->first).second, itr->second);
        properties.push_back(prop);
      }

    //LOG_S(INFO) << "properties: \n\n" << tabulate(properties);

    std::sort(properties.begin(), properties.end());

    //LOG_S(INFO) << "properties: \n\n" << tabulate(properties);

    for(auto itr=properties.begin(); itr!=properties.end(); )
      {
        auto next = itr;
        next++;

        if(itr==properties.end() or next==properties.end())
          {
            break;
          }
        else if(itr->get_type()==next->get_type())
          {
            properties.erase(next);
          }
        else
          {
            itr++;
          }
      }
    
    return true;
  }

  bool subject<DOCUMENT>::finalise_instances()
  {
    instances.clear();

    for(auto& subj:paragraphs)
      {
        //LOG_S(INFO) << __FUNCTION__ << ": " << subj.instances.size();

        for(auto& ent:subj->instances)
          {
            instances.emplace_back(subj->get_hash(),
                                  subj->get_name(),
                                  subj->get_path(),
                                  ent);
          }
      }
    //LOG_S(INFO) << "total #-insts: " << instances.size();

    for(auto& subj:tables)
      {
        for(auto& ent:subj->instances)
          {
            instances.emplace_back(subj->get_hash(),
                                  subj->get_name(),
                                  subj->get_path(),
                                  ent);
          }

        for(auto& capt:subj->captions)
          {
	    for(auto& ent:capt->instances)
	      {
		instances.emplace_back(capt->get_hash(),
				      capt->get_name(),
				      capt->get_path(),
				      ent);
	      }
	  }
      }

    for(auto& subj:figures)
      {
        for(auto& ent:subj->instances)
          {
            instances.emplace_back(subj->get_hash(),
                                  subj->get_name(),
                                  subj->get_path(),
                                  ent);
          }

        for(auto& capt:subj->captions)
          {
	    for(auto& ent:capt->instances)
	      {
		instances.emplace_back(capt->get_hash(),
				      capt->get_name(),
				      capt->get_path(),
				      ent);
	      }
	  }	
      }

    return true;
  }

  bool subject<DOCUMENT>::finalise_relations()
  {
    relations.clear();

    for(auto& paragraph:paragraphs)
      {
        for(auto& rel:paragraph->relations)
          {
            relations.push_back(rel);
          }
      }

    for(auto& table:tables)
      {
        for(auto& rel:table->relations)
          {
            relations.push_back(rel);
          }
      }

    return true;
  }

}

#endif
