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

    const static inline std::string maintext_name_lbl = name_lbl;
    const static inline std::string maintext_type_lbl = type_lbl;

    const static inline std::string prov_bbox_lbl = "bbox";
    const static inline std::string prov_span_lbl = "span";

    const static inline std::set<std::string> texts_types = {"title",
                                                             "subtitle-level-1", "subtitle-level-2", "subtitle-level-3",
                                                             "paragraph", "text",
                                                             "formula", "equation"};

    const static inline std::set<std::string> body_types = {"title",
                                                            "subtitle-level-1", "subtitle-level-2", "subtitle-level-3",
                                                            "paragraph", "text",
                                                            "formula", "equation",
                                                            "table", "figure"};
    /*
      const static inline std::set<std::string> maintext_types = {"title",
      "subtitle-level-1", "paragraph",
      "formula", "equation",
      "table", "figure"};
    */

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
    void set_name(std::string);

    std::filesystem::path get_filepath() { return filepath; }

    nlohmann::json& get_orig() { return orig; }

    std::vector<std::shared_ptr<page_element> >& get_pages() { return pages; }
    std::vector<std::shared_ptr<prov_element> >& get_provs() { return provs; }

    void show(bool txt=true, bool mdls=false,
              bool ctokens=false, bool wtokens=true,
              bool prps=true, bool insts=true, bool rels=true);

    void set_title(std::string title);
    void set_abstract(std::vector<std::string> abstract);
    void set_date(std::string date);
    void set_authors(std::vector<std::string>& authors);
    void set_affiliations(std::vector<std::string>& affils);
    
    void set_advanced(nlohmann::json& advanced);

    bool set_data(nlohmann::json& data, bool order_maintext);

    bool set_data(std::filesystem::path filepath,
                  nlohmann::json& data, bool order_maintext);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool finalise();

    void init_provs();
    void show_provs();

    bool push_back(std::shared_ptr<subject<TEXT> > subj);
    bool push_back(std::shared_ptr<subject<TABLE> > subj);
    bool push_back(std::shared_ptr<subject<FIGURE> > subj);

  private:

    void join_properties();
    void join_instances();
    void join_applied_models();

  private:

    void set_kept(const nlohmann::json& data);
    void set_orig(const nlohmann::json& data);

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

  private:

    std::filesystem::path filepath;

    uint64_t doc_hash;
    std::string doc_name;

    nlohmann::json orig;

    std::vector<std::string> kept_keys;
    nlohmann::json kept;

  public:

    std::vector<std::shared_ptr<page_element> > pages;
    std::vector<std::shared_ptr<prov_element> > provs;

    std::vector<std::shared_ptr<base_subject> > body;
    std::vector<std::shared_ptr<base_subject> > meta;

  public:

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

    orig(nlohmann::json::value_t::null),

    kept_keys({"description", "file-info", "_s3_data", "conversion_settings"}),
    kept(nlohmann::json::object({})),

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

    for(auto key:kept_keys)
      {
        result[key] = nlohmann::json::object({});

        if(kept.count(key))
          {
            result[key] = kept[key];
          }
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
          //if(maintext_types.count(prov->get_type()))
          if(body_types.count(prov->get_type()))
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
          "captions", "footnotes", "mentions"};

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

    set_kept(doc);

    base_subject::from_json(doc, pages_lbl, pages);
    base_subject::from_json(doc, provs_lbl, provs);

    base_subject::from_json(doc, provs, texts_lbl, texts);
    base_subject::from_json(doc, provs, tables_lbl, tables);
    base_subject::from_json(doc, provs, figures_lbl, figures);

    base_subject::from_json(doc, provs, page_headers_lbl, page_headers);
    base_subject::from_json(doc, provs, page_footers_lbl, page_footers);
    base_subject::from_json(doc, provs, footnotes_lbl, footnotes);

    base_subject::from_json(doc, provs, other_lbl, other);

    {
      join_properties();
      join_instances();

      join_applied_models();
    }

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

    orig = nlohmann::json::object({});
    kept = nlohmann::json::object({});

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

  void subject<DOCUMENT>::set_title(std::string title)
  {
    auto& desc = kept["description"];
    desc["title"] = title;
  }

  void subject<DOCUMENT>::set_abstract(std::vector<std::string> abstract)
  {
    auto& desc = kept["description"];
    desc["abstract"] = abstract;
  }

  void subject<DOCUMENT>::set_date(std::string date)
  {
    auto& desc = kept["description"];
    desc["date"] = date;
  }

  void subject<DOCUMENT>::set_authors(std::vector<std::string>& authors)
  {
    auto& desc = kept["description"];
    
    auto& items = desc["authors"];
    items = nlohmann::json::array({});
    
    for(std::size_t i=0; i<authors.size(); i++)
      {
	auto item = nlohmann::json::object({});

	item["name"] = authors.at(i);
	items.push_back(item);
      }
  }

  void subject<DOCUMENT>::set_affiliations(std::vector<std::string>& affiliations)
  {
    auto& desc = kept["description"];
    
    auto& items = desc["affiliations"];
    items = nlohmann::json::array({});
    
    for(std::size_t i=0; i<affiliations.size(); i++)
      {
	auto item = nlohmann::json::object({});

	item["name"] = affiliations.at(i);
	items.push_back(item);
      }
  }

  void subject<DOCUMENT>::set_advanced(nlohmann::json& advanced)
  {
    auto& desc = kept["description"];

    if(desc.count("advanced")==0)
      {
	desc["advanced"] = nlohmann::json::object({});
      }

    auto& adv = desc["advanced"];
    for(auto& elem:advanced.items())
      {
	adv[elem.key()] = elem.value();
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
      set_kept(data);
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

  void subject<DOCUMENT>::set_kept(const nlohmann::json& data)
  {
    for(auto key:kept_keys)
      {
        if(data.count(key))
          {
            kept[key] = data.at(key);
          }
        else
          {
            kept[key] = nlohmann::json::object({});
          }
      }

    if(data.count("file-info") and
       data["file-info"].count("document-hash"))
      {
        //std::string name = data["file-info"].value("document-hash", doc_name)
        std::string name = data["file-info"]["document-hash"].get<std::string>();
        set_name(name);
        //doc_name = data["file-info"].value("document-hash", doc_name);
        //doc_hash = utils::to_reproducible_hash(doc_name);
      }
    else
      {
        LOG_S(WARNING) << "no `file-info.document-hash detected ...`";

        // std::string name = filepath.c_str();
        std::string name = filepath.string();

        set_name(name);
        //doc_name = filepath.c_str();
        //doc_hash = utils::to_reproducible_hash(doc_name);
      }
  }

  void subject<DOCUMENT>::set_name(std::string name)
  {
    doc_name = name;
    doc_hash = utils::to_reproducible_hash(doc_name);

    base_subject::sref = "#";
    base_subject::dloc = doc_name + "#";
  }

  void subject<DOCUMENT>::set_orig(const nlohmann::json& data)
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
    //LOG_S(INFO) << "#-properties: " << properties.size();

    // only keep document global properties
    //std::set<std::pair<hash_type, model_name> > doc_properties={};
    //for(auto& prop:properties)
    //{
    //doc_properties.insert({prop.get_subj_hash(), prop.get_model()});
    //}

    for(auto& text:texts)
      {
        for(auto& prop:text->properties)
          {
            //std::pair<hash_type, model_name> key({prop.get_subj_hash(), prop.get_model()});
            //if(doc_properties.count(key)==0)
            //{
            properties.push_back(prop);
            //}
          }
        text->properties.clear();
      }

    for(auto& table:tables)
      {
        for(auto& prop:table->properties)
          {
            //std::pair<hash_type, model_name> key({prop.get_subj_hash(), prop.get_model()});
            //if(doc_properties.count(key)==0)
            //{
            properties.push_back(prop);
            //}
          }
        table->properties.clear();
      }

    for(auto& figure:figures)
      {
        for(auto& prop:figure->properties)
          {
            //std::pair<hash_type, model_name> key({prop.get_subj_hash(), prop.get_model()});
            //if(doc_properties.count(key)==0)
            //{
            properties.push_back(prop);
            //}
          }
        figure->properties.clear();
      }

    //LOG_S(INFO) << "#-properties: " << properties.size();

    std::sort(properties.begin(), properties.end());

    auto itr = std::unique(properties.begin(), properties.end());
    properties.erase(itr, properties.end());

    //LOG_S(INFO) << "#-properties: " << properties.size();

    return true;
  }

  bool subject<DOCUMENT>::finalise_instances()
  {
    // only keep the DOCUMENT instances ...

    for(auto itr=instances.begin(); itr!=instances.end(); )
      {
        if(itr->is_subject(DOCUMENT))
          {
            itr++;
          }
        else
          {
            itr = instances.erase(itr);
          }
      }

    //instances.clear();

    for(auto& subj:texts)
      {
        for(auto& inst:subj->instances)
          {
            instances.push_back(inst);
          }
      }

    for(auto& subj:tables)
      {
        for(auto& inst:subj->instances)
          {
            instances.push_back(inst);
          }

        for(auto& capt:subj->captions)
          {
            for(auto& inst:capt->instances)
              {
                instances.push_back(inst);
              }
          }
      }

    for(auto& subj:figures)
      {
        for(auto& inst:subj->instances)
          {
            instances.push_back(inst);
          }

        for(auto& capt:subj->captions)
          {
            for(auto& inst:capt->instances)
              {
                instances.push_back(inst);
              }
          }
      }

    std::sort(instances.begin(), instances.end());

    auto itr = std::unique(instances.begin(), instances.end());
    instances.erase(itr, instances.end());

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

        for(auto& capt:table->captions)
          {
            for(auto& rel:capt->relations)
              {
                relations.push_back(rel);
              }
          }
      }

    for(auto& figure:figures)
      {
        for(auto& rel:figure->relations)
          {
            relations.push_back(rel);
          }

        for(auto& capt:figure->captions)
          {
            for(auto& rel:capt->relations)
              {
                relations.push_back(rel);
              }
          }
      }

    std::sort(relations.begin(), relations.end());

    return true;
  }

  void subject<DOCUMENT>::join_properties()
  {
    for(auto& text:texts) { text->properties.clear(); }
    for(auto& table:tables) { table->properties.clear(); }
    for(auto& figure:figures) { figure->properties.clear(); }

    for(auto& prop:this->properties)
      {
        std::string path = prop.get_subj_path();

        auto parts = utils::split(path, "/");

        if(parts.size()==1) // document properties, nothing to be done ...
          {}
        else if(parts.size()==3 and parts.at(1)==texts_lbl)
          {
            int ind = std::stoi(parts.at(2));

            assert(texts.at(ind)->get_hash()==prop.get_subj_hash());
            texts.at(ind)->properties.push_back(prop);
            texts.at(ind)->applied_models.insert(prop.get_type());
          }
        else if(parts.size()==3 and parts.at(1)==tables_lbl)
          {
            int ind = std::stoi(parts.at(2));

            assert(tables.at(ind)->get_hash()==prop.get_subj_hash());
            tables.at(ind)->properties.push_back(prop);
            tables.at(ind)->applied_models.insert(prop.get_type());
          }
        else if(parts.size()==3 and parts.at(1)==figures_lbl)
          {
            int ind = std::stoi(parts.at(2));

            assert(figures.at(ind)->get_hash()==prop.get_subj_hash());
            figures.at(ind)->properties.push_back(prop);
            figures.at(ind)->applied_models.insert(prop.get_type());
          }
        else if(parts.size()==5 and parts.at(1)==tables_lbl and parts.at(3)==captions_lbl)
          {
            int ti = std::stoi(parts.at(2));
            int ci = std::stoi(parts.at(4));

            assert(tables.at(ti)->get_hash()==prop.get_subj_hash());
            tables.at(ti)->captions.at(ci)->properties.push_back(prop);
            tables.at(ti)->captions.at(ci)->applied_models.insert(prop.get_type());
          }
        else if(parts.size()==5 and parts.at(1)==figures_lbl and parts.at(3)==captions_lbl)
          {
            int fi = std::stoi(parts.at(2));
            int ci = std::stoi(parts.at(4));

            assert(figures.at(fi)->get_hash()==prop.get_subj_hash());
            figures.at(fi)->captions.at(ci)->properties.push_back(prop);
            figures.at(fi)->captions.at(ci)->applied_models.insert(prop.get_type());
          }
        else
          {
            LOG_S(WARNING) << "ignoring properties with subj-path: " << path;
          }
      }
  }

  void subject<DOCUMENT>::join_instances()
  {
    for(auto& text:texts) { text->instances.clear(); }
    for(auto& table:tables) { table->instances.clear(); }
    for(auto& figure:figures) { figure->instances.clear(); }

    for(auto& inst:this->instances)
      {
        std::string path = inst.get_subj_path();

        auto parts = utils::split(path, "/");

        if(parts.size()==1) // document instances, nothing to be done ...
          {}
        else if(parts.size()==3 and parts.at(1)==texts_lbl)
          {
            int ind = std::stoi(parts.at(2));

            assert(texts.at(ind)->get_hash()==inst.get_subj_hash());
            texts.at(ind)->instances.push_back(inst);
            texts.at(ind)->applied_models.insert(inst.get_type());
          }
        else if(parts.size()==3 and parts.at(1)==tables_lbl)
          {
            int ind = std::stoi(parts.at(2));

            assert(tables.at(ind)->get_hash()==inst.get_subj_hash());

            assert(inst.get_name().size()>0);

            tables.at(ind)->instances.push_back(inst);
            tables.at(ind)->applied_models.insert(inst.get_type());
          }
        else if(parts.size()==3 and parts.at(1)==figures_lbl)
          {
            int ind = std::stoi(parts.at(2));

            assert(figures.at(ind)->get_hash()==inst.get_subj_hash());

            figures.at(ind)->instances.push_back(inst);
            figures.at(ind)->applied_models.insert(inst.get_type());
          }
        else if(parts.size()==5 and parts.at(1)==tables_lbl and parts.at(3)==captions_lbl)
          {
            int ti = std::stoi(parts.at(2));
            int ci = std::stoi(parts.at(4));

            assert(tables.at(ti)->captions.at(ci)->get_hash()==inst.get_subj_hash());

            tables.at(ti)->captions.at(ci)->instances.push_back(inst);
            tables.at(ti)->captions.at(ci)->applied_models.insert(inst.get_type());
          }
        else if(parts.size()==5 and parts.at(1)==figures_lbl and parts.at(3)==captions_lbl)
          {
            int fi = std::stoi(parts.at(2));
            int ci = std::stoi(parts.at(4));

            assert(figures.at(fi)->captions.at(ci)->get_hash()==inst.get_subj_hash());

            figures.at(fi)->captions.at(ci)->instances.push_back(inst);
            figures.at(fi)->captions.at(ci)->applied_models.insert(inst.get_type());
          }
        else
          {
            LOG_S(WARNING) << "ignoring instances with subj-path: " << path;
          }
      }
  }

  void subject<DOCUMENT>::join_applied_models()
  {
    for(auto& text:texts)
      {
        text->applied_models = this->applied_models;
      }

    for(auto& table:tables)
      {
        table->applied_models = this->applied_models;

        for(auto& capt:table->captions)
          {
            capt->applied_models = this->applied_models;
          }
      }

    for(auto& figure:figures)
      {
        figure->applied_models = this->applied_models;

        for(auto& capt:figure->captions)
          {
            capt->applied_models = this->applied_models;
          }
      }
  }

  bool subject<DOCUMENT>::push_back(std::shared_ptr<subject<TEXT> > subj)
  {
    // we need to make a copy to ensure that we update the self-reference only on the copied struct
    std::shared_ptr<subject<TEXT> > copy = std::make_shared<subject<TEXT> >(*subj);

    std::string sref = fmt::format("#/{}/{}", texts_lbl, texts.size());
    std::string pref = fmt::format("#/{}/{}", provs_lbl, provs.size());
    std::string dloc = fmt::format("{}/#/{}/{}", doc_name, texts_lbl, texts.size());

    copy->set_dloc(dloc);
    copy->set_self_ref(sref);

    if(subj->provs.size()==0)
      {
        range_type rng = {0, subj->get_len()};
        auto prov = std::make_shared<prov_element>(sref, pref, "text", "text", rng);

        copy->provs.push_back(prov);
        provs.push_back(prov);
      }
    else
      {
        for(auto prov:subj->provs)
          {
            std::shared_ptr<prov_element> copy_prov = std::make_shared<prov_element>(*prov);
            copy_prov->set_item_ref(sref);
            copy_prov->set_self_ref(pref);

            copy->provs.push_back(copy_prov);
            provs.push_back(prov);
          }
      }

    texts.push_back(copy);

    return true;
  }

  bool subject<DOCUMENT>::push_back(std::shared_ptr<subject<TABLE> > subj)
  {
    // we need to make a copy to ensure that we update the self-reference only on the copied struct
    std::shared_ptr<subject<TABLE> > copy = std::make_shared<subject<TABLE> >(*subj);

    std::string sref = fmt::format("#/{}/{}", tables_lbl, tables.size());
    std::string pref = fmt::format("#/{}/{}", provs_lbl, provs.size());
    std::string dloc = fmt::format("{}/#/{}/{}", doc_name, tables_lbl, tables.size());

    range_type rng = {0, subj->num_rows()};
    auto prov = std::make_shared<prov_element>(sref, pref, "table", "table", rng);

    provs.push_back(prov);

    copy->set_dloc(dloc);
    copy->set_self_ref(sref);
    copy->provs.push_back(prov);

    tables.push_back(copy);

    return true;
  }

  bool subject<DOCUMENT>::push_back(std::shared_ptr<subject<FIGURE> > subj)
  {
    // we need to make a copy to ensure that we update the self-reference only on the copied struct
    std::shared_ptr<subject<FIGURE> > copy = std::make_shared<subject<FIGURE> >(*subj);

    std::string sref = fmt::format("#/{}/{}", figures_lbl, figures.size());
    std::string pref = fmt::format("#/{}/{}", provs_lbl, provs.size());
    std::string dloc = fmt::format("{}/#/{}/{}", doc_name, figures_lbl, figures.size());

    range_type rng = {0, 0};
    auto prov = std::make_shared<prov_element>(sref, pref, "figure", "figure", rng);

    provs.push_back(prov);

    copy->set_dloc(dloc);
    copy->set_self_ref(sref);
    copy->provs.push_back(prov);

    figures.push_back(copy);

    return true;
  }

}

#endif
