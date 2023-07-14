//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_DOC_NORMALISATION_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_DOC_NORMALISATION_H_

namespace andromeda
{

  template<typename doc_type>
  class doc_normalisation: public base_types
  {
    const static inline std::set<std::string> is_ignored = {"page-header", "page-footer"};

    const static inline std::set<std::string> is_text = {
      "title", "subtitle-level-1", "paragraph",
      "footnote", "caption",
      "formula", "equation"
    };

    const static inline std::set<std::string> is_table = {"table"};
    const static inline std::set<std::string> is_figure = {"figure"};
    
  public:

    doc_normalisation(doc_type& doc);

    void execute_on_doc();
    
    void execute_on_pdf();

  private:

    void set_pdforder();

    void init_provs();

    void sort_provs();

    void init_items();

    void link_items();

    void resolve_paths();
    
  private:

    doc_type& doc;
  };

  template<typename doc_type>
  doc_normalisation<doc_type>::doc_normalisation(doc_type& doc):
    doc(doc)
  {}

  template<typename doc_type>
  void doc_normalisation<doc_type>::execute_on_doc()
  {
    LOG_S(WARNING);
    
    auto& orig = doc.orig;

    if(orig.count(doc_type::maintext_lbl)==0)
      {
        LOG_S(WARNING) << "no `main-text` identified";
        return;
      }

    auto& paragraphs = doc.paragraphs;
    auto& other = doc.other;

    auto& tables = doc.tables;
    auto& figures = doc.figures;
    
    {
      paragraphs.clear();
      other.clear();
      
      tables.clear();
      figures.clear();
    }

    std::string maintext_name_lbl = doc_type::maintext_name_lbl;
    std::string maintext_type_lbl = doc_type::maintext_type_lbl;
    
    //auto& main_text = orig.at(doc_type::maintext_lbl);
    for(std::size_t ind=0; ind<orig.at(doc_type::maintext_lbl).size(); ind++)
      {
	const nlohmann::json& item = orig.at(doc_type::maintext_lbl).at(ind);

        std::string name = item.at(maintext_name_lbl).get<std::string>();
        std::string type = item.at(maintext_type_lbl).get<std::string>();

	std::string ref = "#";
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
	    std::stringstream ss;
	    ss << "#/" << doc_type::maintext_lbl << "/" << ind;

	    ref = ss.str();
	  }

        if(is_text.count(type))
          {
            auto subj = std::make_shared<subject<PARAGRAPH> >(doc.doc_hash);
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
        else if(is_table.count(type))
          {
            auto subj = std::make_shared<subject<TABLE> >(doc.doc_hash);
            bool valid = subj->set_data(item);

	    tables.push_back(subj);

	    if(not valid)
	      {
                LOG_S(WARNING) << "found table without structure";
	      }
          }
        else if(is_figure.count(type))
          {
            auto subj = std::make_shared<subject<FIGURE> >(doc.doc_hash);
            bool valid = subj->set_data(item);

	    figures.push_back(subj);
	    
            if(not valid)
	      {
                LOG_S(WARNING) << "found figure without structure";
              }
          }
	else
	  {
            auto subj = std::make_shared<subject<PARAGRAPH> >(doc.doc_hash);
            bool valid = subj->set_data(item);

	    other.push_back(subj);
	    
            if(not valid)
              {
                LOG_S(WARNING) << "found invalid other-text: " << item.dump();
              }
	  }	
      }
  }
  
  template<typename doc_type>
  void doc_normalisation<doc_type>::execute_on_pdf()
  {
    LOG_S(WARNING);
    
    set_pdforder();

    init_provs();

    sort_provs();

    init_provs();
    
    init_items();

    link_items();

    resolve_paths();    
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::set_pdforder()
  {
    auto& orig = doc.orig;

    if(orig.count(doc_type::maintext_lbl)==0)
      {
        LOG_S(WARNING) << "no `main-text` identified";
        return;
      }

    auto& main_text = orig.at(doc_type::maintext_lbl);
    for(std::size_t pdforder=0; pdforder<main_text.size(); pdforder++)
      {
        main_text.at(pdforder)[doc_type::pdforder_lbl] = pdforder;
      }
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::init_provs()
  {
    auto& orig = doc.orig;
    auto& provs = doc.provs;

    provs.clear();

    std::string pdforder_lbl = doc_type::pdforder_lbl;    

    std::string maintext_name_lbl = doc_type::maintext_name_lbl;
    std::string maintext_type_lbl = doc_type::maintext_type_lbl;
    
    for(std::size_t l=0; l<orig.at(doc_type::maintext_lbl).size(); l++)
      {
	nlohmann::json item = orig.at(doc_type::maintext_lbl).at(l);

        std::stringstream ss;
        ss << "#/" << doc_type::maintext_lbl << "/" << l;

        std::string ref = ss.str();

        ind_type pdforder = item.at(pdforder_lbl).get<ind_type>();
        ind_type maintext = l;

        std::string name = item.at(maintext_name_lbl).get<std::string>();
        std::string type = item.at(maintext_type_lbl).get<std::string>();

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
                prov->set(ref_item[doc_type::prov_lbl][0]);

                provs.push_back(prov);
              }
            else
              {
                LOG_S(WARNING) << "undefined reference path in document: "
                               << prov->path;
              }
          }
        else if(item.count(doc_type::prov_lbl) and
                item[doc_type::prov_lbl].size()==1)
          {
            auto prov = std::make_shared<prov_element>(pdforder, maintext,
                                                       ref, name, type);

            prov->set(item[doc_type::prov_lbl][0]);

            provs.push_back(prov);
          }
        else
          {
            LOG_S(ERROR) << "undefined prov for main-text item: " << item.dump();
          }
      }
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::sort_provs()
  {
    doc_order sorter;
    sorter.order_maintext(doc);
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::init_items()
  {
    auto& orig = doc.orig;
    auto& provs = doc.provs;
    
    auto& paragraphs = doc.paragraphs;
    auto& other = doc.other;

    auto& tables = doc.tables;
    auto& figures = doc.figures;
    
    {
      paragraphs.clear();
      other.clear();
      
      tables.clear();
      figures.clear();
    }

    /*
    std::set<std::string> is_ignored = {"page-header", "page-footer"};

    std::set<std::string> is_text = {
      "title", "subtitle-level-1", "paragraph",
      "footnote", "caption",
      "formula", "equation"
    };

    std::set<std::string> is_table = {"table"};
    std::set<std::string> is_figure = {"figure"};
    */
    
    for(auto& prov:provs)
      {
        std::vector<std::string> parts = utils::split(prov->path, "/");

        std::string base = parts.at(1);
        std::size_t index = std::stoi(parts.at(2));

        auto& item = orig.at(base).at(index);

        if(is_text.count(prov->type))
          {
            auto subj = std::make_shared<subject<PARAGRAPH> >(doc.doc_hash, prov);
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
            auto subj = std::make_shared<subject<TABLE> >(doc.doc_hash, prov);
            bool valid = subj->set_data(item);

	    tables.push_back(subj);

	    if(not valid)
	      {
                LOG_S(WARNING) << "invalid table: "<< prov->path;
	      }
	    else
	      {
		//LOG_S(WARNING) << "valid table: " << prov->path;
	      }
          }
        else if(is_figure.count(prov->type))
          {
            auto subj = std::make_shared<subject<FIGURE> >(doc.doc_hash, prov);
            bool valid = subj->set_data(item);

	    figures.push_back(subj);
	    
            if(not valid)
	      {
                LOG_S(WARNING) << "found figure without structure";
              }
          }
        else
          {
            prov->ignore=true;
            if(not is_ignored.count(prov->type))
              {
                LOG_S(WARNING) << "found new `other` type: " << prov->type;
              }

            auto subj = std::make_shared<subject<PARAGRAPH> >(doc.doc_hash, prov);
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
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::link_items()
  {
    {
      doc_captions linker;
      linker.find_and_link_captions(doc);
    }

    {
      doc_maintext linker;

      linker.filter_maintext(doc);
      linker.concatenate_maintext(doc);
    }
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::resolve_paths()
  {
    auto& paragraphs = doc.paragraphs;
    auto& tables = doc.tables;
    auto& figures = doc.figures;    
    
    for(index_type l=0; l<paragraphs.size(); l++)
      {
	for(auto& prov:paragraphs.at(l)->provs)
	  {
	    std::stringstream ss;
	    ss << "#/" << doc_type::texts_lbl << "/" << l;

	    prov->path = ss.str();
	  }
      }

    for(index_type l=0; l<tables.size(); l++)
      {
	for(auto& prov:tables.at(l)->provs)
	  {
	    std::stringstream ss;
	    ss << "#/" << doc_type::tables_lbl << "/" << l;

	    prov->path = ss.str();
	  }

	for(index_type k=0; k<tables.at(l)->captions.size(); k++)
	  {
	    for(auto& prov:tables.at(l)->captions.at(k)->provs)
	      {
		std::stringstream ss;
		ss << "#/"
		   << doc_type::tables_lbl << "/" << l << "/"
		   << doc_type::captions_lbl << "/" << k;
		
		prov->path = ss.str();
	      }
	  }
      }

    for(index_type l=0; l<figures.size(); l++)
      {
	for(auto& prov:figures.at(l)->provs)
	  {
	    std::stringstream ss;
	    ss << "#/" << doc_type::figures_lbl << "/" << l;

	    prov->path = ss.str();
	  }

	for(index_type k=0; k<figures.at(l)->captions.size(); k++)
	  {
	    for(auto& prov:figures.at(l)->captions.at(k)->provs)
	      {
		std::stringstream ss;
		ss << "#/"
		   << doc_type::figures_lbl << "/" << l << "/"
		   << doc_type::captions_lbl << "/" << k;
		
		prov->path = ss.str();
	      }
	  }	
      }        
  }
  
}

#endif
