//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ELEMENTS_PAGE_ELEMENT_H_
#define ANDROMEDA_STRUCTS_ELEMENTS_PAGE_ELEMENT_H_

namespace andromeda
{

  class page_element: base_types
  {

  public:

    page_element();
    page_element(int page, val_type w, val_type h);

    val_type get_width() { return dims.at(0); }
    val_type get_height() { return dims.at(1); }

    nlohmann::json to_json();
    bool from_json(const nlohmann::json& item);

  private:

    ind_type page;

    std::array<val_type, 2> dims; // (width, height) of page
  };

  page_element::page_element()
  {}

  page_element::page_element(int page, val_type w, val_type h):
    page(page),
    dims({w,h})
  {
  }

  nlohmann::json page_element::to_json()
  {
    nlohmann::json result = nlohmann::json::object();

    result["page"] = page;

    result["width"] = dims.at(0);
    result["height"] = dims.at(1);

    return result;
  }

  bool page_element::from_json(const nlohmann::json& item)
  {
    page = item.at("page").get<ind_type>();

    dims.at(0) = item.at("width").get<val_type>();
    dims.at(1) = item.at("height").get<val_type>();

    return true;
  }

}

#endif
