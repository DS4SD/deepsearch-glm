//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_H_

namespace andromeda
{

  template<>
  class subject<DOCUMENT>: public base_subject
  {
  public:

    const static inline std::string mtexts_lbl = "main-text";
    const static inline std::string tables_lbl = "tables";
    const static inline std::string figures_lbl = "figures";

    const static inline std::string pdforder_lbl = "pdf-order";

    const static inline std::string prov_lbl = "prov";
    const static inline std::string text_lbl = "text";
    const static inline std::string data_lbl = "data";

    const static inline std::string mtext_name_lbl = "name";
    const static inline std::string mtext_type_lbl = "type";
    
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

    void init_provs(std::vector<prov_element>& provs);
    void show_provs(std::vector<prov_element>& provs);

  private:

    void set_orig(nlohmann::json& data);
    
    bool clean_provs(std::vector<prov_element>& provs); // remove all provs with ignore==true

    bool init_items();

    bool remove_headers_and_footers(std::vector<prov_element>& provs);

    bool init_tables(std::vector<prov_element>& provs);
    bool init_figures(std::vector<prov_element>& provs);
    bool init_paragraphs(std::vector<prov_element>& provs);

    bool finalise_properties();
    bool finalise_entities();
    bool finalise_relations();

  public:

    std::filesystem::path filepath;
    nlohmann::json orig;

    std::string doc_name;
    uint64_t doc_hash;
    
    std::vector<subject<PARAGRAPH> > paragraphs;
    std::vector<subject<TABLE> > tables;
    std::vector<subject<FIGURE> > figures;
  };

  subject<DOCUMENT>::subject():
    base_subject(DOCUMENT),

    doc_name(""),
    doc_hash(-1),

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

    if(result.count(mtexts_lbl))
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
                auto& item = result[prov.path.first][prov.path.second];

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
                auto& item = result[prov.path.first][prov.path.second];

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
                auto& item = result[prov.path.first][prov.path.second];

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

  bool subject<DOCUMENT>::set_data(std::filesystem::path filepath, nlohmann::json& data,
                                   bool update_maintext)
  {
    this->filepath = filepath;
    return set_data(data, update_maintext);
  }

  bool subject<DOCUMENT>::set_data(nlohmann::json& data, bool update_maintext)
  {
    //LOG_S(INFO) << __FUNCTION__;

    clear();

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

    {
      set_orig(data);
    }
    
    {
      reading_order sorter;
      sorter.order_maintext(*this, update_maintext);
    }

    {
      init_items();
    }

    return true;
  }

  void subject<DOCUMENT>::set_orig(nlohmann::json& data)
  {
    orig = data;

    if(orig.count(mtexts_lbl)==0)
      {
	LOG_S(WARNING) << "no `main-text` identified";
	return;
      }

    auto& main_text = orig.at(mtexts_lbl);
    for(std::size_t pdforder=0; pdforder<main_text.size(); pdforder++)
      {
	main_text.at(pdforder)[pdforder_lbl] = pdforder;
      }
  }
  
  void subject<DOCUMENT>::init_provs(std::vector<prov_element>& provs)
  {
    provs.clear();

    for(std::size_t l=0; l<orig[mtexts_lbl].size(); l++)
      {
        auto& item = orig[mtexts_lbl][l];

	ind_type pdforder = item[pdforder_lbl].get<ind_type>();
	ind_type maintext = l;

	std::string name = item[mtext_name_lbl].get<std::string>();
	std::string type = item[mtext_type_lbl].get<std::string>();
	
        if(item.count("$ref"))
          {
	    std::string ref = item["$ref"].get<std::string>();	    
	    
            prov_element prov(pdforder, maintext,
			      ref, name, type);

            if(orig.count(prov.path.first))
              {
                auto& ref_item = orig[prov.path.first][prov.path.second];
                prov.set(ref_item[prov_lbl][0]);

                provs.push_back(prov);
              }
            else
              {
                LOG_S(WARNING) << "undefined reference path in document: "
                               << prov.path.first;
              }
          }
        else if(item.count("__ref"))
          {
	    std::string ref = item["$ref"].get<std::string>();	    
	    
            prov_element prov(pdforder, maintext,
			      ref, name, type);

            if(orig.count(prov.path.first))
              {
                auto& ref_item = orig[prov.path.first][prov.path.second];
                prov.set(ref_item[prov_lbl][0]);

                provs.push_back(prov);
              }
            else
              {
                LOG_S(WARNING) << "undefined reference path in document: "
                               << prov.path.first;
              }
          }
        else if(item.count(prov_lbl) and
		item[prov_lbl].size()==1)
          {
            prov_element prov(pdforder, maintext, name, type);
            prov.set(item[prov_lbl][0]);

            provs.push_back(prov);
          }
        else
          {
            LOG_S(ERROR) << "undefined prov for main-text item: " << item.dump();
          }
      }
  }

  void subject<DOCUMENT>::show_provs(std::vector<prov_element>& provs)
  {
    //LOG_S(INFO) << __FUNCTION__;
    std::vector<std::string> headers = prov_element::headers();

    std::vector<std::vector<std::string> > rows={};
    for(auto& item:provs)
      {
        rows.push_back(item.to_row());
      }

    LOG_S(INFO) << "filepath: " << filepath << "\n\n"
                << utils::to_string(headers, rows, -1);
  }

  bool subject<DOCUMENT>::init_items()
  {
    //LOG_S(INFO) << __FUNCTION__;

    if(not orig.count(mtexts_lbl))
      {
        return false;
      }

    std::vector<prov_element> provs={};
    init_provs(provs);

    {
      remove_headers_and_footers(provs);
      //clean_provs(provs);
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

  bool subject<DOCUMENT>::init_tables(std::vector<prov_element>& provs)
  {
    //LOG_S(INFO) << __FUNCTION__;

    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
	/*
        if(itr->ignore)
          {
            continue;
          }

        else
	*/
	if(itr->type=="table")
          {
            subject<TABLE> table(doc_hash, *itr);

            auto item = orig[(itr->path).first][(itr->path).second];
            bool valid_table = table.set_data(item);

            if(not valid_table)
              {
		LOG_S(WARNING) << "found invalid table: " << item.dump();
	      }
	    
	    itr->ignore=true;
	    tables.push_back(table);
          }
      }

    LOG_S(WARNING) << "orig #-tables: " << orig["tables"].size();
    LOG_S(WARNING) << "list #-tables: " << tables.size();

    int cnt=0;
    
    // FIXME: we need to factor this out in another class
    for(auto& table:tables)
      {
        auto mtext_ind = table.provs.at(0).maintext_ind;

        auto ind_m1 = mtext_ind-1;
        auto ind_p1 = mtext_ind+1;

	LOG_S(INFO) << "table-" << (++cnt) << " => ("
		    << "m1: " << provs.at(ind_m1).type << "; "
		    << "p1: " << provs.at(ind_p1).type << ")";
	
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

	    LOG_S(INFO) << "table-caption text: " << caption.text;
	    
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
    //LOG_S(INFO) << __FUNCTION__;

    paragraphs.clear();
    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
        //if(itr->ignore)
	//{
	//continue;
	//}

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

  /*
    bool subject<DOCUMENT>::set(nlohmann::json& data,
    std::shared_ptr<utils::char_normaliser> char_normaliser,
    std::shared_ptr<utils::text_normaliser> text_normaliser)
    {
    //LOG_S(INFO) << __FUNCTION__;

    if(set_data(data, order_text))
    {
    return set_tokens(char_normaliser, text_normaliser);
    }

    return false;
    }

    bool subject<DOCUMENT>::set(std::filesystem::path filepath, nlohmann::json& data,
    std::shared_ptr<utils::char_normaliser> char_normaliser,
    std::shared_ptr<utils::text_normaliser> text_normaliser)
    {
    //LOG_S(INFO) << __FUNCTION__;

    if(set_data(filepath, data, order_text))
    {
    return set_tokens(char_normaliser, text_normaliser);
    }

    return false;
    }
  */

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
