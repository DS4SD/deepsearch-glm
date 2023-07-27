//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_
#define ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_

namespace andromeda
{

  class prov_element: base_types
  {
  private:

    const static inline val_type eps = 1.0;
    
    const static inline int x0 = 0;
    const static inline int y0 = 1;
    const static inline int x1 = 2;
    const static inline int y1 = 3;

  public:
    
    prov_element();
    prov_element(ind_type pdforder_ind, ind_type maintext_ind, std::string name, std::string type);

    prov_element(ind_type pdforder_ind, ind_type maintext_ind,
		 std::string dref, std::string name, std::string type);

    static std::vector<std::string> get_headers();

    std::vector<std::string> to_row();

    nlohmann::json to_json();
    nlohmann::json to_json_row();

    bool from_json(const nlohmann::json& item);
    
    bool follows_maintext_order(const prov_element& rhs) const;
    
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

  public:
    
    ind_type pdforder_ind, maintext_ind;
    std::string name, type;

    std::string path;
    //std::pair<subject_name, ind_type> dref;

    bool ignore;
    ind_type page;
    
    std::array<float, 2> dims; // (width, height) of page
    std::array<float, 4> bbox; // (x0, y0, x1, y1) with x0<x1 and y0<y1
    
    range_type char_range;
    range_type coor_range;    
  };
  
  prov_element::prov_element():
    pdforder_ind(-1),
    maintext_ind(-1),

    name("undef"),
    type("undef"),
    
    path("#"),
    //dref(UNDEF, -1),
    
    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  prov_element::prov_element(ind_type pdforder_ind, ind_type maintext_ind,
			     std::string name, std::string type):
    pdforder_ind(pdforder_ind),
    maintext_ind(maintext_ind),

    name(name),
    type(type),
    
    path("#/main-text/"+std::to_string(maintext_ind)),
    //dref(UNDEF, -1),
    
    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  prov_element::prov_element(ind_type pdforder_ind,
			     ind_type maintext_ind,
			     std::string path,
			     std::string name,
			     std::string type):
    pdforder_ind(pdforder_ind),
    maintext_ind(maintext_ind),

    name(name),
    type(type),
    
    path(path),
    //dref(UNDEF, -1),

    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  //std::string prov_element::to_path() const
  //{
  //return path;
  //}

  /*
  std::pair<std::string, typename prov_element::ind_type> prov_element::from_path(std::string doc_path)
  {
    std::vector<std::string> parts = utils::split(doc_path, "/");
    //assert(parts.size()==3 and parts.at(0)=="#");
    
    //return std::pair<std::string, ind_type>{parts.at(1), std::stoi(parts.at(2))};
  }
  */
  
  /*
  std::string prov_element::to_dref() const
  {
    std::stringstream ss;
    ss << "#" << "/" << to_string(dref.first) << "/" << path.second;
    
    return ss.str();
  }
  */
  
  bool prov_element::follows_maintext_order(const prov_element& rhs) const
  {
    return (maintext_ind+1==rhs.maintext_ind);
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
	auto u0 = std::min(bbox[y0], rhs.bbox[y0]);
	auto u1 = std::max(bbox[y1], rhs.bbox[y1]);

	auto i0 = std::max(bbox[y0], rhs.bbox[y0]);
	auto i1 = std::min(bbox[y1], rhs.bbox[y1]);

	auto iou_ = (i1-i0)/(u1-u0);
	assert(0.0<=iou_ and iou_<=1.0);

	return (iou_)>iou;
      }

    return false;
  }
  
  bool prov_element::is_above(const prov_element& rhs) const
  {
    assert(page==rhs.page);

    return (bbox.at(y0)>rhs.bbox.at(y0));
  }

  bool prov_element::is_strictly_above(const prov_element& rhs) const
  {
    assert(page==rhs.page);

    return (bbox.at(y0)+eps>rhs.bbox.at(y1));
  }
  
  bool prov_element::is_left_of(const prov_element& rhs) const
  {
    assert(page==rhs.page);
    
    return (bbox.at(x0)<rhs.bbox.at(x0));
  }

  bool prov_element::is_strictly_left_of(const prov_element& rhs) const
  {
    assert(page==rhs.page);

    return (bbox.at(x1)<rhs.bbox.at(x0)+eps);
  }
  
  bool prov_element::is_horizontally_connected(const prov_element& elem_i,
					       const prov_element& elem_j) const
  {
    assert(page==elem_i.page);
    assert(page==elem_j.page);

    auto min_ij = std::min(elem_i.bbox[y0], elem_j.bbox[y0]);
    auto max_ij = std::max(elem_i.bbox[y1], elem_j.bbox[y1]);

    if(bbox.at(y0)<max_ij and bbox.at(y1)>min_ij) // overlap_y
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
    
    if(bbox[x0] < elem_i.bbox[x1] and bbox[x1] > elem_j.bbox[x0])
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
	    return (lhs.bbox.at(prov_element::y0)>rhs.bbox.at(prov_element::y0));
	  }
	else
	  {
	    return (lhs.bbox.at(prov_element::x0)<rhs.bbox.at(prov_element::x0));
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

  std::vector<std::string> prov_element::get_headers()
  {
    static std::vector<std::string> row
      = { "maintext-ind", "pdforder-ind",
	  "path",
	  "name", "type",
	  "page",
	  "x0", "y0", "x1", "y1"};
    return row;
  }

  bool prov_element::from_json(const nlohmann::json& item)
  {
    type = item.at("type").get<std::string>();
    name = item.at("name").get<std::string>();

    if(item.count("__ref"))
      {
	path = item.at("__ref").get<std::string>();	
      }
    else if(item.count("$ref"))
      {
	path = item.at("$ref").get<std::string>();	
      }
    else
      {
	path = "#";
      }
    
    page = item.at("page").get<ind_type>();
    bbox = item.at("bbox").get<std::array<float, 4> >();
    
    char_range = item.at("span").get<range_type>();

    return true;
  }
  
  nlohmann::json prov_element::to_json()
  {
    nlohmann::json result = nlohmann::json::object();

    result["type"] = type;
    result["name"] = name;

    if(path!="")
      {
	result["__ref"] = path;
      }

    result["page"] = page;
    result["bbox"] = bbox;
    result["span"] = char_range;

    result["text-order"] = maintext_ind;
    result["orig-order"] = pdforder_ind;
    
    return result;
  }
  
  std::vector<std::string> prov_element::to_row()
  {
    std::vector<std::string> row
      = { std::to_string(maintext_ind),
	  std::to_string(pdforder_ind),
	  path,
	  name, type,	  
	  std::to_string(page),
	  std::to_string(bbox[0]), std::to_string(bbox[1]),
	  std::to_string(bbox[2]), std::to_string(bbox[3]) };
    
    return row;
  }

  nlohmann::json prov_element::to_json_row()
  {
    nlohmann::json row = nlohmann::json::array();

    {
      row.push_back(maintext_ind);
      row.push_back(pdforder_ind);
      row.push_back(path);
      row.push_back(name);
      row.push_back(type);
      row.push_back(x0);
      row.push_back(y0);
      row.push_back(x1);
      row.push_back(y1);
    }
    assert(row.size()==prov_element::get_headers().size());
    
    return row;
  }
  

  
  /*
  class provenance
  {
  public:
    
    provenance();

    void set(const nlohmann::json& data);
    
  public:

    uint64_t dhash;
    std::vector<prov_element> elements;
  };

  provenance::provenance():
    elements({})
  {}

  void provenance::set(const nlohmann::json& provs)
  {
    elements={};

    for(std::size_t l=0; l<provs.size(); l++)
      {
	prov_element elem;
	elem.set(provs.at(l));

	elements.push_back(elem);
      }
  }
  */
  
}

#endif
