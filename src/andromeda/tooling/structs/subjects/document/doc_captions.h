//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_DOC_CAPTIONS_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_DOC_CAPTIONS_H_

namespace andromeda
{
  class doc_captions: public base_types
  {

  public:

    doc_captions();

    template<typename doc_type>
    void find_and_link_captions(doc_type& doc);

  private:

    template<typename doc_type>
    void init_inds(doc_type& doc);

    template<typename doc_type>
    void link_captions(doc_type& doc);

    template<typename doc_type>
    void assign_captions(doc_type& doc);

    template<typename doc_type>
    void remove_captions_from_maintext(doc_type& doc);

  private:

    std::map<ind_type, std::vector<ind_type> > obj_to_caption;
    std::map<ind_type, std::vector<ind_type> > obj_to_notes;

    std::set<ind_type> page_nums, is_assigned;

    // mappings based on prov.type
    std::map<ind_type, std::set<ind_type> > page_inds, obj_inds={}, table_inds={}, figure_inds={},
      capt_inds={}, text_inds={}, note_inds={};

    // mappings based on text (starts_with `tab` or `fig`)
    std::map<ind_type, std::set<ind_type> > table_caption_inds={}, figure_caption_inds={};

    std::map<std::shared_ptr<prov_element>, index_type> prov_to_index={};

    std::map<std::shared_ptr<prov_element>, std::shared_ptr<subject<TEXT> > > prov_to_text={};
    std::map<std::shared_ptr<prov_element>, std::shared_ptr<subject<TABLE> > > prov_to_table={};
    std::map<std::shared_ptr<prov_element>, std::shared_ptr<subject<FIGURE> > > prov_to_figure={};
  };

  doc_captions::doc_captions()
  {}

  template<typename doc_type>
  void doc_captions::find_and_link_captions(doc_type& doc)
  {
    init_inds(doc);

    link_captions(doc);

    assign_captions(doc);

    remove_captions_from_maintext(doc);
  }

  template<typename doc_type>
  void doc_captions::init_inds(doc_type& doc)
  {
    obj_to_caption={};
    obj_to_notes={};

    auto& provs = doc.get_provs();

    page_nums={};
    is_assigned={};

    // find page-nums
    for(ind_type i=0; i<provs.size(); i++)
      {
        auto prov_i = provs.at(i);
        ind_type page = prov_i->get_page();

        page_nums.insert(page);
      }

    // initialise indices
    for(auto page_num:page_nums)
      {
        page_inds[page_num]={};
        obj_inds[page_num]={};

        table_inds[page_num]={};
        figure_inds[page_num]={};

        text_inds[page_num]={};
        capt_inds[page_num]={};
        note_inds[page_num]={};

        table_caption_inds[page_num]={};
        figure_caption_inds[page_num]={};
      }

    // populate indices
    for(ind_type i=0; i<provs.size(); i++)
      {
        auto prov_i = provs.at(i);
        ind_type page_num = prov_i->get_page();

        page_inds.at(page_num).insert(i);

        prov_to_index[provs.at(i)] = i;

        if(provs.at(i)->get_type()=="table")
          {
            obj_inds.at(page_num).insert(i);
            table_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->get_type()=="figure")
          {
            obj_inds.at(page_num).insert(i);
            figure_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->get_type()=="paragraph")
          {
            text_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->get_type()=="caption")
          {
            capt_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->get_type()=="footnote")
          {
            note_inds.at(page_num).insert(i);
          }
        else
          {}
      }

    for(auto text:doc.texts)
      {
	for(auto& prov:text->provs)
	  {
	    prov_to_text[prov] = text;
	  }
      }

    for(auto table:doc.tables)
      {
	for(auto& prov:table->provs)
	  {
	    prov_to_table[prov] = table;
	  }
      }

    for(auto figure:doc.figures)
      {
	for(auto& prov:figure->provs)
	  {
	    prov_to_figure[prov] = figure;
	  }
      }        
    
    for(auto& elem:doc.texts)
      {	
        auto& prov = elem->provs.at(0);

        ind_type prov_ind = prov_to_index.at(prov);
        ind_type page_num = prov->get_page();

        std::string text = elem->get_text();

        text = utils::to_lower(text);
        text = utils::strip(text);

        if(text.starts_with("tab"))
          {
            table_caption_inds.at(page_num).insert(prov_ind);
          }
        else if(text.starts_with("fig"))
          {
            figure_caption_inds.at(page_num).insert(prov_ind);
          }
        else
          {}
      }

    for(auto itr_i=obj_inds.begin(); itr_i!=obj_inds.end(); itr_i++)
      {
        for(ind_type ind:itr_i->second)
          {
            obj_to_caption[ind]={};
            obj_to_notes[ind]={};
          }
      }
  }

  template<typename doc_type>
  void doc_captions::link_captions(doc_type& doc)
  {
    for(auto page_num:page_nums)
      {
        // nothing to link on this page
        if(obj_inds.at(page_num).size()==0)
          {
            continue;
          }

        // find the captions closest to the object (= table or figure)
        for(auto ind:obj_inds.at(page_num))
          {
            auto ind_m1 = ind-1;
            auto ind_p1 = ind+1;

            // ind_m1 is on same page AND is_caption AND is_not_assigned yet
            if(page_inds.at(page_num).count(ind_m1)==1 and
               capt_inds.at(page_num).count(ind_m1)==1 and
               is_assigned.count(ind_m1)==0)
              {
                obj_to_caption.at(ind).push_back(ind_m1);
                is_assigned.insert(ind_m1);
              }

            // ind_p1 is on same page AND is_caption AND is_not_assigned yet
            if(page_inds.at(page_num).count(ind_p1)==1 and
               capt_inds.at(page_num).count(ind_p1)==1 and
               is_assigned.count(ind_p1)==0)
              {
                obj_to_caption.at(ind).push_back(ind_p1);
                is_assigned.insert(ind_p1);
              }
          }
      }

    for(auto itr=obj_inds.begin(); itr!=obj_inds.end(); itr++)
      {
        ind_type page_num = itr->first;

        for(ind_type ind:itr->second)
          {
            if(obj_to_caption.at(ind).size()>0)
              {
                continue;
              }

            if(table_inds.at(page_num).count(ind))
              {
                for(auto cind:table_caption_inds.at(page_num))
                  {
                    if(is_assigned.count(cind)==0)
                      {
                        obj_to_caption.at(ind) = {cind};
                        is_assigned.insert(cind);
                      }
                  }
              }
            else if(figure_inds.at(page_num).count(ind))
              {
                for(auto cind:figure_caption_inds.at(page_num))
                  {
                    if(is_assigned.count(cind)==0)
                      {
                        obj_to_caption.at(ind) = {cind};
                        is_assigned.insert(cind);
                      }
                  }
              }
            else
              {}
          }
      }
  }

  template<typename doc_type>
  void doc_captions::assign_captions(doc_type& doc)
  {
    auto& provs = doc.get_provs();

    for(auto itr=obj_to_caption.begin(); itr!=obj_to_caption.end(); itr++)
      {
        auto& prov_i = provs.at(itr->first);

	if(prov_i->get_type()=="table" and prov_to_table.count(prov_i)==0)
	  {
	    LOG_S(WARNING) << "no table associated with prov: "
			   << prov_i->to_json(false).dump();
	    continue;
	  }
	else if(prov_i->get_type()=="figure" and prov_to_figure.count(prov_i)==0)
	  {
	    LOG_S(WARNING) << "no figure associated with prov: "
			   << prov_i->to_json(false).dump();
	    continue;
	  }
	else
	  {}
	
        if(prov_i->get_type()=="table")
          {
	    auto& table = prov_to_table.at(prov_i);

            for(ind_type j:itr->second)
              {
                auto& prov_j = provs.at(j);
		auto& caption = prov_to_text.at(prov_j);

                table->captions.push_back(caption);
              }
          }
        else if(prov_i->get_type()=="figure")
          {
	    auto& figure = prov_to_figure.at(prov_i);
	    
            for(ind_type j:itr->second)
              {
                auto& prov_j = provs.at(j);
		auto& caption = prov_to_text.at(prov_j);

                figure->captions.push_back(caption);
              }
          }
	else
	  {}
      }
  }

  template<typename doc_type>
  void doc_captions::remove_captions_from_maintext(doc_type& doc)
  {
    std::set<std::shared_ptr<prov_element> > captions={};
    
    for(auto table:doc.tables)
      {
	for(auto& prov:table->provs)
	  {
	    //prov_to_table[prov] = table;
	    captions.insert(prov);
	  }
      }

    for(auto figure:doc.figures)
      {
	for(auto& prov:figure->provs)
	  {
	    //prov_to_figure[prov] = figure;
	    captions.insert(prov);
	  }
      }

    {
      auto itr=doc.texts.begin();
      while(itr!=doc.texts.end())
	{
	  bool is_caption=false;
	  for(auto& prov:(*itr)->provs)
	    {
	      if(captions.count(prov))
		{
		  is_caption=true;
		}
	    }

	  if(is_caption)
	    {
	      itr = doc.texts.erase(itr);
	    }
	  else
	    {
	      itr++;
	    }
	}
    }
  }
  
}

#endif
