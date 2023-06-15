//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_DOC_LINKER_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_DOC_LINKER_H_

namespace andromeda
{
  class doc_linker: public base_types
  {

  public:

    doc_linker();

    template<typename doc_type>
    void find_captions(doc_type& doc);

  private:

    template<typename doc_type>
    void init_inds(doc_type& doc);
    
    template<typename doc_type>
    void link_captions(doc_type& doc);

    template<typename doc_type>
    void assign_captions(doc_type& doc);

  private:

    std::map<ind_type, std::vector<ind_type> > obj_to_caption;
    std::map<ind_type, std::vector<ind_type> > obj_to_notes;


    std::set<ind_type> page_nums, is_assigned;

    // mappings based on prov.type
    std::map<ind_type, std::set<ind_type> > page_inds, obj_inds={}, table_inds={}, figure_inds={},
      capt_inds={}, text_inds={}, note_inds={};

    // mappings based on text (starts_with `tab` or `fig`)
    std::map<ind_type, std::set<ind_type> > table_caption_inds={}, figure_caption_inds={};
  };

  doc_linker::doc_linker()
  {}

  template<typename doc_type>
  void doc_linker::find_captions(doc_type& doc)
  {
    init_inds(doc);

    link_captions(doc);

    assign_captions(doc);
  }

  template<typename doc_type>
  void doc_linker::init_inds(doc_type& doc)
  {
    obj_to_caption={};
    obj_to_notes={};
    
    auto& provs = doc.provs;

    page_nums={};
    is_assigned={};

    // find page-nums
    for(ind_type i=0; i<provs.size(); i++)
      {
        auto prov_i = provs.at(i);
        ind_type page = prov_i->page;

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
        ind_type page_num = prov_i->page;

        page_inds.at(page_num).insert(i);

        if(provs.at(i)->type=="table")
          {
            obj_inds.at(page_num).insert(i);
            table_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->type=="figure")
          {
            obj_inds.at(page_num).insert(i);
            figure_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->type=="paragraph")
          {
            text_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->type=="caption")
          {
            capt_inds.at(page_num).insert(i);
          }
        else if(provs.at(i)->type=="footnote")
          {
            note_inds.at(page_num).insert(i);
          }
        else
          {}

        if(prov_i->dref.first==PARAGRAPH)
          {
            std::string text = doc.paragraphs.at(prov_i->dref.second).text;

            text = utils::to_lower(text);
            text = utils::strip(text);

            if(text.starts_with("tab"))
              {
                table_caption_inds.at(page_num).insert(i);
              }
            else if(text.starts_with("fig"))
              {
                figure_caption_inds.at(page_num).insert(i);
              }
            else
              {}
          }
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
  void doc_linker::link_captions(doc_type& doc)
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
    
    /*
      for(auto ind:obj_inds)
      {
      auto& prov_i = doc.provs.at(ind);

      // skip if caption is already assigned
      if(obj_to_caption.at(ind).size()>0)
      {
      continue;
      }
      else if(table_inds.count(ind))
      {
      std::vector<ind_type> capts={};
      for(ind_type j=0; j<doc.provs.size(); j++)
      {
      auto prov_j = doc.provs.at(j);

      if(prov_i->dref.second==prov_j->dref.second and
      prov_j->dref.first==PARAGRAPH and taken.count(j)==0)
      {

      }
      }

      if(capts.size()==1)
      {
      obj_to_caption[ind] = {capts.at(0)};
      }

      }
      else if(figure_inds.count(ind))
      {
      std::vector<ind_type> capts={};
      for(ind_type j=0; j<doc.provs.size(); j++)
      {
      auto prov_j = doc.provs.at(j);

      if(prov_i->dref.second==prov_j->dref.second and
      prov_j->dref.first==PARAGRAPH and taken.count(j)==0)
      {
      std::string text = doc.paragraphs.at(prov_j->dref.second).text;

      text = utils::to_lower(text);
      text = utils::strip(text);

      if(text.starts_with("fig"))
      {
      capts.push_back(j);
      }
      }
      }

      if(capts.size()==1)
      {
      obj_to_caption[ind] = {capts.at(0)};
      }

      }
      else
      {

      }
    */
    /*
      auto ind_m1 = ind-1;
      auto ind_p1 = ind+1;

      auto prov = provs.at(ind);
      auto prov_m1 = provs.at(ind_m1);
      auto prov_p1 = provs.at(ind_p1);

      if(prov_m1->page==prov->page and
      prov_p1->page==prov->page)
      {
      std::string text_m1="", text_p1="";

      if(prov_m1->dref.second!=-1 and taken.count(ind_m1)==0)
      {
      text_m1 = doc.paragraphs.at(prov_m1->dref.second).text;
      }

      if(prov_p1->dref.second!=-1 and taken.count(ind_p1)==0)
      {
      text_p1 = doc.paragraphs.at(prov_p1->dref.second).text;
      }

      if(table_inds.count(ind)==1)
      {

      }
      else if(figure_inds.count(ind)==1)
      {

      }
      }
      else if(prov_m1->page==prov->page and
      text_inds.count(ind_m1)==1)
      {
      obj_to_caption[ind].push_back(ind_m1);
      taken.insert(ind_m1);
      }
      else if(prov_p1->page==prov->page)
      {
      obj_to_caption[ind].push_back(ind_p1);
      taken.insert(ind_p1);
      }
      else
      {
      LOG_S(ERROR) << "inconsistent state ...";
      }

      }
    */
  }

  template<typename doc_type>
  void doc_linker::assign_captions(doc_type& doc)
  {
    auto& provs = doc.provs;

    for(auto itr=obj_to_caption.begin(); itr!=obj_to_caption.end(); itr++)
      {
        auto& prov_i = provs.at(itr->first);

        switch(prov_i->dref.first)
          {
          case TABLE:
            {
              auto& table = doc.tables.at(prov_i->dref.second);

              LOG_S(WARNING) << "table: "
                             << prov_i->maintext_ind;

              for(ind_type j:itr->second)
                {
                  auto& prov_j = provs.at(j);

                  LOG_S(WARNING) << "\tassigning caption "
                                 << prov_i->maintext_ind
                                 << " to table "
                                 << prov_j->maintext_ind;

                  assert(prov_j->dref.first==PARAGRAPH);
                  table.captions.push_back(doc.paragraphs.at(prov_j->dref.second));
                }
            }
            break;

          case FIGURE:
            {
              auto& figure = doc.figures.at(prov_i->dref.second);

              LOG_S(WARNING) << "figure: "
                             << prov_i->maintext_ind;

              for(ind_type j:itr->second)
                {
                  auto& prov_j = provs.at(j);

                  LOG_S(WARNING) << "\tassigning caption "
                                 << prov_i->maintext_ind
                                 << " to figure "
                                 << prov_j->maintext_ind;

                  assert(prov_j->dref.first==PARAGRAPH);
                  figure.captions.push_back(doc.paragraphs.at(prov_j->dref.second));
                }
            }
            break;

          default:
            {}
          }
      }
  }


}

#endif
