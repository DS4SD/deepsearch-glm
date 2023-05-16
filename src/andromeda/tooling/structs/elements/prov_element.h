//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_
#define ANDROMEDA_STRUCTS_ELEMENTS_PROV_ELEMENT_H_

namespace andromeda
{

  class prov_element
  {
  public:

    prov_element();

    void set(const nlohmann::json& data);
    
  public:

    uint16_t             page;
    std::array<float, 2> dims; // (width, height)
    std::array<float, 4> bbox; // (x0, y0, x1, y1) with x0<x1 and y0<y1
    
    std::array<uint64_t, 2> char_range;
    std::array<uint64_t, 2> coor_range;    
  };
  
  prov_element::prov_element():
    page(0),

    dims({0.0, 0.0}),
    bbox({0.0, 0.0, 0.0, 0.0}),

    char_range({0,0}),
    coor_range({0,0})
  {}
  
  void prov_element::set(const nlohmann::json& data)
  {
    page = data.value("page", page);
    bbox = data.value("bbox", bbox);
  }
  
  class provenance
  {
  public:
    
    provenance();

    void set(const nlohmann::json& data);
    
  public:
    
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
  
}

#endif
