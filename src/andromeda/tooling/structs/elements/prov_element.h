//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_
#define ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_

namespace andromeda
{

  class prov_element
  {
  public:

    prov_element();

    prov_element(int16_t maintext_ind, std::string name, std::string type);

    prov_element(int16_t maintext_ind,
		 std::string dref, std::string name, std::string type);

    std::string to_path() const;
    static std::pair<std::string, int16_t> from_path(std::string dref);

    bool overlaps_x(const prov_element& rhs) const;
    bool overlaps_y(const prov_element& rhs) const;
    
    friend bool operator<(const prov_element& lhs, const prov_element& rhs);
    
    void set(const nlohmann::json& data);

    static std::vector<std::string> headers();
    
    std::vector<std::string> to_row();
    nlohmann::json to_json_row();
    
  public:
    
    int32_t maintext_ind;
    std::string name, type;

    std::pair<std::string, int32_t> path;

    bool ignore;
    uint16_t page;
    
    std::array<float, 2> dims; // (width, height) of page
    std::array<float, 4> bbox; // (x0, y0, x1, y1) with x0<x1 and y0<y1
    
    std::array<uint64_t, 2> char_range;
    std::array<uint64_t, 2> coor_range;    
  };
  
  prov_element::prov_element():
    maintext_ind(-1),

    name("undef"),
    type("undef"),
    
    path("undef", -1),
    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  prov_element::prov_element(int16_t maintext_ind, std::string name, std::string type):
    maintext_ind(maintext_ind),

    name(name),
    type(type),
    
    path("main-text", maintext_ind),
    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  prov_element::prov_element(int16_t maintext_ind, std::string doc_path,
			     std::string name, std::string type):
    maintext_ind(maintext_ind),

    name(name),
    type(type),
    
    path(from_path(doc_path)),
    ignore(false),
    
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}

  std::string prov_element::to_path() const
  {
    std::stringstream ss;
    ss << "#" << "/" << path.first << "/" << path.second;

    return ss.str();
  }
  
  std::pair<std::string, int16_t> prov_element::from_path(std::string doc_path)
  {
    std::vector<std::string> parts = utils::split(doc_path, "/");
    assert(parts.size()==3 and parts.at(0)=="#");
    
    return std::pair<std::string, int16_t>{parts.at(1), std::stoi(parts.at(2))};
  }

  bool prov_element::overlaps_x(const prov_element& rhs) const 
  {
    return ((bbox[0]<=rhs.bbox[0] and rhs.bbox[0]<bbox[2]) or
	    (bbox[0]<=rhs.bbox[2] and rhs.bbox[2]<bbox[2]) or
	    (rhs.bbox[0]<=bbox[0] and bbox[0]<rhs.bbox[2]) or
	    (rhs.bbox[0]<=bbox[2] and bbox[2]<rhs.bbox[2]) ); 
  }

  bool prov_element::overlaps_y(const prov_element& rhs) const
  {
    return ((bbox[1]<=rhs.bbox[1] and rhs.bbox[1]<bbox[3]) or
	    (bbox[1]<=rhs.bbox[3] and rhs.bbox[3]<bbox[3]) or
	    (rhs.bbox[1]<=bbox[1] and bbox[1]<rhs.bbox[3]) or
	    (rhs.bbox[1]<=bbox[3] and bbox[3]<rhs.bbox[3]) ); 
  }
  
  bool operator<(const prov_element& lhs, const prov_element& rhs)
  {
    //LOG_S(INFO) << std::setw(6) << lhs.maintext_ind
    //<< std::setw(6) << rhs.maintext_ind;
    
    //const double delta=1.0;

    if(lhs.page==rhs.page)
      {
	if(lhs.overlaps_x(rhs))
	  {
	    auto lhs_ycm = (lhs.bbox.at(1)+lhs.bbox.at(3))/2.0;
	    auto rhs_ycm = (rhs.bbox.at(1)+rhs.bbox.at(3))/2.0;
	    
	    return (lhs_ycm>rhs_ycm);	    
	  }
	else
	  {
	    return (lhs.bbox.at(0)<rhs.bbox.at(0));
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
  }

  std::vector<std::string> prov_element::headers()
  {
    std::vector<std::string> row
      = { "mtext", "path-key", "path-ind", "name", "type", "page", "x0", "y0", "x1", "y1"};
    return row;
  }
  
  std::vector<std::string> prov_element::to_row()
  {
    std::vector<std::string> row
      = { std::to_string(maintext_ind), path.first, std::to_string(path.second),
	  name, type, std::to_string(page),
	  std::to_string(bbox[0]), std::to_string(bbox[1]),
	  std::to_string(bbox[2]), std::to_string(bbox[3]) };
    
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
