//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_DOC_ORDER_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_DOC_ORDER_H_

namespace andromeda
{
  class doc_order: public base_types
  {
    typedef std::vector<prov_element> prov_vec_type;

    typedef std::map<ind_type,             ind_type  > ind_to_ind_type;
    typedef std::map<ind_type, std::vector<ind_type> > ind_to_vec_type;
    
  public:

    doc_order();

    template<typename doc_type>
    void order_maintext(doc_type& doc);

  private:

    template<typename doc_type>
    void update_document(doc_type& doc, prov_vec_type& provs);

    bool sort_provs(prov_vec_type& provs);

    prov_vec_type sort_page_provs(prov_vec_type& provs);

    void init_h2i_map(prov_vec_type& provs,
                      ind_to_ind_type& h2i_map,
                      ind_to_ind_type& i2h_map);

    void init_l2r_map(prov_vec_type& provs,
                      ind_to_ind_type& l2r_map,
                      ind_to_ind_type& r2l_map);

    void init_ud_maps(prov_vec_type& provs,
                      ind_to_ind_type& l2r_map,
                      ind_to_ind_type& r2l_map,
                      ind_to_vec_type& up_map,
                      ind_to_vec_type& dn_map);

    std::vector<ind_type> find_heads(prov_vec_type& provs,
                                     ind_to_ind_type& h2i_map,
                                     ind_to_ind_type& i2h_map,
                                     ind_to_vec_type& up_map,
                                     ind_to_vec_type& dn_map);

    void sort_ud_maps(prov_vec_type& provs,
                      ind_to_ind_type& h2i_map,
                      ind_to_ind_type& i2h_map,
                      ind_to_vec_type& up_map,
                      ind_to_vec_type& dn_map);

    std::vector<ind_type> find_order(prov_vec_type& provs,
                                     std::vector<ind_type>& heads,
                                     ind_to_vec_type& up_map,
                                     ind_to_vec_type& dn_map);

    ind_type depth_first_search_upwards(ind_type node_ind,
					std::vector<ind_type>& order,
					std::vector<bool>& visited,
					ind_to_vec_type& dn_map,
					ind_to_vec_type& up_map);
    
    void depth_first_search_downwards(ind_type node_ind,
				      std::vector<ind_type>& order,
				      std::vector<bool>& visited,
				      ind_to_vec_type& dn_map,
				      ind_to_vec_type& up_map);
  };

  doc_order::doc_order()
  {}

  template<typename doc_type>
  void doc_order::order_maintext(doc_type& doc)
  {
    // make a deep-copy !
    prov_vec_type provs={};
    for(auto& prov:doc.provs)
      {
        provs.push_back(*prov);
      }

    sort_provs(provs);

    update_document(doc, provs);
  }

  template<typename doc_type>
  void doc_order::update_document(doc_type& doc, prov_vec_type& provs)
  {
    // copy ...
    nlohmann::json maintext = doc.orig["main-text"];

    // re-order
    //prov_vec_type& provs = doc.provs;
    for(std::size_t l=0; l<provs.size(); l++)
      {
        maintext.at(l) = doc.orig["main-text"][provs.at(l).maintext_ind];
      }

    // overwrite ...
    doc.orig["main-text"] = maintext;

    // reset the provs ...
    doc.init_provs();
  }

  bool doc_order::sort_provs(prov_vec_type& provs)
  {
    //LOG_S(WARNING) << __FUNCTION__;

    std::map<ind_type, prov_vec_type> page_provs={};

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
        prov_vec_type& local = itr->second;

        prov_vec_type order = sort_page_provs(local);

        for(auto& item:order)
          {
            provs.push_back(item);
          }
      }

    return true;
  }

  typename doc_order::prov_vec_type doc_order::sort_page_provs(prov_vec_type& provs)
  {
    std::size_t N=provs.size();

    if(N<2)
      {
        return provs;
      }

    // hash-to-pageindex
    ind_to_ind_type h2i_map={}, i2h_map={};
    init_h2i_map(provs, h2i_map, i2h_map);

    // left-to-right from PDF order
    ind_to_ind_type l2r_map={}, r2l_map={};
    init_l2r_map(provs, l2r_map, r2l_map);

    ind_to_vec_type up_map={}, dn_map={};
    init_ud_maps(provs, l2r_map, r2l_map, up_map, dn_map);

    std::vector<ind_type> heads = find_heads(provs, h2i_map, i2h_map, up_map, dn_map);

    sort_ud_maps(provs, h2i_map, i2h_map, up_map, dn_map);

    std::vector<ind_type> order = find_order(provs, heads, up_map, dn_map);

    prov_vec_type result={};
    for(auto ind:order)
      {
        result.push_back(provs.at(ind));
      }

    return result;
  }

  void doc_order::init_h2i_map(prov_vec_type& provs,
                               ind_to_ind_type& h2i_map,
                               ind_to_ind_type& i2h_map)
  {
    // hash-to-pageindex
    for(std::size_t i=0; i<provs.size(); i++)
      {
        auto h = provs.at(i).maintext_ind;

        h2i_map[h] = i;
        i2h_map[i] = h;
      }
  }

  void doc_order::init_l2r_map(prov_vec_type& provs,
                               ind_to_ind_type& l2r_map,
                               ind_to_ind_type& r2l_map)
  {
    for(ind_type i=0; i<provs.size(); i++)
      {
        for(ind_type j=0; j<provs.size(); j++)
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

  void doc_order::init_ud_maps(prov_vec_type& provs,
                               ind_to_ind_type& l2r_map,
                               ind_to_ind_type& r2l_map,
                               ind_to_vec_type& up_map,
                               ind_to_vec_type& dn_map)
  {
    for(ind_type ind=0; ind<provs.size(); ind++)
      {
        dn_map[ind]={};
        up_map[ind]={};
      }

    for(ind_type j=0; j<provs.size(); j++)
      {
        if(r2l_map.count(j)==1)
          {
            ind_type i = r2l_map.at(j);

            dn_map[i] = {j};
            up_map[j] = {i};

            continue;
          }

        auto& prov_j = provs.at(j);

        for(ind_type i=0; i<provs.size(); i++)
          {
            if(i==j)
              {
                continue;
              }

            auto& prov_i = provs.at(i);

            bool is_horizontally_connected=false;
            bool is_i_just_above_j = (prov_i.overlaps_x(prov_j) and prov_i.is_strictly_above(prov_j));

            for(ind_type w=0; w<provs.size(); w++)
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

  std::vector<base_types::ind_type> doc_order::find_heads(prov_vec_type& provs,
                                                          ind_to_ind_type& h2i_map,
                                                          ind_to_ind_type& i2h_map,
                                                          ind_to_vec_type& up_map,
                                                          ind_to_vec_type& dn_map)
  {
    std::vector<ind_type> heads = {};

    prov_vec_type head_provs={};
    for(auto& item:up_map)
      {
        if(item.second.size()==0)
          {
            head_provs.push_back(provs.at(item.first));
          }
      }

    std::sort(head_provs.begin(), head_provs.end());

    for(auto& item:head_provs)
      {
        //LOG_S(INFO) << item.maintext_ind << ": " << h2i_map.count(item.maintext_ind);
        heads.push_back(h2i_map.at(item.maintext_ind));
      }

    return heads;
  }

  void doc_order::sort_ud_maps(prov_vec_type& provs,
                               ind_to_ind_type& h2i_map,
                               ind_to_ind_type& i2h_map,
                               ind_to_vec_type& up_map,
                               ind_to_vec_type& dn_map)
  {
    for(auto& item:dn_map)
      {
        prov_vec_type child_provs={};
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

  std::vector<base_types::ind_type> doc_order::find_order(prov_vec_type& provs,
                                                          std::vector<ind_type>& heads,
                                                          ind_to_vec_type& up_map,
                                                          ind_to_vec_type& dn_map)
  {
    std::vector<ind_type> order={};

    std::vector<bool> visited(provs.size(), false);

    for(auto& j:heads)
      {
        if(not visited.at(j))
          {
	    LOG_S(WARNING) << "register : " << j;

	    order.push_back(j);
	    visited.at(j) = true;
	    
            depth_first_search_downwards(j, order, visited, dn_map, up_map);
          }
      }
    assert(order.size()==provs.size());

    return order;
  }

  typename doc_order::ind_type doc_order::depth_first_search_upwards(ind_type j,
								     std::vector<ind_type>& order,
								     std::vector<bool>& visited,
								     ind_to_vec_type& dn_map,
								     ind_to_vec_type& up_map)
  {
    LOG_S(INFO) << " -> up: " << j;
    
    ind_type k=j;
    
    auto& inds = up_map.at(j);
    for(auto ind:inds)
      {
	if(not visited.at(ind))
	  {
	    return depth_first_search_upwards(ind, order, visited, dn_map, up_map);
	  }
      }
    
    return k;
  }
  
  void doc_order::depth_first_search_downwards(ind_type j,
					       std::vector<ind_type>& order,
					       std::vector<bool>& visited,
					       ind_to_vec_type& dn_map,
					       ind_to_vec_type& up_map)
  {
    LOG_S(INFO) << " -> dn: " << j;
    
    std::vector<ind_type>& inds = dn_map.at(j);

    for(auto& i:inds)
      {
	ind_type k = depth_first_search_upwards(i, order, visited, dn_map, up_map);
	
        if(not visited.at(k))
          {
	    LOG_S(WARNING) << "register : " << k;
	    
            order.push_back(k);
            visited.at(k) = true;

            depth_first_search_downwards(k, order, visited, dn_map, up_map);
          }
      }
  }

}

#endif
