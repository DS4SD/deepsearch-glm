//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_H_

namespace andromeda
{

  template<>
  class subject<DOCUMENT>: public base_subject
  {
  public:

    // legacy
    const static inline std::string maintext_lbl = "main-text";

    // top-level document elements
    const static inline std::string pages_lbl = "page-dimensions";
    const static inline std::string provs_lbl = "page-elements";

    const static inline std::string body_lbl = "body"; // containng the body of the document (texts/tables/figures)
    const static inline std::string meta_lbl = "meta"; // containing other stuff (page-headers/footers etc)

    const static inline std::string texts_lbl = "texts";
    const static inline std::string tables_lbl = "tables";
    const static inline std::string figures_lbl = "figures";

    const static inline std::string page_headers_lbl = "page-headers";
    const static inline std::string page_footers_lbl = "page-footers";
    const static inline std::string footnotes_lbl = "footnotes";

    const static inline std::string other_lbl = "other";

    // element-labels
    const static inline std::string pdforder_lbl = "pdf-order";

    //const static inline std::string prov_lbl = "prov";
    //const static inline std::string text_lbl = "text";
    //const static inline std::string data_lbl = "data";

    const static inline std::string maintext_name_lbl = name_lbl;
    const static inline std::string maintext_type_lbl = type_lbl;

    const static inline std::string prov_bbox_lbl = "bbox";
    const static inline std::string prov_span_lbl = "span";

    const static inline std::set<std::string> texts_types = {"title",
                                                             "subtitle-level-1", "paragraph",
                                                             "formula", "equation"};

    const static inline std::set<std::string> maintext_types = {"title",
                                                                "subtitle-level-1", "paragraph",
                                                                "formula", "equation",
                                                                "table", "figure"};

  public:

    subject();
    virtual ~subject();

    void show();
    void clear();

    virtual nlohmann::json to_json(const std::set<std::string>& filters);

    virtual bool from_json(const nlohmann::json& item);
    virtual bool from_json(const nlohmann::json& item,
			   const std::vector<std::shared_ptr<prov_element> >& doc_provs);

    uint64_t get_hash() const { return doc_hash; }
    std::string get_name() const { return doc_name; }

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

    void clear_properties_from_texts();
    void clear_properties_from_tables();
    
    void join_properties_with_texts();
    void join_properties_with_tables();
    
  private:

    void set_dscr(nlohmann::json& data);
    void set_orig(nlohmann::json& data);

    bool is_preprocessed();
    bool originates_from_pdf();

    void set_pages();
    void set_provs();

    void set_texts();
    void set_tables();
    void set_figures();

    bool finalise_properties();
    bool finalise_instances();
    bool finalise_relations();

  public:

    std::filesystem::path filepath;

    uint64_t doc_hash;
    std::string doc_name;

    nlohmann::json orig, dscr;

    std::vector<std::shared_ptr<page_element> > pages;
    std::vector<std::shared_ptr<prov_element> > provs;

    std::vector<std::shared_ptr<base_subject> > body;
    std::vector<std::shared_ptr<base_subject> > meta;

    std::vector<std::shared_ptr<subject<TEXT> > > texts;
    std::vector<std::shared_ptr<subject<TABLE> > > tables;
    std::vector<std::shared_ptr<subject<FIGURE> > > figures;

    std::vector<std::shared_ptr<subject<TEXT> > > page_headers, page_footers, footnotes, other;
  };

  subject<DOCUMENT>::subject():
    base_subject(DOCUMENT),

    filepath("<undef>"),

    doc_hash(-1),
    doc_name(""),

    pages(),
    provs(),

    body(),
    meta(),

    texts(),
    tables(),
    figures(),

    page_headers(),
    page_footers(),
    footnotes(),
    
    other()
  {}

  subject<DOCUMENT>::~subject()
  {}

  nlohmann::json subject<DOCUMENT>::to_json(const std::set<std::string>& filters)
  {
    nlohmann::json result = base_subject::_to_json(filters);
    
    if(orig.count("description"))
      {
        result["description"] = orig["description"];
      }
    else
      {
        result["description"] = nlohmann::json::object({});
      }

    // updated the description with predefined labels in schema
    {
      auto& desc = result.at("description");
      for(auto& prop:properties)
        {
          if(prop.is_type("language"))
            {
              std::vector<std::string> langs = { prop.get_label() };
              desc["languages"] = langs;
            }
        }
      //LOG_S(INFO) << "description: " << desc.dump(2);
    }

    // pages contain everything on the document pages including index, widht and height
    base_subject::to_json(result, pages_lbl, pages);
    
    // page-elements contain everything on the document pages including page-header/footer,
    // title, subtitle, paragraph, table, figure, etc
    base_subject::to_json(result, provs_lbl, provs, false);

    // the body will only have page-item prov and references to other
    // other parts of the document (text, table, figure, page-header/footer, footnotes)
    {
      nlohmann::json& body_text = result[body_lbl];
      nlohmann::json& meta_text = result[meta_lbl];

      body_text = nlohmann::json::array({});
      meta_text = nlohmann::json::array({});

      std::set<std::string> paths={};

      for(auto& prov:provs)
        {
          std::string path = prov->get_item_ref();

          if(paths.count(path)==1) // skip items that have already been included
            {
              continue;
            }
          paths.insert(path);

          auto item = prov->to_json(true);
          if(maintext_types.count(prov->get_type()))
            {
              body_text.push_back(item);
            }
          else
            {
              meta_text.push_back(item);
            }
        }
    }

    std::set<std::string> doc_filters
        = { "hash", "dloc", "prov", "text", "data",
            "captions", "footnotes", "mentions",
            "properties"};

    base_subject::to_json(result, texts_lbl, texts, doc_filters);
    base_subject::to_json(result, tables_lbl, tables, doc_filters);
    base_subject::to_json(result, figures_lbl, figures, doc_filters);

    base_subject::to_json(result, page_headers_lbl, page_headers, doc_filters);
    base_subject::to_json(result, page_footers_lbl, page_footers, doc_filters);
    base_subject::to_json(result, footnotes_lbl, footnotes, doc_filters);

    base_subject::to_json(result, other_lbl, other, doc_filters);        

    return result;
  }

  bool subject<DOCUMENT>::from_json(const nlohmann::json& doc)
  {
    base_subject::_from_json(doc);

    base_subject::from_json(doc, pages_lbl, pages);
    base_subject::from_json(doc, provs_lbl, provs);
    
    base_subject::from_json(doc, provs, texts_lbl  , texts  );
    base_subject::from_json(doc, provs, tables_lbl , tables );
    base_subject::from_json(doc, provs, figures_lbl, figures);

    base_subject::from_json(doc, provs, page_headers_lbl, page_headers);
    base_subject::from_json(doc, provs, page_footers_lbl, page_footers);
    base_subject::from_json(doc, provs, footnotes_lbl, footnotes);
    
    base_subject::from_json(doc, provs, other_lbl, other);        
    
    return true;
  }
  
  bool subject<DOCUMENT>::from_json(const nlohmann::json& item,
				    const std::vector<std::shared_ptr<prov_element> >& doc_provs)
  {
    return (this->from_json(item));
  }
  
  void subject<DOCUMENT>::clear()
  {
    base_subject::clear();

    dscr = nlohmann::json::object({});
    orig = nlohmann::json::object({});

    body.clear();
    meta.clear();

    texts.clear();
    tables.clear();
    figures.clear();

    page_headers.clear();
    page_footers.clear();
    footnotes.clear();

    other.clear();
  }

  void subject<DOCUMENT>::show(bool txt, bool mdls,
                               bool ctok, bool wtok,
                               bool prps, bool insts, bool rels)
  {
    for(auto text:texts)
      {
        text->show(txt,mdls, ctok,wtok, prps,insts,rels);
      }
  }

  bool subject<DOCUMENT>::set_data(std::filesystem::path filepath,
                                   nlohmann::json& data,
                                   bool update_maintext)
  {
    this->filepath = filepath;

    return set_data(data, update_maintext);
  }

  bool subject<DOCUMENT>::set_data(nlohmann::json& data,
                                   bool update_maintext)
  {
    clear();

    {
      set_dscr(data);
      set_orig(data);
    }

    if(is_preprocessed())
      {
	from_json(data);
      }
    else if(originates_from_pdf())
      {
        doc_normalisation<subject<DOCUMENT> > normaliser(*this);
        normaliser.execute_on_pdf();
      }
    else
      {
        LOG_S(WARNING) << "does not originates-from-pdf ... ";
        return false;
      }

    return true;
  }

  void subject<DOCUMENT>::set_dscr(nlohmann::json& data)
  {
    if(data.count("file-info") and
       data["file-info"].count("document-hash"))
      {
        doc_name = data["file-info"].value("document-hash", doc_name);
        doc_hash = utils::to_reproducible_hash(doc_name);
      }
    else
      {
        LOG_S(WARNING) << "no `file-info.document-hash detected ...`";

        doc_name = filepath.c_str();
        doc_hash = utils::to_reproducible_hash(doc_name);
      }

    if(data.count("description"))
      {
        dscr = data.at("description");
      }

    base_subject::dloc = doc_name + "#";
  }

  void subject<DOCUMENT>::set_orig(nlohmann::json& data)
  {
    orig = data;
  }

  bool subject<DOCUMENT>::is_preprocessed()
  {
    if(orig.count(pages_lbl) and
       orig.count(provs_lbl) and

       orig.count(body_lbl) and
       orig.count(meta_lbl) and

       orig.count(texts_lbl) and
       orig.count(tables_lbl) and
       orig.count(figures_lbl))
      {
        return true;
      }

    return false;
  }

  bool subject<DOCUMENT>::originates_from_pdf()
  {
    return (orig.count(maintext_lbl) and (not orig.count(body_lbl)));
  }

  void subject<DOCUMENT>::show_provs()
  {
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
    for(auto& text:texts)
      {
        bool valid = text->set_tokens(char_normaliser, text_normaliser);

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
    // only keep document global properties

    std::set<std::pair<hash_type, model_name> > doc_properties={};

    for(auto& prop:properties)
      {
	doc_properties.insert({prop.get_subj_hash(), prop.get_model()});
      }
    
    for(auto& text:texts)
      {
        for(auto& prop:text->properties)
          {
	    std::pair<hash_type, model_name> key({prop.get_subj_hash(), prop.get_model()});
	    if(doc_properties.count(key)==0)
	      {
		properties.push_back(prop);
	      }
	  }
	text->properties.clear();
      }

    for(auto& table:tables)
      {
        for(auto& prop:table->properties)
          {
	    std::pair<hash_type, model_name> key({prop.get_subj_hash(), prop.get_model()});
	    if(doc_properties.count(key)==0)
	      {
		properties.push_back(prop);
	      }	    
	  }
	table->properties.clear();
      }    

    for(auto& figure:figures)
      {
        for(auto& prop:figure->properties)
          {
	    std::pair<hash_type, model_name> key({prop.get_subj_hash(), prop.get_model()});
	    if(doc_properties.count(key)==0)
	      {
		properties.push_back(prop);
	      }
	  }
	figure->properties.clear();
      }    
     
    return true;
  }

  bool subject<DOCUMENT>::finalise_instances()
  {
    instances.clear();

    for(auto& subj:texts)
      {
        //LOG_S(INFO) << __FUNCTION__ << ": " << subj.instances.size();

        for(auto& ent:subj->instances)
          {
            instances.emplace_back(subj->get_hash(),
                                   subj->get_name(),
                                   //subj->get_path(),
				   subj->get_self_ref(),
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
                                   //subj->get_path(),
				   subj->get_self_ref(),
                                   ent);
          }

        for(auto& capt:subj->captions)
          {
            for(auto& ent:capt->instances)
              {
                instances.emplace_back(capt->get_hash(),
                                       capt->get_name(),
                                       //capt->get_path(),
				       capt->get_self_ref(),
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
                                   //subj->get_path(),
				   subj->get_self_ref(),
                                   ent);
          }

        for(auto& capt:subj->captions)
          {
            for(auto& ent:capt->instances)
              {
                instances.emplace_back(capt->get_hash(),
                                       capt->get_name(),
                                       //capt->get_path(),
				       capt->get_self_ref(),
                                       ent);
              }
          }
      }

    return true;
  }

  bool subject<DOCUMENT>::finalise_relations()
  {
    relations.clear();

    for(auto& text:texts)
      {
        for(auto& rel:text->relations)
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

  void subject<DOCUMENT>::clear_properties_from_texts()
  {
    for(auto& text:texts)    
      {
	text->properties.clear();
      }
  }
  
  void subject<DOCUMENT>::join_properties_with_texts()
  {
    clear_properties_from_texts();
    
    for(auto& prop:this->properties)
      {
	std::string path = prop.get_subj_path();
	LOG_S(INFO) << path;
	
	auto parts = utils::split(path, "/");

	if(parts.size()<3)
	  {
	    continue;
	  }
	
	int ind = std::stoi(parts.at(2));
	LOG_S(INFO) << " -> " << ind;
	
	if(parts.at(1)==texts_lbl and ind<texts.size())
	  {
	    assert(texts.at(ind)->get_hash()==prop.get_subj_hash());
	    texts.at(ind)->properties.push_back(prop);
	  }
	else
	  {}
      }
  }

  void subject<DOCUMENT>::clear_properties_from_tables()
  {
    for(auto& table:tables)    
      {
	table->properties.clear();
      }
  }
  
  void subject<DOCUMENT>::join_properties_with_tables()
  {
    for(auto& table:tables)    
      {
	table->properties.clear();
      }
    
    for(auto& prop:this->properties)
      {
	std::string path = prop.get_subj_path();
	LOG_S(INFO) << path;

	auto parts = utils::split(path, "/");
	if(parts.size()<3)
	  {
	    continue;
	  }
	
	int ind = std::stoi(parts.at(2));
	LOG_S(INFO) << " -> " << ind;
	
	if(parts.at(1)==tables_lbl and ind<tables.size())
	  {
	    assert(tables.at(ind)->get_hash()==prop.get_subj_hash());
	    tables.at(ind)->properties.push_back(prop);
	  }
	else
	  {}
      }
  }
  
}

#endif
