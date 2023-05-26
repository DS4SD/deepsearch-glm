//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_H_

namespace andromeda
{

  template<>
  class subject<DOCUMENT>: public base_subject
  {
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

    bool set_data(nlohmann::json& data);

    bool set_data(std::filesystem::path filepath,
                  nlohmann::json& data);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool set(nlohmann::json& data,
             std::shared_ptr<utils::char_normaliser> char_normaliser,
             std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool set(std::filesystem::path filepath, nlohmann::json& data,
             std::shared_ptr<utils::char_normaliser> char_normaliser,
             std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool finalise();
    
  private:

    void init_provs(std::vector<prov_element>& provs);
    bool clean_provs(std::vector<prov_element>& provs); // remove all provs with ignore==true
      
    void order_items();

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
    
    if(result.count("main-text"))
      {
        std::vector<std::string> keys
	  = { "hash", "orig", "text",
	      "properties", "entities", "relations"};

        for(std::size_t l=0; l<paragraphs.size(); l++)
          {   	    
            if(paragraphs.at(l).is_valid())
              {
		auto& paragraph = paragraphs.at(l);
		
		auto& prov = paragraph.provs.at(0);
                auto& item = result[prov.path.first][prov.path.first];
		
                auto _ = paragraph.to_json();
                for(auto& elem:_.items())
                  {
                    item[elem.key()] = elem.value();
                  }
              }
          }
      }

    if(result.count("tables"))
      {
        std::vector<std::string> keys
	  = { "hash", "captions", "footnotes", "mentions", 
	  "properties", "entities", "relations"};

        for(std::size_t l=0; l<tables.size(); l++)
          {
            if(tables.at(l).is_valid())
              {
		auto& table = tables.at(l);
		
		auto& prov = table.provs.at(0);
                auto& item = result[prov.path.first][prov.path.first];
		
                auto _ = table.to_json();
		for(auto& elem:_.items())
                  {
                    item[elem.key()] = elem.value();
                  }
	      }
          }
      }

    if(result.count("figures"))
      {
        std::vector<std::string> keys
	  = { "hash",  "captions", "footnotes", "mentions", 
	      "properties", "entities", "relations"};

        for(std::size_t l=0; l<figures.size(); l++)
          {
            if(figures.at(l).is_valid())
              {
		auto& figure = figures.at(l);
		
		auto& prov = figure.provs.at(0);
                auto& item = result[prov.path.first][prov.path.first];
		
                auto _ = figure.to_json();
                for(auto& elem:_.items())
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

  bool subject<DOCUMENT>::set_data(std::filesystem::path filepath, nlohmann::json& data)
  {
    this->filepath = filepath;
    return set_data(data);
  }

  bool subject<DOCUMENT>::set_data(nlohmann::json& data)
  {
    LOG_S(INFO) << __FUNCTION__;
    
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

    orig = data;

    order_items();

    init_items();

    return true;
  }

  void subject<DOCUMENT>::init_provs(std::vector<prov_element>& provs)
  {
    LOG_S(INFO) << __FUNCTION__;
    
    provs.clear();

    for(std::size_t l=0; l<orig["main-text"].size(); l++)
      {
        auto& item = orig["main-text"][l];

        if(item.count("$ref"))
          {
            prov_element prov(l, item["$ref"], item["name"], item["type"]);

	    if(orig.count(prov.path.first))
	      {
		auto& ref_item = orig[prov.path.first][prov.path.second];
		prov.set(ref_item["prov"][0]);
		
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
            prov_element prov(l, item["__ref"], item["name"], item["type"]);

	    if(orig.count(prov.path.first))
	      {
		auto& ref_item = orig[prov.path.first][prov.path.second];
		prov.set(ref_item["prov"][0]);
		
		provs.push_back(prov);
	      }
	    else
	      {
		LOG_S(WARNING) << "undefined reference path in document: "
			       << prov.path.first; 
	      }	    
	  }
        else if(item.count("prov") and item["prov"].size()==1)
          {
            prov_element prov(l, item["name"], item["type"]);
            prov.set(item["prov"][0]);

            provs.push_back(prov);
          }
        else
          {
            LOG_S(WARNING) << "undefined: " << item.dump();
          }
      }
  }

  void subject<DOCUMENT>::order_items()
  {
    LOG_S(INFO) << __FUNCTION__;
    
    if(orig.count("main-text")==0)
      {
        return;
      }

    std::vector<prov_element> provs={};
    init_provs(provs);

    /*
    {
      std::vector<std::string> headers = prov_element::headers();
      
      std::vector<std::vector<std::string> > rows={};
      for(auto& item:provs)
	{
	  rows.push_back(item.to_row());
	}
      
      LOG_S(INFO) << "filepath: " << filepath << "\n\n"
		  << utils::to_string(headers, rows, -1);
      
    }
    */
    
    //LOG_S(WARNING) << "sorting ... ";
    sort(provs.begin(), provs.end());

    /*
    {
      std::vector<std::string> headers = prov_element::headers();

      std::vector<std::vector<std::string> > rows={};
      for(auto& item:provs)
	{
	  rows.push_back(item.to_row());
	}
      
      LOG_S(INFO) << "sorted: " << "\n\n"
		  << utils::to_string(headers, rows, -1);

      std::string tmp;
      std::cin >> tmp;
    }
    */
    
    {
      // copy ...
      nlohmann::json maintext = orig["main-text"];
      for(std::size_t l=0; l<provs.size(); l++)
        {
          maintext[l] = orig["main-text"][provs.at(l).maintext_ind];
          maintext[l]["pdf-order"] = provs.at(l).maintext_ind;
        }

      // overwrite ...
      orig["main-text"] = maintext;
    }
  }

  bool subject<DOCUMENT>::init_items()
  {
    //LOG_S(INFO) << __FUNCTION__;
    
    if(not orig.count("main-text"))
      {
        return false;
      }

    std::vector<prov_element> provs={};
    init_provs(provs);

    /*
    {
      std::vector<std::string> headers = prov_element::headers();

      std::vector<std::vector<std::string> > rows={};
      for(auto& item:provs)
        {
          rows.push_back(item.to_row());
        }

      LOG_S(INFO) << "sorted: " << "\n\n"
                  << utils::to_string(headers, rows, -1);
    }
    */
    
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

    /*
    {
      std::vector<std::string> headers = prov_element::headers();

      std::vector<std::vector<std::string> > rows={};
      for(auto& item:provs)
        {
          rows.push_back(item.to_row());
        }

      LOG_S(INFO) << "sorted: " << "\n\n"
                  << utils::to_string(headers, rows, -1);
    }
    */
    
    {
      init_paragraphs(provs);
    }

    /*
    {
      for(auto& item:paragraphs)
        {
          item.show(true, false, false, false, false, false, false);
        }

      //for(auto& item:tables)
      //{
      //item.show(false, false, false);
      //}

      LOG_S(WARNING) << "set doc ...";
      std::string tmp;
      std::cin >> tmp;
    }
    */
    
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
    LOG_S(INFO) << __FUNCTION__;
    
    std::set<std::string> to_be_ignored={"page-header", "page-footer"};
    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
        itr->ignore = to_be_ignored.count(itr->type)? true:false;
      }

    return true;
  }

  bool subject<DOCUMENT>::init_tables(std::vector<prov_element>& provs)				          
  {
    LOG_S(INFO) << __FUNCTION__;
    
    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
	if(itr->ignore)
	  {
	    continue;
	  }
	
        else if(itr->type=="table")
          {
            subject<TABLE> table(doc_hash, *itr);

	    auto item = orig[(itr->path).first][(itr->path).second];
            bool valid_table = table.set_data(item);

            if(valid_table)
              {
		itr->ignore=true;
		tables.push_back(table);
              }
          }
      }

    for(auto& table:tables)
      {
	auto mtext_ind = table.provs.at(0).maintext_ind;

	auto caption_ind = mtext_ind-1;	
	if(caption_ind>=0 and caption_ind<provs.size() and
	   provs.at(caption_ind).type=="caption")
	  {
	    auto caption_prov = provs.at(caption_ind);
	    subject<PARAGRAPH> caption(doc_hash, provs.at(caption_ind));
	    
	    auto item = orig[caption_prov.path.first][caption_prov.path.second];
	    bool valid = caption.set_data(item);

	    if(valid)
	      {
		table.captions.push_back(caption);
		provs.at(caption_ind).ignore = true; 
	      }
	  }

	auto fnote_ind = mtext_ind+1;	
	if(fnote_ind>=0 and fnote_ind<provs.size() and
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
    LOG_S(INFO) << __FUNCTION__;
    
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

	auto caption_ind = mtext_ind+1;	
	if(caption_ind>=0 and caption_ind<provs.size() and
	   provs.at(caption_ind).type=="caption")
	  {
	    auto caption_prov = provs.at(caption_ind);
	    subject<PARAGRAPH> caption(doc_hash, provs.at(caption_ind));

	    auto item = orig[caption_prov.path.first][caption_prov.path.second];
	    bool valid = caption.set_data(item);

	    if(valid)
	      {
		figure.captions.push_back(caption);
		provs.at(caption_ind).ignore = true; 
	      }
	  }
      }
    
    return true;
  }

  bool subject<DOCUMENT>::init_paragraphs(std::vector<prov_element>& provs)
  {
    LOG_S(INFO) << __FUNCTION__;
    
    paragraphs.clear();
    for(auto itr=provs.begin(); itr!=provs.end(); itr++)
      {
	if(itr->ignore)
	  {
	    continue;
	  }
	
        if(itr->type=="paragraph" or
	   itr->type=="subtitle-level-1")
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
    LOG_S(INFO) << __FUNCTION__;
    
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

  bool subject<DOCUMENT>::set(nlohmann::json& data,
                              std::shared_ptr<utils::char_normaliser> char_normaliser,
                              std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    LOG_S(INFO) << __FUNCTION__;
    
    if(set_data(data))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }

  bool subject<DOCUMENT>::set(std::filesystem::path filepath, nlohmann::json& data,
                              std::shared_ptr<utils::char_normaliser> char_normaliser,
                              std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    LOG_S(INFO) << __FUNCTION__;
    
    if(set_data(filepath, data))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
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
	for(auto& ent:subj.entities)
	  {	    
	    entities.emplace_back(subj.get_hash(),
				  subj.get_name(),
				  subj.get_path(),
				  ent);
	  }
      }

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
