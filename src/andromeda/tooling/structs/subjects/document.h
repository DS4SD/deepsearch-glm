//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_H_

namespace andromeda
{

  template<>
  class subject<DOCUMENT>: public base_subject
  {
  public:

    const static inline std::string provs_lbl = "document-items";
    
    const static inline std::string maintext_lbl = "main-text";
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

    //std::vector<prov_element> provs;
    std::vector<std::shared_ptr<prov_element> > provs;
    
    std::vector<subject<PARAGRAPH> > paragraphs;
    std::vector<subject<TABLE> > tables;
    std::vector<subject<FIGURE> > figures;
  };

  subject<DOCUMENT>::subject():
    base_subject(DOCUMENT),

    filepath("<undef>"),
    
    doc_name(""),
    doc_hash(-1),

    provs(),
    
    paragraphs(),
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

    {
      nlohmann::json& doc_provs = result[provs_lbl];
      doc_provs = nlohmann::json::object({});

      doc_provs[base_subject::head_lbl] = prov_element::get_headers();
      doc_provs[base_subject::data_lbl] = nlohmann::json::array({});

      auto& data = doc_provs.at(base_subject::data_lbl);
      for(auto& prov:provs)
	{
	  data.push_back(prov->to_json_row());
	}
    }
    
    if(result.count(maintext_lbl))
      {
        //std::set<std::string> keys
        //= { "hash", "orig", "text", "properties"};
        //"entities", "relations"};

        for(std::size_t l=0; l<paragraphs.size(); l++)
          {
            if(paragraphs.at(l).is_valid())
              {
                auto& paragraph = paragraphs.at(l);

                auto& prov = paragraph.provs.at(0);
                auto& item = result[prov->path.first][prov->path.second];

                auto _ = paragraph.to_json();
                for(auto& elem:_.items())
                  {
                    //if(keys.count(elem.key()))
                    //{
                    item[elem.key()] = elem.value();
                    //  }
                  }
              }
          }
      }

    if(result.count("tables"))
      {
        std::set<std::string> keys
          = { "hash", "captions", "footnotes", "mentions", "properties"};
        //"entities", "relations"};

        for(std::size_t l=0; l<tables.size(); l++)
          {
            if(tables.at(l).is_valid())
              {
                auto& table = tables.at(l);

                auto& prov = table.provs.at(0);
                auto& item = result[prov->path.first][prov->path.second];

                auto _ = table.to_json();
                for(auto& elem:_.items())
                  {
                    //if(keys.count(elem.key()))
                    //{
                    item[elem.key()] = elem.value();
                    //}
                  }
              }
          }
      }

    if(result.count("figures"))
      {
        //std::vector<std::string> keys
        //= { "hash",  "captions", "footnotes", "mentions", "properties"};
        //, "entities", "relations"};

        for(std::size_t l=0; l<figures.size(); l++)
          {
            if(figures.at(l).is_valid())
              {
                auto& figure = figures.at(l);

                auto& prov = figure.provs.at(0);
                auto& item = result[prov->path.first][prov->path.second];

                auto _ = figure.to_json();
                for(auto& elem:_.items())
                  {
                    //if(keys.count(elem.key()))
                    //{
                    item[elem.key()] = elem.value();
                    //}
                  }
              }
          }
      }

    return result;
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
        paragraph.show(txt,mdls, ctok,wtok, prps,ents,rels);
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
	
        if(item.count("$ref"))
          {
	    ref = item["$ref"].get<std::string>();	    

	    auto prov = std::make_shared<prov_element>(pdforder, maintext,
						       ref, name, type);
	    
            if(orig.count(prov->path.first))
              {
                auto& ref_item = orig[(prov->path).first][(prov->path).second];
                prov->set(ref_item[prov_lbl][0]);

                provs.push_back(prov);
              }
            else
              {
                LOG_S(WARNING) << "undefined reference path in document: "
                               << prov->path.first;
              }
          }
        else if(item.count("__ref"))
          {
	    ref = item["__ref"].get<std::string>();	    

	    auto prov = std::make_shared<prov_element>(pdforder, maintext,
						       ref, name, type);
	    
            if(orig.count(prov->path.first))
              {
		auto& ref_item = orig[(prov->path).first][(prov->path).second];
                prov->set(ref_item[prov_lbl][0]);

                provs.push_back(prov);
              }
            else
              {
                LOG_S(WARNING) << "undefined reference path in document: "
                               << prov->path.first;
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
    tables.clear();
    figures.clear();
    paragraphs.clear();

    std::set<std::string> is_ignored = {"page-header", "page-footer"};
    std::set<std::string> is_text = {"title", "subtitle-level-1", "paragraph",
				   "footnote", "caption"};

    std::set<std::string> is_table = {"table"};
    std::set<std::string> is_figure = {"figure"};
    
    //for(auto itr=provs.begin(); itr!=provs.end(); itr++)
    for(auto& prov:provs)
      {
	auto item = orig.at((prov->path).first).at((prov->path).second);

	if(is_ignored.count(prov->type))
	  {
	    prov->ignore=true;
	  }	
	else if(is_text.count(prov->type))
          {
            subject<PARAGRAPH> subj(doc_hash, prov);
            bool valid = subj.set_data(item);

            if(valid)
              {
		prov->dref = {PARAGRAPH, paragraphs.size()};
		paragraphs.push_back(subj);
              }
	    else
	      {
		LOG_S(WARNING) << "found invalid paragraph: " << item.dump();	
	      }
	  }
	else if(is_table.count(prov->type))
	  {
            subject<TABLE> table(doc_hash, prov);
            bool valid = table.set_data(item);

            if(valid)
              {
		prov->dref = {TABLE, tables.size()};
		tables.push_back(table);
	      }
	    else
	      {
		LOG_S(WARNING) << "found table without structure";// << item.dump();
	      }
	  }
	else if(is_figure.count(prov->type))
	  {
            subject<FIGURE> figure(doc_hash, prov);
            bool valid = figure.set_data(item);

            if(valid)
              {
		prov->dref = {FIGURE, figures.size()};
		figures.push_back(figure);
	      }
	    else
	      {
		LOG_S(WARNING) << "found figure without structure";// << item.dump();
	      }
	  }
	else
	  {
	    LOG_S(WARNING) << "ignoring: " << prov->type;		  
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
      doc_linker linker;
      linker.find_captions(*this);
    }
    
    return true;
  }
  


  
  /*
  bool subject<DOCUMENT>::init_items()
  {
    //LOG_S(INFO) << __FUNCTION__;

    if(not orig.count(maintext_lbl))
      {
        return false;
      }

    init_provs(provs);

    {
      remove_headers_and_footers(provs);
    }

    {
      init_tables(provs);
      //clean_provs(provs);
    }

    {
      init_figures(provs);
      //clean_provs(provs);
    }

    {
      init_paragraphs(provs);
    }

    return true;
  }

  bool subject<DOCUMENT>::clean_provs(std::vector<prov_element>& provs)
  {
    for(auto itr=provs.begin(); itr!=provs.end(); )
      {
        if(itr->ignore)
          {
            itr = provs.erase(itr);
          }
        else
          {
            itr++;
          }
      }

    return true;
  }
  
  bool subject<DOCUMENT>::remove_headers_and_footers(std::vector<prov_element>& provs)
  {
    //LOG_S(INFO) << __FUNCTION__;

    std::set<std::string> to_be_ignored={"page-header", "page-footer"};
    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
        itr->ignore = to_be_ignored.count(itr->type)? true:false;
      }

    return true;
  }

  bool subject<DOCUMENT>::identify_repeating_text(std::vector<prov_element>& provs)
  {
    typedef std::array<ind_type, 4> key_type;
    typedef std::vector<std::array<ind_type, 3> > val_type;

    std::set<std::string> text_types
      = { "title", "paragraph", "subtitle-level-1", "caption", "footnote"};

    std::map<key_type, val_type> bbox_to_ind={};
    for(ind_type ind=0; ind<provs.size(); ind++)
      {
	auto& prov = provs.at(ind);
	
	if(prov.ignore or text_types.count(prov.type)==0)
	  {
	    continue;
	  }
	
	std::array<ind_type, 4> bbox
	  = { ind_type(prov.bbox.at(0)),
	      ind_type(prov.bbox.at(1)),
	      ind_type(prov.bbox.at(2)),
	      ind_type(prov.bbox.at(3))};

	bbox_to_ind[bbox].push_back({prov.page, prov.maintext_ind, ind});
      }

    for(auto itr=bbox_to_ind.begin(); itr!=bbox_to_ind.end(); itr++)
      {
	bool repeating = (itr->second).size()>1;
	std::string text="", other="";

	auto& maintext = orig[maintext_lbl];
	
	if(repeating)
	  {
	    ind_type mind = (itr->second).at(0).at(1);
	    text = maintext[mind]["text"].get<std::string>();

	    for(auto coor:itr->second)
	      {
		other = maintext[coor.at(1)]["text"].get<std::string>();

		if(text!=other)
		  {
		    repeating=false;		    
		  }
	      }
	  }

	if(repeating)
	  {
	    for(auto coor:itr->second)
	      {
		provs.at(coor.at(2)).ignore = true;

		other = maintext[coor.at(1)]["text"].get<std::string>();
		//LOG_S(WARNING) << "ignoring: " << other;
	      }
	  }
      }

    return true;
  }
  
  bool subject<DOCUMENT>::init_tables(std::vector<prov_element>& provs)
  {
    //LOG_S(INFO) << __FUNCTION__;

    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
	if(itr->type=="table")
          {
            subject<TABLE> table(doc_hash, *itr);

            auto item = orig[(itr->path).first][(itr->path).second];
            bool valid_table = table.set_data(item);

            if(not valid_table)
              {
		LOG_S(WARNING) << "found table without structure";// << item.dump();
	      }
	    
	    itr->ignore=true;
	    tables.push_back(table);
          }
      }

    //LOG_S(WARNING) << "orig #-tables: " << orig["tables"].size();
    //LOG_S(WARNING) << "list #-tables: " << tables.size();

    //int cnt=0;
    
    // FIXME: we need to factor this out in another class
    for(auto& table:tables)
      {
        auto mtext_ind = table.provs.at(0).maintext_ind;

        auto ind_m1 = mtext_ind-1;
        auto ind_p1 = mtext_ind+1;

	//LOG_S(INFO) << "table-" << (++cnt) << " => ("
	//<< "m1: " << provs.at(ind_m1).type << "; "
	//<< "p1: " << provs.at(ind_p1).type << ")";
	
        if(ind_m1>=0 and ind_m1<provs.size() and
           (not provs.at(ind_m1).ignore) and
           provs.at(ind_m1).type=="caption")
          {
            auto caption_prov = provs.at(ind_m1);
            subject<PARAGRAPH> caption(doc_hash, provs.at(ind_m1));

            auto item = orig[caption_prov.path.first][caption_prov.path.second];
            bool valid = caption.set_data(item);

	    //LOG_S(INFO) << "table-caption text: " << caption.text;
	    
            if(valid)
              {
                table.captions.push_back(caption);
                provs.at(ind_m1).ignore = true;
              }
	    else
	      {
		LOG_S(WARNING) << "found invalid table-caption: " << item.dump();
	      }
          }
        else if(ind_p1>=0 and ind_p1<provs.size() and
                (not provs.at(ind_p1).ignore) and
                provs.at(ind_p1).type=="caption")
          {
            auto caption_prov = provs.at(ind_p1);
            subject<PARAGRAPH> caption(doc_hash, provs.at(ind_p1));

            auto item = orig[caption_prov.path.first][caption_prov.path.second];
            bool valid = caption.set_data(item);

	    //LOG_S(INFO) << "table-caption text: " << caption.text;
	    
            if(valid)
              {
                table.captions.push_back(caption);
                provs.at(ind_p1).ignore = true;
              }
	    else
	      {
		LOG_S(WARNING) << "found invalid table-caption: " << item.dump();
	      }
          }

        auto fnote_ind = mtext_ind+1;
        if(fnote_ind>=0 and fnote_ind<provs.size() and
           (not provs.at(fnote_ind).ignore) and
           provs.at(fnote_ind).type=="footnote")
          {
            auto footnote_prov = provs.at(fnote_ind);
            subject<PARAGRAPH> footnote(doc_hash, footnote_prov);

            auto item = orig[footnote_prov.path.first][footnote_prov.path.second];
            bool valid = footnote.set_data(item);

            if(valid)
              {
                table.footnotes.push_back(footnote);
                provs.at(fnote_ind).ignore = true;
              }
          }
      }
    
    return true;
  }

  bool subject<DOCUMENT>::init_figures(std::vector<prov_element>& provs)
  {
    //LOG_S(INFO) << __FUNCTION__;

    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
        if(itr->ignore)
          {
            continue;
          }

        if(itr->type=="figure")
          {
            auto item = orig[(itr->path).first][(itr->path).second];

            subject<FIGURE> figure(doc_hash, *itr);
            bool valid_figure = figure.set_data(item);

            if(valid_figure)
              {
                itr->ignore=true;
                figures.push_back(figure);
              }
          }
      }

    for(auto& figure:figures)
      {
        auto mtext_ind = figure.provs.at(0).maintext_ind;

        auto ind_p1 = mtext_ind+1;
        if(ind_p1>=0 and ind_p1<provs.size() and
           (not provs.at(ind_p1).ignore) and
           provs.at(ind_p1).type=="caption")
          {
            auto caption_prov = provs.at(ind_p1);
            subject<PARAGRAPH> caption(doc_hash, provs.at(ind_p1));

            auto item = orig[caption_prov.path.first][caption_prov.path.second];
            bool valid = caption.set_data(item);
	    
            if(valid)
              {
                figure.captions.push_back(caption);
                provs.at(ind_p1).ignore = true;
              }
	    else
	      {
		LOG_S(WARNING) << "found invalid figure-caption: " << item.dump();
	      }	    
          }
      }
    
    return true;
  }

  bool subject<DOCUMENT>::init_paragraphs(std::vector<prov_element>& provs)
  {
    paragraphs.clear();
    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
        if(itr->type=="title" or
	   itr->type=="paragraph" or
	   itr->type=="subtitle-level-1" or
	   itr->type=="caption" or
	   itr->type=="footnote")
          {
            auto item = orig[(itr->path).first][(itr->path).second];

            subject<PARAGRAPH> subj(doc_hash, *itr);
            bool valid = subj.set_data(item);

            if(valid)
              {
                paragraphs.push_back(subj);
              }
          }
      }

    return true;
  }
  */
  
  bool subject<DOCUMENT>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                                     std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    //LOG_S(INFO) << __FUNCTION__;

    for(auto& paragraph:paragraphs)
      {
        bool valid = paragraph.set_tokens(char_normaliser, text_normaliser);

        if(not valid)
          {
            LOG_S(WARNING) << __FILE__ << ":" << __FUNCTION__
                           << " --> unvalid text detected in main-text";
          }
      }

    for(auto& table:tables)
      {
        bool valid = table.set_tokens(char_normaliser, text_normaliser);

        if(not valid)
          {
            LOG_S(WARNING) << __FILE__ << ":" << __FUNCTION__
                           << " --> unvalid text detected in table";
          }
      }

    for(auto& figure:figures)
      {
        bool valid = figure.set_tokens(char_normaliser, text_normaliser);

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
    //LOG_S(INFO) << __FUNCTION__;

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

    for(subject<PARAGRAPH>& paragraph:paragraphs)
      {
        for(auto& prop:paragraph.properties)
          {
            std::string mdl = prop.get_type();
            std::string lbl = prop.get_name();
            val_type   conf = prop.get_conf();
            val_type    dst = paragraph.dst;

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

        for(auto& ent:subj.entities)
          {
            entities.emplace_back(subj.get_hash(),
                                  subj.get_name(),
                                  subj.get_path(),
                                  ent);
          }
      }
    //LOG_S(INFO) << "total #-ents: " << entities.size();

    for(auto& subj:tables)
      {
        for(auto& ent:subj.entities)
          {
            entities.emplace_back(subj.get_hash(),
                                  subj.get_name(),
                                  subj.get_path(),
                                  ent);
          }
      }

    for(auto& subj:figures)
      {
        for(auto& ent:subj.entities)
          {
            entities.emplace_back(subj.get_hash(),
                                  subj.get_name(),
                                  subj.get_path(),
                                  ent);
          }
      }

    return true;
  }

  bool subject<DOCUMENT>::finalise_relations()
  {
    relations.clear();

    for(subject<PARAGRAPH>& paragraph:paragraphs)
      {
        for(auto& rel:paragraph.relations)
          {
            relations.push_back(rel);
          }
      }

    for(subject<TABLE>& table:tables)
      {
        for(auto& rel:table.relations)
          {
            relations.push_back(rel);
          }
      }

    return true;
  }

}

#endif
