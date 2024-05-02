//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_
#define ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_

namespace andromeda
{

  class prov_element: base_types
  {
  private:

    const static inline val_type eps = 1.0;
    
    const static inline int x0_ind = 0;
    const static inline int y0_ind = 1;
    const static inline int x1_ind = 2;
    const static inline int y1_ind = 3;
    
  public:
    
    prov_element();
    prov_element(ind_type pdforder_ind,
		 ind_type maintext_ind,
		 std::string name,
		 std::string type);

    prov_element(ind_type pdforder_ind,
		 ind_type maintext_ind,
		 std::string item_ref,
		 std::string name,
		 std::string type);

    prov_element(std::string item_ref,
		 std::string self_ref,
		 std::string name,
		 std::string type,
		 range_type char_range);
    
    static std::vector<std::string> get_headers();

    val_type x0() const;// { return bbox.at(x0_ind); }
    val_type x1() const;// { return bbox.at(x1_ind); }

    val_type y0() const;// { return bbox.at(y0_ind); }
    val_type y1() const;// { return bbox.at(y1_ind); }

    /* get/set methods */

    ind_type get_page() { return page; }

    ind_type get_maintext_ind() { return maintext_ind; }
    ind_type get_pdforder_ind() { return pdforder_ind; }
    
    std::string get_item_ref() { return item_ref; }
    void set_item_ref(std::string val) { item_ref = val; }
    
    std::string get_self_ref() { return self_ref; }
    void set_self_ref(std::string val) { self_ref = val; }
    
    std::string get_name() { return name; }
    void set_name(std::string val) { name = val; }
    
    std::string get_type() { return type; }
    void set_type(std::string val) { type = val; }
        
    std::array<ind_type, 2> get_char_range() { return char_range; }
    void set_char_range(std::array<ind_type, 2> cr) { char_range = cr;}
    
    bool is_ignored() { return ignore; }
    void set_ignored(bool val) { ignore = val; }
    
    std::array<val_type, 4> get_bbox();
    void set_bbox(std::array<val_type, 4> val) { bbox = val; }

    /* to/from json */    
    
    std::vector<std::string> to_row();

    nlohmann::json to_json(bool json_ref);
    nlohmann::json to_json_row();
    
    bool from_json(const nlohmann::json& item);
    
    bool follows_maintext_order(const prov_element& rhs) const;

    bool overlaps(const prov_element& rhs) const;
    
    bool overlaps_x(const prov_element& rhs) const;
    bool overlaps_y(const prov_element& rhs) const;
    bool overlaps_y(const prov_element& rhs, val_type iou) const;

    bool is_above(const prov_element& rhs) const;
    bool is_strictly_above(const prov_element& rhs) const;

    bool is_left_of(const prov_element& rhs) const;
    bool is_strictly_left_of(const prov_element& rhs) const;

    bool is_horizontally_connected(const prov_element& elem_i,
				   const prov_element& elem_j) const;
    
    friend bool operator<(const prov_element& lhs, const prov_element& rhs);
    
    void set(const nlohmann::json& data);

  private:
    
    ind_type pdforder_ind, maintext_ind;
    std::string name, type;

    std::string item_ref, self_ref;

    bool ignore;
    ind_type page;
    
    std::array<val_type, 2> dims; // (width, height) of page
    std::array<val_type, 4> bbox; // (x0, y0, x1, y1) with x0<x1 and y0<y1
    
    range_type char_range;
    range_type coor_range;    
  };
  
  prov_element::prov_element():
    pdforder_ind(-1),
    maintext_ind(-1),

    name("undef"),
    type("undef"),
    
    item_ref("#"),
    
    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  prov_element::prov_element(ind_type pdforder_ind,
			     ind_type maintext_ind,
			     std::string name,
			     std::string type):
    pdforder_ind(pdforder_ind),
    maintext_ind(maintext_ind),

    name(name),
    type(type),
    
    item_ref("#/main-text/"+std::to_string(maintext_ind)),
    
    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  prov_element::prov_element(ind_type pdforder_ind,
			     ind_type maintext_ind,
			     std::string item_ref,
			     std::string name,
			     std::string type):
    pdforder_ind(pdforder_ind),
    maintext_ind(maintext_ind),

    name(name),
    type(type),
    
    item_ref(item_ref),

    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  prov_element::prov_element(std::string item_ref,
			     std::string self_ref,
			     std::string name,
			     std::string type,
			     range_type rng):
    pdforder_ind(-1),
    maintext_ind(-1),

    name(name),
    type(type),
    
    item_ref(item_ref),
    self_ref(self_ref),

    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range(rng),
    coor_range({0,0})
  {}

  bool prov_element::follows_maintext_order(const prov_element& rhs) const
  {
    return (maintext_ind+1==rhs.maintext_ind);
  }

  typename prov_element::val_type prov_element::x0() const
  {
    return bbox.at(x0_ind);
  }
  
  typename prov_element::val_type prov_element::x1() const
  {
    return bbox.at(x1_ind);
  }
  
  typename prov_element::val_type prov_element::y0() const
  {
    return bbox.at(y0_ind);
  }
  
  typename prov_element::val_type prov_element::y1() const
  {
    return bbox.at(y1_ind);
  }

  /*
  void prov_element::set_bbox(val_type x0_val, val_type y0_val,
			      val_type x1_val, val_type y1_val)
  {
    bbox.at(x0_ind) = x0_val;
    bbox.at(y0_ind) = y0_val;
    bbox.at(x1_ind) = x1_val;
    bbox.at(y1_ind) = y1_val;
  }
  */
  
  bool prov_element::overlaps(const prov_element& rhs) const
  {
    return (overlaps_x(rhs) and overlaps_y(rhs));
  }
  
  bool prov_element::overlaps_x(const prov_element& rhs) const 
  {
    assert(page==rhs.page);
    //*
    return ((bbox[0]<=rhs.bbox[0] and rhs.bbox[0]<bbox[2]) or
	    (bbox[0]<=rhs.bbox[2] and rhs.bbox[2]<bbox[2]) or
	    (rhs.bbox[0]<=bbox[0] and bbox[0]<rhs.bbox[2]) or
	    (rhs.bbox[0]<=bbox[2] and bbox[2]<rhs.bbox[2]) );
    //*/

    //return (bbox.at(x0) < rhs.bbox.at(x1) and bbox.at(x1) > rhs.bbox.at(x0));
  }

  bool prov_element::overlaps_y(const prov_element& rhs) const
  {
    assert(page==rhs.page);
    //*
    return ((bbox[1]<=rhs.bbox[1] and rhs.bbox[1]<bbox[3]) or
	    (bbox[1]<=rhs.bbox[3] and rhs.bbox[3]<bbox[3]) or
	    (rhs.bbox[1]<=bbox[1] and bbox[1]<rhs.bbox[3]) or
	    (rhs.bbox[1]<=bbox[3] and bbox[3]<rhs.bbox[3]) );
    //*/

    //return (bbox.at(y0)<rhs.bbox.at(y1) and bbox.at(y1) > rhs.bbox.at(y0));
  }

  bool prov_element::overlaps_y(const prov_element& rhs, val_type iou) const
  {
    assert(page==rhs.page);
    assert(0.0<=iou and iou<=1.0);

    if(this->overlaps_y(rhs))
      {
	auto u0 = std::min(bbox[y0_ind], rhs.bbox[y0_ind]);
	auto u1 = std::max(bbox[y1_ind], rhs.bbox[y1_ind]);

	auto i0 = std::max(bbox[y0_ind], rhs.bbox[y0_ind]);
	auto i1 = std::min(bbox[y1_ind], rhs.bbox[y1_ind]);

	auto iou_ = (i1-i0)/(u1-u0);
	assert(0.0<=iou_ and iou_<=1.0);

	return (iou_)>iou;
      }

    return false;
  }
  
  bool prov_element::is_above(const prov_element& rhs) const
  {
    assert(page==rhs.page);

    return (bbox.at(y0_ind)>rhs.bbox.at(y0_ind));
  }

  bool prov_element::is_strictly_above(const prov_element& rhs) const
  {
    assert(page==rhs.page);

    return (bbox.at(y0_ind)+eps>rhs.bbox.at(y1_ind));
  }
  
  bool prov_element::is_left_of(const prov_element& rhs) const
  {
    assert(page==rhs.page);
    
    return (bbox.at(x0_ind)<rhs.bbox.at(x0_ind));
  }

  bool prov_element::is_strictly_left_of(const prov_element& rhs) const
  {
    assert(page==rhs.page);

    return (bbox.at(x1_ind)<rhs.bbox.at(x0_ind)+eps);
  }
  
  bool prov_element::is_horizontally_connected(const prov_element& elem_i,
					       const prov_element& elem_j) const
  {
    assert(page==elem_i.page);
    assert(page==elem_j.page);

    auto min_ij = std::min(elem_i.bbox[y0_ind], elem_j.bbox[y0_ind]);
    auto max_ij = std::max(elem_i.bbox[y1_ind], elem_j.bbox[y1_ind]);

    if(bbox.at(y0_ind)<max_ij and bbox.at(y1_ind)>min_ij) // overlap_y
      {
	return false;
      }
    
    /*
    if(bbox[y1] < std::min(elem_i.bbox[y0], elem_j.bbox[y0]))
      {
	return false;
      }
    
    if(bbox[y0] > std::max(elem_i.bbox[y1], elem_j.bbox[y1]))
      {	
	return false;
      }
    */
    
    if(bbox[x0_ind] < elem_i.bbox[x1_ind] and bbox[x1_ind] > elem_j.bbox[x0_ind])
      {	
	return true;
      }
    
    return false;
  }

  
  bool operator<(const prov_element& lhs, const prov_element& rhs)
  {
    if(lhs.page==rhs.page)
      {
	if(lhs.overlaps_x(rhs))
	  {
	    //auto lhs_ycm = (lhs.bbox.at(prov_element::y0)+lhs.bbox.at(prov_element::y1))/2.0;
	    //auto rhs_ycm = (rhs.bbox.at(prov_element::y0)+rhs.bbox.at(prov_element::y1))/2.0;
	    
	    //return (lhs_ycm>rhs_ycm);
	    return (lhs.bbox.at(prov_element::y0_ind)>rhs.bbox.at(prov_element::y0_ind));
	  }
	else
	  {
	    return (lhs.bbox.at(prov_element::x0_ind)<rhs.bbox.at(prov_element::x0_ind));
	  }
      }
    else
      {
	return (lhs.page<rhs.page);
      }
  }
  
  void prov_element::set(const nlohmann::json& data)
  {
    page = data.value("page", page);
    bbox = data.value("bbox", bbox);
    char_range = data.value("span", char_range);
  }

  bool prov_element::from_json(const nlohmann::json& item)
  {
    type = item.at("type").get<std::string>();
    name = item.at("name").get<std::string>();

    if(item.count("__ref"))
      {
	item_ref = item.at("__ref").get<std::string>();	
      }
    else if(item.count("$ref"))
      {
	item_ref = item.at("$ref").get<std::string>();	
      }
    else if(item.count("iref"))
      {
	item_ref = item.at("iref").get<std::string>();	
      } 
    else
      {
	item_ref = "#";
      }

    if(item.count("sref"))
      {
	self_ref = item.at("sref").get<std::string>();	
      }    
    
    page = item.at("page").get<ind_type>();
    bbox = item.at("bbox").get<std::array<val_type, 4> >();
    
    char_range = item.at("span").get<range_type>();

    if(item.count("text-order")==1)
      {
	maintext_ind = item.at("text-order");
      }
    
    if(item.count("orig-order")==1)
      {
	pdforder_ind = item.at("orig-order");
      }
    
    return true;
  }
  
  nlohmann::json prov_element::to_json(bool json_ref)
  {
    nlohmann::json result = nlohmann::json::object();

    if(item_ref!="" and json_ref)
      {
	result["$ref"] = item_ref;
	return result;
      }      

    result["iref"] = item_ref;
    result["sref"] = self_ref;
    
    result["type"] = type;
    result["name"] = name;
    
    result["page"] = page;
    result["bbox"] = bbox;
    result["span"] = char_range;

    result["text-order"] = maintext_ind;
    result["orig-order"] = pdforder_ind;
    
    return result;
  }

  std::vector<std::string> prov_element::get_headers()
  {
    static std::vector<std::string> row
      = { "maintext-ind", "pdforder-ind",
	  "item-ref", "self-ref",
	  "name", "type",
	  "page",
	  "x0", "y0", "x1", "y1"};
    return row;
  }


  
  std::vector<std::string> prov_element::to_row()
  {
    std::vector<std::string> row
      = { std::to_string(maintext_ind),
	  std::to_string(pdforder_ind),
	  item_ref,
	  self_ref,
	  name,
	  type,	  
	  std::to_string(page),

	  std::to_string(x0()),
	  std::to_string(y0()),
	  std::to_string(x1()),
	  std::to_string(y1()) };
    
    return row;
  }

  nlohmann::json prov_element::to_json_row()
  {
    nlohmann::json row = nlohmann::json::array();

    {
      row.push_back(maintext_ind);
      row.push_back(pdforder_ind);

      row.push_back(item_ref);
      row.push_back(self_ref);

      row.push_back(name);
      row.push_back(type);
      row.push_back(page);

      row.push_back(int(x0()));
      row.push_back(int(y0()));
      row.push_back(int(x1()));
      row.push_back(int(y1()));
    }
    assert(row.size()==prov_element::get_headers().size());
    
    return row;
  }
  
}

#endif
