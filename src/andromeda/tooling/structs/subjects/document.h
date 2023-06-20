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
    
  public:

    subject();
    ~subject();

    void show();

    void clear();

    nlohmann::json to_json();

    uint64_t get_hash() const { return doc_hash; }

    void show(bool txt=true, bool mdls=false,
              bool ctokens=false, bool wtokens=true,
              bool prps=true, bool ents=true, bool rels=true);

    bool set_data(nlohmann::json& data, bool order_maintext);

    bool set_data(std::filesystem::path filepath,
                  nlohmann::json& data, bool order_maintext);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool finalise();

    void init_provs();
    void show_provs();

  private:

    void resolve_paths();

    //subject_name to_subject(std::shared_ptr<prov_element>& prov);
    
    //std::string to_dref(const prov_element& prov);
    
    void set_meta(nlohmann::json& data);
    void set_orig(nlohmann::json& data);
    
    bool clean_provs(std::vector<prov_element>& provs); // remove all provs with ignore==true

    bool init_items();
    bool link_items();

    bool remove_headers_and_footers(std::vector<prov_element>& provs);

    bool identify_repeating_text(std::vector<prov_element>& provs);
    
    bool init_tables(std::vector<prov_element>& provs);
    bool init_figures(std::vector<prov_element>& provs);
    bool init_paragraphs(std::vector<prov_element>& provs);

    bool finalise_properties();
    bool finalise_entities();
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

    //captions(),
    //footnotes(),
    
    tables(),
    figures()
  {}

  subject<DOCUMENT>::~subject()
  {}

  /*
  std::string subject<DOCUMENT>::to_dref(const prov_element& prov)
  {    
    std::stringstream ss;

    switch(prov.dref.first)
      {
      case PARAGRAPH:
	{
	  ss << "#" << "/" << flowtext_lbl << "/" << prov.dref.second;
	}
	break;

      case TABLE:
	{
	  ss << "#" << "/" << tables_lbl << "/" << prov.dref.second;
	}
	break;	

      default:
	{
	  ss << prov.to_path();
	}
      }
    
    return ss.str();
  }
  */
  
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

    resolve_paths();
    
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

      std::set<std::string> maintext_types={"title", "subtitle-level-1", "paragraph", "formula",
					    "table", "figure"};
      
      for(auto& prov:provs)
	{
	  std::string path = prov->path;
	  
	  if(paths.count(path))
	    {
	      continue;
	    }
	  paths.insert(path);

	  if(maintext_types.count(prov->type))
	    {
	      main_text.push_back(prov->to_json());
	    }
	  else
	    {
	      other_text.push_back(prov->to_json());
	    }
	}
    }
    
    {
      std::set<std::string> keys
	= { "hash", "orig", "text", "prov", "properties", "word-tokens"};
      //"entities", "relations"};

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
      //"entities", "relations"};
      
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
      //, "entities", "relations"};
      
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

  /*
  std::pair<std::string, index_type> subject<DOCUMENT>::to_path(std::shared_ptr<prov_element>& prov)
  {
    std::vector<std::string> parts = utils::split(prov->path, "/");

    // {'#', "<texts|tables|figures|other>", <index>, }
    index_type ind = std::stoi(parts.at(2));
    
    return {parts.at(1), ind};
  }
  */
  
  void subject<DOCUMENT>::resolve_paths()
  {
    //std::map<std::shared_ptr<prov_element>, std::pair<subject_name, index_type> > to_element={};

    //for(index_type l=0; l<provs.size(); l++)
    //{
    //provs.at(l)->path = "#";
    //}
    
    for(index_type l=0; l<paragraphs.size(); l++)
      {
	for(auto& prov:paragraphs.at(l)->provs)
	  {
	    std::stringstream ss;
	    ss << "#/" << texts_lbl << "/" << l;

	    prov->path = ss.str();
	  }
      }

    for(index_type l=0; l<tables.size(); l++)
      {
	for(auto& prov:tables.at(l)->provs)
	  {
	    std::stringstream ss;
	    ss << "#/" << tables_lbl << "/" << l;

	    prov->path = ss.str();
	  }

	for(index_type k=0; k<tables.at(l)->captions.size(); k++)
	  {
	    for(auto& prov:tables.at(l)->captions.at(k)->provs)
	      {
		std::stringstream ss;
		ss << "#/" << tables_lbl << "/" << l << "/" << captions_lbl << "/" << k;
		
		prov->path = ss.str();
	      }
	  }
      }

    for(index_type l=0; l<figures.size(); l++)
      {
	for(auto& prov:figures.at(l)->provs)
	  {
	    std::stringstream ss;
	    ss << "#/" << figures_lbl << "/" << l;

	    prov->path = ss.str();
	  }

	for(index_type k=0; k<figures.at(l)->captions.size(); k++)
	  {
	    for(auto& prov:figures.at(l)->captions.at(k)->provs)
	      {
		std::stringstream ss;
		ss << "#/" << figures_lbl << "/" << l << "/" << captions_lbl << "/" << k;
		
		prov->path = ss.str();
	      }
	  }	
      }        
  }
  
  void subject<DOCUMENT>::clear()
  {
    base_subject::clear();

    orig = nlohmann::json::object({});

    paragraphs.clear();
    tables.clear();
    figures.clear();
  }
  
  void subject<DOCUMENT>::show(bool txt, bool mdls,
                               bool ctok, bool wtok,
                               bool prps, bool ents, bool rels)
  {
    for(auto paragraph:paragraphs)
      {
        paragraph->show(txt,mdls, ctok,wtok, prps,ents,rels);
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

    {
      init_provs();
    }

    if(update_maintext)
      {
	doc_order sorter;
	sorter.order_maintext(*this);
      }

    {
      init_items();
    }

    {
      link_items();
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

    if(orig.count(maintext_lbl)==0)
      {
	LOG_S(WARNING) << "no `main-text` identified";
	return;
      }

    auto& main_text = orig.at(maintext_lbl);
    for(std::size_t pdforder=0; pdforder<main_text.size(); pdforder++)
      {
	main_text.at(pdforder)[pdforder_lbl] = pdforder;
      }
  }
  
  void subject<DOCUMENT>::init_provs()
  {
    provs.clear();

    for(std::size_t l=0; l<orig[maintext_lbl].size(); l++)
      {
        auto& item = orig[maintext_lbl][l];

	std::stringstream ss;
	ss << "#/" << maintext_lbl << "/" << l;

	std::string ref = ss.str();
	
	ind_type pdforder = item[pdforder_lbl].get<ind_type>();
	ind_type maintext = l;

	std::string name = item[maintext_name_lbl].get<std::string>();
	std::string type = item[maintext_type_lbl].get<std::string>();
	
        if(item.count("$ref") or
	   item.count("__ref"))
          {
	    if(item.count("$ref"))
	      {
		ref = item["$ref"].get<std::string>();	    
	      }
	    else if(item.count("__ref"))
	      {
		ref = item["__ref"].get<std::string>();
	      }
	    else
	      {
		LOG_S(FATAL) << "no `$ref` or `__ref` defined in " << item.dump();
	      }
	    
	    auto prov = std::make_shared<prov_element>(pdforder, maintext,
						       ref, name, type);

	    std::vector<std::string> parts = utils::split(prov->path, "/");	  
	    assert(parts.size()>=3);
	    
	    std::string base = parts.at(1);
	    std::size_t index = std::stoi(parts.at(2));
	    
            if(orig.count(base))
              {
                auto& ref_item = orig[base][index];
                prov->set(ref_item[prov_lbl][0]);

                provs.push_back(prov);
              }
            else
              {
                LOG_S(WARNING) << "undefined reference path in document: "
                               << prov->path;
              }
          }
        else if(item.count(prov_lbl) and
		item[prov_lbl].size()==1)
          {
	    auto prov = std::make_shared<prov_element>(pdforder, maintext,
						       ref, name, type);

            prov->set(item[prov_lbl][0]);

            provs.push_back(prov);
          }
        else
          {
            LOG_S(ERROR) << "undefined prov for main-text item: " << item.dump();
          }
      }
  }

  bool subject<DOCUMENT>::init_items()
  {
    paragraphs.clear();
    other.clear();
    
    tables.clear();
    figures.clear();

    std::set<std::string> is_ignored = {"page-header", "page-footer"};

    std::set<std::string> is_text = {"title", "subtitle-level-1", "paragraph",
				     "footnote", "caption", "formula"};

    std::set<std::string> is_table = {"table"};
    std::set<std::string> is_figure = {"figure"};
    
    for(auto& prov:provs)
      {
	//auto item = orig.at((prov->path).first).at((prov->path).second);

	std::vector<std::string> parts = utils::split(prov->path, "/");	  
	
	std::string base = parts.at(1);
	std::size_t index = std::stoi(parts.at(2));
	  
	auto item = orig.at(base).at(index);
	
	if(is_text.count(prov->type))
          {
	    auto subj = std::make_shared<subject<PARAGRAPH> >(doc_hash, prov);
            bool valid = subj->set_data(item);

            if(valid)
              {
		paragraphs.push_back(subj);
              }
	    else
	      {
		LOG_S(WARNING) << "found invalid paragraph: " << item.dump();	
	      }
	  }
	else if(is_table.count(prov->type))
	  {
            //subject<TABLE> table(doc_hash, prov);
	    auto subj = std::make_shared<subject<TABLE> >(doc_hash, prov);	      
            bool valid = subj->set_data(item);

            if(valid)
              {
		tables.push_back(subj);
	      }
	    else
	      {
		LOG_S(WARNING) << "found table without structure";// << item.dump();
	      }
	  }
	else if(is_figure.count(prov->type))
	  {
            //subject<FIGURE> figure(doc_hash, prov);
            //bool valid = figure.set_data(item);

	    auto subj = std::make_shared<subject<FIGURE> >(doc_hash, prov);	      
            bool valid = subj->set_data(item);
	    
            if(valid)
              {
		figures.push_back(subj);
	      }
	    else
	      {
		LOG_S(WARNING) << "found figure without structure";// << item.dump();
	      }
	  }
	else
	  {
	    prov->ignore=true;
	    if(not is_ignored.count(prov->type))
	      {
		LOG_S(WARNING) << "found new `other` type: " << prov->type;			
	      }
	    
	    auto subj = std::make_shared<subject<PARAGRAPH> >(doc_hash, prov);
            bool valid = subj->set_data(item);

            if(valid)
              {
		other.push_back(subj);
              }
	    else
	      {
		LOG_S(WARNING) << "found invalid paragraph: " << item.dump();	
	      }
	  }
      }

    return true;    
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
  
  bool subject<DOCUMENT>::link_items()
  {
    //show_provs();    

    {
      doc_captions linker;
      linker.find_and_link_captions(*this);
    }

    /*
    {
      int cnt=0;
      for(auto& paragraph:paragraphs)
	{
	  if(paragraph.use_count()==1)
	    {
	      LOG_S(INFO) << cnt++ << "\t" << paragraph.use_count() << "\t" << paragraph->provs.at(0)->type;
	    }
	  else
	    {
	      LOG_S(WARNING) << cnt++ << "\t" << paragraph.use_count() << "\t" << paragraph->provs.at(0)->type;
	    }
	}
    }
    */

    {
      doc_maintext linker;

      linker.filter_maintext(*this);
      linker.concatenate_maintext(*this);
    }
    
    return true;
  }
  
  bool subject<DOCUMENT>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                                     std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    //LOG_S(INFO) << __FUNCTION__;

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

    bool valid_ents = finalise_entities();

    bool valid_rels = finalise_relations();

    //LOG_S(INFO) << "document: " << filepath;
    //LOG_S(INFO) << "properties: \n" << tabulate(properties);
    //LOG_S(INFO) << "entities: \n" << tabulate(entities, false);
    //LOG_S(INFO) << "relations: \n" << tabulate(entities, relations);

    return (valid_props and valid_ents and valid_rels);
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

  bool subject<DOCUMENT>::finalise_entities()
  {
    entities.clear();

    for(auto& subj:paragraphs)
      {
        //LOG_S(INFO) << __FUNCTION__ << ": " << subj.entities.size();

        for(auto& ent:subj->entities)
          {
            entities.emplace_back(subj->get_hash(),
                                  subj->get_name(),
                                  subj->get_path(),
                                  ent);
          }
      }
    //LOG_S(INFO) << "total #-ents: " << entities.size();

    for(auto& subj:tables)
      {
        for(auto& ent:subj->entities)
          {
            entities.emplace_back(subj->get_hash(),
                                  subj->get_name(),
                                  subj->get_path(),
                                  ent);
          }
      }

    for(auto& subj:figures)
      {
        for(auto& ent:subj->entities)
          {
            entities.emplace_back(subj->get_hash(),
                                  subj->get_name(),
                                  subj->get_path(),
                                  ent);
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
