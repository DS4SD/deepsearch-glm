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
      "title", "subtitle-level-1", "paragraph", "list-item",
      "caption",
      "formula", "equation"
    };

    const static inline std::set<std::string> is_table = {"table"};
    const static inline std::set<std::string> is_figure = {"figure"};

    const static inline std::set<std::string> is_page_header = {"page-header"};
    const static inline std::set<std::string> is_page_footer = {"page-footer"};
    const static inline std::set<std::string> is_footnote = {"footnote"};

  public:

    doc_normalisation(doc_type& doc);

    void execute_on_pdf();

  private:

    void set_pdforder();

    void init_pages();

    void unroll_provs();

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
  void doc_normalisation<doc_type>::execute_on_pdf()
  {
    set_pdforder();

    init_pages();

    unroll_provs();

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
    auto& orig = doc.get_orig();

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
  void doc_normalisation<doc_type>::init_pages()
  {
    auto& orig = doc.get_orig();
    
    auto& pages = doc.get_pages();
    pages.clear();

    for(ind_type l=0; l<orig.at(doc_type::pages_lbl).size(); l++)
      {
        const nlohmann::json& item = orig.at(doc_type::pages_lbl).at(l);

        std::shared_ptr<page_element> ptr
          = std::make_shared<page_element>();

        ptr->from_json(item);

        pages.push_back(ptr);
      }
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::unroll_provs()
  {
    auto& orig = doc.get_orig();

    nlohmann::json& old_maintext = orig.at(doc_type::maintext_lbl);
    nlohmann::json new_maintext = nlohmann::json::array({});

    for(std::size_t l=0; l<old_maintext.size(); l++)
      {
        nlohmann::json item = old_maintext.at(l);

        if(item.count(doc_type::prov_lbl) and
	   item.count(doc_type::text_lbl) and
           item[doc_type::prov_lbl].size()>1)
          {
            std::string text = item.at(doc_type::text_lbl).template get<std::string>();

            for(auto prov_item:item[doc_type::prov_lbl])
              {
                nlohmann::json new_item = item;
                new_item[doc_type::prov_lbl] = nlohmann::json::array({});
                new_item[doc_type::prov_lbl].push_back(prov_item);

                std::array<int,2> rng = prov_item.at(doc_type::prov_span_lbl).template get<std::array<int,2> >();

		if(rng.at(1)<=text.size())
		  {
		    std::string new_text = text.substr(rng.at(0), rng.at(1)-rng.at(0));
		    
		    new_item[doc_type::text_lbl] = new_text;
		    
		    std::array<int,2> new_rng = {0, rng.at(1)-rng.at(0)};
		    new_item[doc_type::prov_lbl][0][doc_type::prov_span_lbl] = new_rng;
		    
		    //LOG_S(INFO) << " --> sub-item: " << new_item.dump();
		    
		    new_maintext.push_back(new_item);
		  }
		else if(rng.at(0)<=text.size()) // seems there is a small bug in the span ...
		  {
		    rng.at(1) = text.size();
		    
		    std::string new_text = text.substr(rng.at(0), rng.at(1)-rng.at(0));
		    
		    new_item[doc_type::text_lbl] = new_text;
		    
		    std::array<int,2> new_rng = {0, rng.at(1)-rng.at(0)};
		    new_item[doc_type::prov_lbl][0][doc_type::prov_span_lbl] = new_rng;
		    
		    new_maintext.push_back(new_item);
		  }
		else
		  {
		    LOG_S(ERROR) << "encountered illegal span in item: "
				 << item.dump();
		  }
	      }	    
          }
        else
          {
            new_maintext.push_back(item);
          }
      }

    orig.at(doc_type::maintext_lbl) = new_maintext;
  }

  template<typename doc_type>
  void doc_normalisation<doc_type>::init_provs()
  {
    //std::string doc_name = doc.doc_name;
    std::string doc_name = doc.get_name();

    auto& orig = doc.get_orig();
    auto& provs = doc.get_provs();

    provs.clear();

    std::string pdforder_lbl = doc_type::pdforder_lbl;

    std::string maintext_name_lbl = doc_type::maintext_name_lbl;
    std::string maintext_type_lbl = doc_type::maintext_type_lbl;

    for(std::size_t l=0; l<orig.at(doc_type::maintext_lbl).size(); l++)
      {
        nlohmann::json item = orig.at(doc_type::maintext_lbl).at(l);

        std::string path="";
        if(item.count("$ref"))
          {
            path = item["$ref"].get<std::string>();
          }
        else if(item.count("__ref"))
          {
            path = item["__ref"].get<std::string>();
          }
        else
          {
            std::stringstream ss;
            ss << "#/" << doc_type::maintext_lbl << "/" << l;

            path = ss.str();
          }

        ind_type pdforder = item.at(pdforder_lbl).get<ind_type>();
        ind_type maintext = l;

        std::string name = item.at(maintext_name_lbl).get<std::string>();
        std::string type = item.at(maintext_type_lbl).get<std::string>();

        auto prov = std::make_shared<prov_element>(pdforder, maintext,
                                                   path, name, type);

        std::vector<std::string> parts = utils::split(prov->get_item_ref(), "/");
        assert(parts.size()>=3);

        std::string base = parts.at(1);
        std::size_t index = std::stoi(parts.at(2));

        if(orig.count(base) and index<orig[base].size()) // make sure that the path exists in the orig document
          {
            auto& ref_item = orig[base][index];

            if(ref_item.count(doc_type::prov_lbl) and
               ref_item[doc_type::prov_lbl].size()==1)
              {
                prov->set(ref_item[doc_type::prov_lbl][0]);
                provs.push_back(prov);
              }
	    /*
	    else if(ref_item.count(doc_type::prov_lbl)==0)
	      {
		LOG_S(ERROR) << "missing 'prov' key in "
                             << item.dump();
	      }
	    */
            else
              {
                LOG_S(ERROR) << "undefined prov for main-text item: "
                             << item.dump();
              }
          }
        else
          {
            LOG_S(WARNING) << "undefined reference path in document: "
                           << item.dump();
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
    //std::string doc_name = doc.doc_name;
    std::string doc_name = doc.get_name();

    auto& orig = doc.get_orig();
    auto& provs = doc.get_provs();

    auto& texts = doc.texts;
    auto& tables = doc.tables;
    auto& figures = doc.figures;

    auto& page_headers = doc.page_headers;
    auto& page_footers = doc.page_footers;
    auto& footnotes = doc.footnotes;

    auto& other = doc.other;

    {
      texts.clear();
      tables.clear();
      figures.clear();

      page_headers.clear();
      page_footers.clear();
      footnotes.clear();

      other.clear();
    }

    for(uint64_t i=0; i<provs.size(); i++)
      {
        auto& prov = provs.at(i);

        // set a self-reference for later use after sorting ...
        {
          std::stringstream ss;
          ss << "#/" << doc_type::provs_lbl << "/" << i;
          prov->set_self_ref(ss.str());
        }

        std::vector<std::string> parts = utils::split(prov->get_item_ref(), "/");

        std::string base = parts.at(1);
        std::size_t index = std::stoi(parts.at(2));

        auto& item = orig.at(base).at(index);

        if(is_text.count(prov->get_type()))
          {
            std::stringstream ss;
            ss << doc_name << "#/" << doc_type::texts_lbl << "/" << texts.size();

            std::string dloc = ss.str();

            auto subj = std::make_shared<subject<TEXT> >(doc.get_hash(), dloc, prov);
            bool valid = subj->set_data(item);

            if(valid)
              {
                texts.push_back(subj);
              }
            else
              {
                LOG_S(WARNING) << "found invalid text: " << item.dump();
              }
          }
        else if(is_table.count(prov->get_type()))
          {
            std::stringstream ss;
            ss << doc_name << "#/" << doc_type::tables_lbl << "/" << tables.size();

            std::string dloc = ss.str();

            auto subj = std::make_shared<subject<TABLE> >(doc.get_hash(), dloc, prov);
            bool valid = subj->set_data(item);

            tables.push_back(subj);

            if(not valid)
              {
                //LOG_S(WARNING) << "invalid table: "<< prov->get_path();
              }
            else
              {
                //LOG_S(WARNING) << "valid table: " << prov->path;
              }
          }
        else if(is_figure.count(prov->get_type()))
          {
            std::stringstream ss;
            ss << doc_name << "#/" << doc_type::figures_lbl << "/" << figures.size();

            std::string dloc = ss.str();

            auto subj = std::make_shared<subject<FIGURE> >(doc.get_hash(), dloc, prov);
            bool valid = subj->set_data(item);

            figures.push_back(subj);

            if(not valid)
              {
                //LOG_S(WARNING) << "found figure without structure";
              }
          }
        else if(is_page_header.count(prov->get_type()))
          {
            std::stringstream ss;
            ss << doc_name << "#/" << doc_type::page_headers_lbl << "/" << page_headers.size();

            std::string dloc = ss.str();

            auto subj = std::make_shared<subject<TEXT> >(doc.get_hash(), dloc, prov);
            bool valid = subj->set_data(item);

            if(valid)
              {
                page_headers.push_back(subj);
              }
            else
              {
                LOG_S(WARNING) << "found invalid text: " << item.dump();
              }
          }
        else if(is_page_footer.count(prov->get_type()))
          {
            std::stringstream ss;
            ss << doc_name << "#/" << doc_type::page_footers_lbl << "/" << page_footers.size();

            std::string dloc = ss.str();

            auto subj = std::make_shared<subject<TEXT> >(doc.get_hash(), dloc, prov);
            bool valid = subj->set_data(item);

            if(valid)
              {
                page_footers.push_back(subj);
              }
            else
              {
                LOG_S(WARNING) << "found invalid text: " << item.dump();
              }
          }
        else if(is_footnote.count(prov->get_type()))
          {
            std::stringstream ss;
            ss << doc_name << "#/" << doc_type::footnotes_lbl << "/" << footnotes.size();

            std::string dloc = ss.str();

            auto subj = std::make_shared<subject<TEXT> >(doc.get_hash(), dloc, prov);
            bool valid = subj->set_data(item);

            if(valid)
              {
                footnotes.push_back(subj);
              }
            else
              {
                LOG_S(WARNING) << "found invalid text: " << item.dump();
              }
          }	
        else
          {
            prov->set_ignored(true);
            if(not is_ignored.count(prov->get_type()))
              {
                LOG_S(WARNING) << "found new `other` type: " << prov->get_type();
              }

            std::stringstream ss;
            ss << doc_name << "#/" << doc_type::meta_lbl << "/" << texts.size();

            std::string dloc = ss.str();

            auto subj = std::make_shared<subject<TEXT> >(doc.get_hash(), dloc, prov);
            bool valid = subj->set_data(item);

            if(valid)
              {
                other.push_back(subj);
              }
            else
              {
                LOG_S(WARNING) << "found invalid text: " << item.dump();
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
    auto& texts = doc.texts;

    auto& footnotes = doc.footnotes;
    auto& page_headers = doc.page_headers;
    auto& page_footers = doc.page_footers;
    auto& other = doc.other;
    
    auto& tables = doc.tables;
    auto& figures = doc.figures;

    for(index_type l=0; l<texts.size(); l++)
      {
	std::stringstream ss;
	ss << "#/" << doc_type::texts_lbl << "/" << l;

	texts.at(l)->set_self_ref(ss.str());
	
        for(auto& prov:texts.at(l)->provs)
          {
            prov->set_item_ref(ss.str());
          }
      }

    for(index_type l=0; l<footnotes.size(); l++)
      {
	std::stringstream ss;
	ss << "#/" << doc_type::footnotes_lbl << "/" << l;

	footnotes.at(l)->set_self_ref(ss.str());
	
        for(auto& prov:footnotes.at(l)->provs)
          {
            prov->set_item_ref(ss.str());
          }
      }

    for(index_type l=0; l<page_headers.size(); l++)
      {
	std::stringstream ss;
	ss << "#/" << doc_type::page_headers_lbl << "/" << l;

	page_headers.at(l)->set_self_ref(ss.str());
	
        for(auto& prov:page_headers.at(l)->provs)
          {
            prov->set_item_ref(ss.str());
          }
      }

    for(index_type l=0; l<page_footers.size(); l++)
      {
	std::stringstream ss;
	ss << "#/" << doc_type::page_footers_lbl << "/" << l;

	page_footers.at(l)->set_self_ref(ss.str());
	
        for(auto& prov:page_footers.at(l)->provs)
          {
            prov->set_item_ref(ss.str());
          }
      }

    for(index_type l=0; l<other.size(); l++)
      {
	std::stringstream ss;
	ss << "#/" << doc_type::other_lbl << "/" << l;

	other.at(l)->set_self_ref(ss.str());
	
        for(auto& prov:other.at(l)->provs)
          {
            prov->set_item_ref(ss.str());
          }
      }
    
    for(index_type l=0; l<tables.size(); l++)
      {
	std::stringstream ss;
	ss << "#/" << doc_type::tables_lbl << "/" << l;

	tables.at(l)->set_self_ref(ss.str());
	
        for(auto& prov:tables.at(l)->provs)
          {
            prov->set_item_ref(ss.str());
          }

        for(index_type k=0; k<tables.at(l)->captions.size(); k++)
          {
            for(auto& prov:tables.at(l)->captions.at(k)->provs)
              {
                std::stringstream ss;
                ss << "#/"
                   << doc_type::tables_lbl << "/" << l << "/"
                   << doc_type::captions_lbl << "/" << k;

                prov->set_item_ref(ss.str());

		tables.at(l)->captions.at(k)->set_self_ref(ss.str());
              }
          }
      }

    for(index_type l=0; l<figures.size(); l++)
      {
	std::stringstream ss;
	ss << "#/" << doc_type::figures_lbl << "/" << l;

	figures.at(l)->set_self_ref(ss.str());
	
        for(auto& prov:figures.at(l)->provs)
          {
            prov->set_item_ref(ss.str());
          }

        for(index_type k=0; k<figures.at(l)->captions.size(); k++)
          {
            for(auto& prov:figures.at(l)->captions.at(k)->provs)
              {
                std::stringstream ss;
                ss << "#/"
                   << doc_type::figures_lbl << "/" << l << "/"
                   << doc_type::captions_lbl << "/" << k;

                prov->set_item_ref(ss.str());

		figures.at(l)->captions.at(k)->set_self_ref(ss.str());
              }
          }
      }
  }

}

#endif
