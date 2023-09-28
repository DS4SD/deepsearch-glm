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
    const static inline std::string provs_lbl = "page-elements";

    const static inline std::string body_lbl = "body"; // containng the body of the document (text/tables/figures)
    const static inline std::string meta_lbl = "meta"; // containing other stuff (page-headers/footers etc)

    const static inline std::string texts_lbl = "texts";
    const static inline std::string tables_lbl = "tables";
    const static inline std::string figures_lbl = "figures";

    // element-labels
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
    virtual ~subject();

    void show();
    void clear();

    virtual nlohmann::json to_json();
    virtual bool from_json(const nlohmann::json& item);

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

  private:

    void set_meta(nlohmann::json& data);
    void set_orig(nlohmann::json& data);

    bool is_preprocessed();
    bool originates_from_pdf();
    
    void set_provs();

    void set_other();

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

    std::vector<std::shared_ptr<prov_element> > provs;

    std::vector<std::shared_ptr<base_subject> > body;
    std::vector<std::shared_ptr<base_subject> > meta;
    
    std::vector<std::shared_ptr<subject<PARAGRAPH> > > other;

    std::vector<std::shared_ptr<subject<PARAGRAPH> > > texts;
    std::vector<std::shared_ptr<subject<TABLE> > > tables;
    std::vector<std::shared_ptr<subject<FIGURE> > > figures;
  };

  subject<DOCUMENT>::subject():
    base_subject(DOCUMENT),

    filepath("<undef>"),

    doc_hash(-1),
    doc_name(""),

    provs(),

    body(),
    meta(),

    other(),
    
    texts(),
    tables(),
    figures()
  {}

  subject<DOCUMENT>::~subject()
  {}

  nlohmann::json subject<DOCUMENT>::to_json()
  {
    nlohmann::json result = orig;

    {
      nlohmann::json base = base_subject::_to_json();

      for(auto& elem:base.items())
        {
          result[elem.key()] = elem.value();
        }
    }

    // updated the description with predefined labels in schema
    if(result.count("description"))
      {
        auto& desc = result.at("description");
        for(auto& prop:properties)
          {
	    //LOG_S(INFO) << prop.get_type(); 
            if(prop.get_type()=="language")
              {
                std::vector<std::string> langs = {prop.get_name()};
                desc["languages"] = langs;
              }
          }

	//LOG_S(INFO) << "description: " << desc.dump(2);
      }

    // page-items contain everything on the document pages including page-header/footer,
    // title, subtitle, paragraph, table, figure, etc
    {
      nlohmann::json& page_items = result[provs_lbl];
      page_items = nlohmann::json::array({});

      for(auto& prov:provs)
        {
          //std::string ref = to_dref(*prov);
          page_items.push_back(prov->to_json(false));
        }
    }

    // the main-text will only have page-item prov and references to other
    // other parts of the document (text, table, figure, page-header/footer, footnotes)
    {
      nlohmann::json& body_text = result[body_lbl];
      nlohmann::json& meta_text = result[meta_lbl];

      body_text = nlohmann::json::array({});
      meta_text = nlohmann::json::array({});

      std::set<std::string> paths={};

      for(auto& prov:provs)
        {
          std::string path = prov->get_path();

          if(paths.count(path)==1) // skip items that have already been included
            {
              continue;
            }
          paths.insert(path);

          //
          {
            //item.erase("page");
            //item.erase("bbox");
            //item.erase("span");

	    //item.erase("text-order");
	    //item.erase("orig-order");	    
          }

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
    
    {
      std::set<std::string> keys
        = { "hash", "dloc", "orig", "text", "prov", "properties"};//, "word-tokens"};
      //"instances", "relations"};

      auto& json_texts = result[texts_lbl];
      json_texts = nlohmann::json::array({});

      for(std::size_t l=0; l<texts.size(); l++)
        {
          if(not texts.at(l)->is_valid())
            {
              continue;
            }

          nlohmann::json item = nlohmann::json::object({});

          auto& text = texts.at(l);

          auto _ = text->to_json();
          for(auto& elem:_.items())
            {
              if(keys.count(elem.key()))
                {
                  item[elem.key()] = elem.value();
                }
            }

          json_texts.push_back(item);
        }
    }

    {
      std::set<std::string> keys
        = { "hash", "dloc", "captions", "footnotes", "mentions", "properties"};
      //"instances", "relations"};

      for(std::size_t l=0; l<tables.size(); l++)
        {
          auto& prov = tables.at(l)->provs.at(0);

          std::vector<std::string> parts = utils::split(prov->get_path(), "/");

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
        = { "hash", "dloc", "captions", "footnotes", "mentions", "properties"};
      //, "instances", "relations"};

      for(std::size_t l=0; l<figures.size(); l++)
        {
          auto& prov = figures.at(l)->provs.at(0);

          std::vector<std::string> parts = utils::split(prov->get_path(), "/");

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

  bool subject<DOCUMENT>::from_json(const nlohmann::json& item)
  {
    LOG_S(ERROR) << "implement `from_json` for subject<DOCUMENT>";
    return false;
  }
  
  void subject<DOCUMENT>::clear()
  {
    base_subject::clear();

    dscr = nlohmann::json::object({});
    orig = nlohmann::json::object({});

    body.clear();
    meta.clear();
    
    other.clear();

    texts.clear();
    tables.clear();
    figures.clear();
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
      set_meta(data);
      set_orig(data);
    }

    if(is_preprocessed())
      {
        set_provs();

        set_other();
	
        set_texts();
        set_tables();
        set_figures();
      }
    else if(originates_from_pdf())
      {
	LOG_S(INFO) << "originates-from-pdf ... ";

        doc_normalisation<subject<DOCUMENT> > normaliser(*this);
        normaliser.execute_on_pdf();
      }
    else
      {
	LOG_S(WARNING) << "does not originates-from-pdf ... ";
	return false;
	//doc_normalisation<subject<DOCUMENT> > normaliser(*this);
	//normaliser.execute_on_doc();
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

    base_subject::dloc = doc_name + "#";
  }

  void subject<DOCUMENT>::set_orig(nlohmann::json& data)
  {
    orig = data;
  }

  bool subject<DOCUMENT>::is_preprocessed()
  {
    if(orig.count(provs_lbl) and
       //orig.count(maintext_lbl) and
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
    bool all_have_prov_or_ref = true;
    if(orig.count(maintext_lbl))
      {	
	for(const auto& item:orig.at(maintext_lbl))
	  {
	    bool has_prov = item.count(prov_lbl);
	    bool has_ref = (item.count("$ref") or item.count("__ref"));
	    
	    if((not has_prov) and (not has_ref))
	      {
		//LOG_S(INFO) << "item: " << item.dump(2);
		all_have_prov_or_ref = false;
	      }
	  }
      }
    //LOG_S(INFO) << "all_have_prov_or_ref: " << all_have_prov_or_ref;
    
    return all_have_prov_or_ref;
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

  void subject<DOCUMENT>::set_texts()
  {
    texts.clear();

    for(ind_type l=0; l<orig.at(texts_lbl).size(); l++)
      {
        const nlohmann::json& item = orig.at(texts_lbl).at(l);

        std::shared_ptr<subject<PARAGRAPH> > ptr
          = std::make_shared<subject<PARAGRAPH> >();

        ptr->from_json(item);

        texts.push_back(ptr);
      }
  }

  void subject<DOCUMENT>::set_other()
  {
    other.clear();

    for(ind_type l=0; l<orig.at(meta_lbl).size(); l++)
      {
        const nlohmann::json& item = orig.at(meta_lbl).at(l);

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
    std::map<std::string, val_type>                         property_total;
    std::map<std::pair<std::string, std::string>, val_type> property_label_mapping;

    for(auto& text:texts)
      {
        for(auto& prop:text->properties)
          {
            std::string mdl = prop.get_type();
            std::string lbl = prop.get_name();

            val_type conf = prop.get_conf();
            val_type dst = text->dst;

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

    for(auto& subj:texts)
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

}

#endif
