//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_READING_ORDER_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_READING_ORDER_H_

namespace andromeda
{
  class reading_order: public base_types
  {

  public:

    reading_order();

    template<typename doc_type>
    void order_maintext(doc_type& doc, bool update);

    template<typename doc_type>
    void update_document(doc_type& doc, std::vector<prov_element>& provs);
    
  private:
    
    bool sort_provs(std::vector<prov_element>& provs);

    std::vector<prov_element> sort_page_provs(std::vector<prov_element>& provs);

    void init_h2i_map(std::vector<prov_element>& provs,
		      std::map<ind_type, ind_type>& h2i_map,
		      std::map<ind_type, ind_type>& i2h_map);

    void init_l2r_map(std::vector<prov_element>& provs,
		      std::map<ind_type, ind_type>& l2r_map,
		      std::map<ind_type, ind_type>& r2l_map);

    void init_ud_maps(std::vector<prov_element>& provs,
		      std::map<ind_type, ind_type>& l2r_map,
		      std::map<ind_type, ind_type>& r2l_map,
		      std::map<ind_type, std::vector<ind_type> >& up_map,
		      std::map<ind_type, std::vector<ind_type> >& dn_map);

    std::vector<ind_type> find_heads(std::vector<prov_element>& provs,
				     std::map<ind_type, ind_type>& h2i_map,
				     std::map<ind_type, ind_type>& i2h_map,
				     std::map<ind_type, std::vector<ind_type> >& up_map,
				     std::map<ind_type, std::vector<ind_type> >& dn_map);

    void sort_ud_maps(std::vector<prov_element>& provs,
		      std::map<ind_type, ind_type>& h2i_map,
		      std::map<ind_type, ind_type>& i2h_map,
		      std::map<ind_type, std::vector<ind_type> >& up_map,
		      std::map<ind_type, std::vector<ind_type> >& dn_map);
    
    std::vector<ind_type> find_order(std::vector<prov_element>& provs,
				     std::vector<ind_type>& heads,
				     std::map<ind_type, std::vector<ind_type> >& up_map,
				     std::map<ind_type, std::vector<ind_type> >& dn_map);
    
    void depth_first_search(ind_type node_ind,
                            std::vector<ind_type>& order,
                            std::vector<bool>& visited,
                            std::map<ind_type, std::vector<ind_type> >& dn_map);

  };

  reading_order::reading_order()
  {}

  template<typename doc_type>
  void reading_order::order_maintext(doc_type& doc, bool update)
  {
    LOG_S(INFO);
    
    std::vector<prov_element> provs={};
    doc.init_provs(provs);

    sort_provs(provs);

    if(update)
      {
	update_document(doc, provs);
      }
  }

  template<typename doc_type>
  void reading_order::update_document(doc_type& doc,
                                      std::vector<prov_element>& provs)
  {
    LOG_S(INFO);
    
    // copy ...
    nlohmann::json maintext = doc.orig["main-text"];

    for(std::size_t l=0; l<provs.size(); l++)
      {
        maintext.at(l) = doc.orig["main-text"][provs.at(l).maintext_ind];
        maintext.at(l)["pdf-order"] = provs.at(l).maintext_ind;
      }

    // overwrite ...
    doc.orig["main-text"] = maintext;
  }

  bool reading_order::sort_provs(std::vector<prov_element>& provs)
  {
    LOG_S(WARNING) << __FUNCTION__;

    std::map<std::size_t, std::vector<prov_element> > page_provs={};

    for(auto& prov:provs)
      {
        if(page_provs.count(prov.page))
          {
            page_provs.at(prov.page).push_back(prov);
          }
        else
          {
            page_provs[prov.page] = {prov};
          }
      }

    provs.clear();
    for(auto itr=page_provs.begin(); itr!=page_provs.end(); itr++)
      {
        std::vector<prov_element>& local = itr->second;

        std::vector<prov_element> order = sort_page_provs(local);

        for(auto& item:order)
          {
            provs.push_back(item);
          }
      }

    return true;
  }

  std::vector<prov_element> reading_order::sort_page_provs(std::vector<prov_element>& provs)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    std::size_t N=provs.size();

    if(N<2)
      {
        return provs;
      }

    // hash-to-pageindex
    std::map<ind_type, ind_type> h2i_map={}, i2h_map={};
    init_h2i_map(provs, h2i_map, i2h_map);
    
    // left-to-right from PDF order
    std::map<ind_type, ind_type> l2r_map={}, r2l_map={};
    init_l2r_map(provs, l2r_map, r2l_map);
    
    /*
    LOG_S(WARNING) << provs.at(0).page << ": " << l2r_map.size();
    for(auto& _:l2r_map)
      {
        LOG_S(INFO) << "\t" << _.first << " => " << _.second;
      }
    */
    
    std::map<ind_type, std::vector<ind_type> > up_map={}, dn_map={};
    init_ud_maps(provs, l2r_map, r2l_map, up_map, dn_map);

    /*
    LOG_S(INFO) << "page-edges: ";
    //for(auto& _:dn_map)
    for(std::size_t ind=0; ind<N; ind++)
      {
        LOG_S(INFO) << ind << ": " << up_map.at(ind).size() << "\t" << dn_map.at(ind).size();
        for(auto j:dn_map.at(ind))
          {
            LOG_S(INFO) << "\t" << ind << " -[dn]-> " << j;
          }

        for(auto j:up_map.at(ind))
          {
            LOG_S(INFO) << "\t" << ind << " -[up]-> " << j;
          }
      }
    */
    
    std::vector<ind_type> heads = find_heads(provs, h2i_map, i2h_map, up_map, dn_map);

    sort_ud_maps(provs, h2i_map, i2h_map, up_map, dn_map);

    std::vector<ind_type> order = find_order(provs, heads, up_map, dn_map);    

    std::vector<prov_element> result={};
    for(auto ind:order)
      {
        result.push_back(provs.at(ind));
      }

    return result;
  }

  void reading_order::init_h2i_map(std::vector<prov_element>& provs,
				   std::map<ind_type, ind_type>& h2i_map,
				   std::map<ind_type, ind_type>& i2h_map)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    // hash-to-pageindex
    for(std::size_t i=0; i<provs.size(); i++)
      {
        auto h = provs.at(i).maintext_ind;
        h2i_map[h] = i;
        i2h_map[i] = h;
      }
  }

  void reading_order::init_l2r_map(std::vector<prov_element>& provs,
				   std::map<ind_type, ind_type>& l2r_map,
				   std::map<ind_type, ind_type>& r2l_map)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    for(std::size_t i=0; i<provs.size(); i++)
      {
        for(std::size_t j=0; j<provs.size(); j++)
          {
            auto& prov_i = provs.at(i);
            auto& prov_j = provs.at(j);

            if(prov_i.follows_maintext_order(prov_j) and
               prov_i.is_strictly_left_of(prov_j) and
               //prov_i.overlaps_y(prov_j)
               prov_i.overlaps_y(prov_j, 0.8)
               )
              {
                l2r_map[i] = j;
                r2l_map[j] = i;
              }
          }
      }
  }

  void reading_order::init_ud_maps(std::vector<prov_element>& provs,
				   std::map<ind_type, ind_type>& l2r_map,
				   std::map<ind_type, ind_type>& r2l_map,
				   std::map<ind_type, std::vector<ind_type> >& up_map,
				   std::map<ind_type, std::vector<ind_type> >& dn_map)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    for(std::size_t ind=0; ind<provs.size(); ind++)
      {
        dn_map[ind]={};
        up_map[ind]={};
      }

    for(std::size_t j=0; j<provs.size(); j++)
      {
	if(r2l_map.count(j)==1)
          {
            std::size_t i = r2l_map.at(j);

            dn_map[i] = {j};
            up_map[j] = {i};

            continue;
          }

        auto& prov_j = provs.at(j);

        for(std::size_t i=0; i<provs.size(); i++)
          {
            if(i==j)
              {
                continue;
              }

            auto& prov_i = provs.at(i);

            bool is_horizontally_connected=false;
            bool is_i_just_above_j = (prov_i.overlaps_x(prov_j) and prov_i.is_strictly_above(prov_j));

            //LOG_S(WARNING) << "(" << i << ", " << j << "): " << prov_i.overlaps_x(prov_j) << "\t" << is_i_just_above_j;

            for(std::size_t w=0; w<provs.size(); w++)
              {
                auto& prov_w = provs.at(w);

                if(not is_horizontally_connected)
                  {
                    is_horizontally_connected = provs.at(w).is_horizontally_connected(prov_i, prov_j);
                  }

                // ensure there is no other element that is between i and j vertically
                if(is_i_just_above_j and (prov_i.overlaps_x(prov_w) or prov_j.overlaps_x(prov_w)))
                  {
                    bool i_above_w = prov_i.is_strictly_above(prov_w);
                    bool w_above_j = prov_w.is_strictly_above(prov_j);

                    is_i_just_above_j = (not (i_above_w and w_above_j));
                  }
              }

            if(is_i_just_above_j)
              {
                while(l2r_map.count(i))
                  {
                    i = l2r_map.at(i);
                  }

                dn_map.at(i).push_back(j);
                up_map.at(j).push_back(i);
              }
          }
      }    
  }  

  std::vector<base_types::ind_type> reading_order::find_heads(std::vector<prov_element>& provs,
							      std::map<ind_type, ind_type>& h2i_map,
							      std::map<ind_type, ind_type>& i2h_map,
							      std::map<ind_type, std::vector<ind_type> >& up_map,
							      std::map<ind_type, std::vector<ind_type> >& dn_map)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    std::vector<ind_type> heads = {};
      
    std::vector<prov_element> head_provs={};
    for(auto& item:up_map)
      {
	LOG_S(INFO) << item.first << ": " << item.second.size();

	if(item.second.size()==0)
	  {
	    head_provs.push_back(provs.at(item.first));
	  }
      }

    std::sort(head_provs.begin(), head_provs.end());
    
    for(auto& item:head_provs)
      {
	LOG_S(INFO) << item.maintext_ind << ": " << h2i_map.count(item.maintext_ind);
	heads.push_back(h2i_map.at(item.maintext_ind));
      }

    return heads;
  }
  
  void reading_order::sort_ud_maps(std::vector<prov_element>& provs,
				   std::map<ind_type, ind_type>& h2i_map,
				   std::map<ind_type, ind_type>& i2h_map,
				   std::map<ind_type, std::vector<ind_type> >& up_map,
				   std::map<ind_type, std::vector<ind_type> >& dn_map)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    for(auto& item:dn_map)
      {
	std::vector<prov_element> child_provs={};
	for(auto& ind:item.second)
	  {
	    child_provs.push_back(provs.at(ind));
	  }
	
	std::sort(child_provs.begin(), child_provs.end());
	
	item.second.clear();
	for(auto& child:child_provs)
	  {
	    item.second.push_back(h2i_map.at(child.maintext_ind));
	  }
      }
  }

  std::vector<base_types::ind_type> reading_order::find_order(std::vector<prov_element>& provs,
							      std::vector<ind_type>& heads,
							      std::map<ind_type, std::vector<ind_type> >& up_map,
							      std::map<ind_type, std::vector<ind_type> >& dn_map)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    std::vector<ind_type> order={};

    std::vector<bool> visited(provs.size(), false);

    for(auto& j:heads)
      {
        if(not visited.at(j))
          {
            LOG_S(INFO) << "head ind: " << j;

            order.push_back(j);
            visited.at(j) = true;

            depth_first_search(j, order, visited, dn_map);
          }
      }
    assert(order.size()==provs.size());
    
    return order;
  }
  
  void reading_order::depth_first_search(ind_type j,
                                         std::vector<ind_type>& order,
                                         std::vector<bool>& visited,
                                         std::map<ind_type, std::vector<ind_type> >& dn_map)
  {
    LOG_S(WARNING) << __FUNCTION__;
    
    std::vector<ind_type>& inds = dn_map.at(j);

    for(auto& i:inds)
      {
        if(not visited.at(i))
          {
            LOG_S(INFO) << " -> next ind: " << i;

            order.push_back(i);
            visited.at(i) = true;

            depth_first_search(i, order, visited, dn_map);
          }
      }
  }

}

#endif
