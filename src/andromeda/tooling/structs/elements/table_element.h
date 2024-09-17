//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_ELEMENTS_TABLE_ELEMENT_H_
#define ANDROMEDA_SUBJECTS_ELEMENTS_TABLE_ELEMENT_H_

namespace andromeda
{
  class table_element: public text_element
  {
  public:

    table_element(nlohmann::json& json_cell);

    /*
    table_element(uint64_t i,
		  uint64_t j,
		  std::string orig);
    */
    
    table_element(uint64_t i,
		  uint64_t j,
		  std::array<uint64_t,2> row_span,			       
		  std::array<uint64_t,2> col_span,
		  std::array<float, 4> bbox,
		  //nlohmann::json bbox,
      std::string type,
		  std::string orig);
    
    nlohmann::json to_json();
    bool from_json(nlohmann::json& json_cell);
    
    std::array<uint64_t, 2> get_coor() { return {i,j}; }
    std::array<uint64_t, 2> get_row_span() { return row_span; }
    std::array<uint64_t, 2> get_col_span() { return col_span; }

    std::string get_text() const { return text; }
    
    bool is_col_header() { return col_header; }
    bool is_row_header() { return row_header; }
    bool is_numeric() { return numeric; }

    void set_numeric(bool val) { numeric = val; }

    bool skip() { return (numeric or word_tokens.size()==0); }
    
    void show();
    
  private:
    
    uint64_t i, j;
    std::string type;
    
    std::array<uint64_t,2> row_span, col_span;
    std::array<float, 4> bbox;
    //nlohmann::json bbox;
    
    bool row_header, col_header, numeric;
  };

  table_element::table_element(nlohmann::json& json_cell)
  {
    from_json(json_cell);    
  }

  table_element::table_element(uint64_t i, uint64_t j,
			       std::array<uint64_t,2> row_span,			       
			       std::array<uint64_t,2> col_span,
			       std::array<float, 4> bbox,
             std::string type,
			       std::string orig):
    text_element(),
    i(i), j(j),
    type(type),

    row_span(row_span),
    col_span(col_span),

    bbox(bbox),
    
    row_header(false),
    col_header(false),

    numeric(false)
  {
    //text_element::set(orig, NULL, NULL);
    text_element::set_text(orig);
  }    
  
  nlohmann::json table_element::to_json()
  {
    nlohmann::json cell = nlohmann::json::object({});

    cell["row"] = i;
    cell["col"] = j;

    cell["row-span"] = row_span;
    cell["col-span"] = col_span;

    std::vector<std::vector<uint64_t> >spans={};
    for(uint64_t ri=row_span[0]; ri<row_span[1]; ri++)
      {
	for(uint64_t cj=col_span[0]; cj<col_span[1]; cj++)
	  {
	    spans.push_back({ri,cj});
	  }
      }
    cell["spans"] = spans;
    
    cell["text"] = text;
    cell["type"] = type;

    if(std::abs(bbox[0])<1.e-3 and
       std::abs(bbox[1])<1.e-3 and
       std::abs(bbox[2])<1.e-3 and
       std::abs(bbox[3])<1.e-3)
      {
	cell["bbox"] = nlohmann::json::value_t::null;
      }
    else
      {
	cell["bbox"] = bbox;
      }
    
    cell["row-header"] = row_header;
    cell["col-header"] = col_header;

    return cell;
  }

  bool table_element::from_json(nlohmann::json& json_cell)
  {
    i = json_cell.at("row").get<index_type>();
    j = json_cell.at("col").get<index_type>();

    row_span = json_cell.at("row-span").get<std::array<index_type, 2> >();
    col_span = json_cell.at("col-span").get<std::array<index_type, 2> >();
    
    std::string ctext = json_cell.at("text").get<std::string>();
    text_element::set_text(ctext);
    
    type = json_cell.at("type").get<std::string>();

    if(json_cell.at("bbox").is_array())
      {
	bbox = json_cell.at("bbox").get<std::array<float, 4> >();
      }
    else
      {
	bbox = {0.0, 0.0, 0.0, 0.0};
      }
    
    row_header = json_cell.at("row-header").get<bool>();
    col_header = json_cell.at("col-header").get<bool>();

    return true;
  }
  
  void table_element::show()
  {
    LOG_S(INFO) << "table(" << i << ", " << j << "): " << text;
    LOG_S(INFO) << tabulate(this->word_tokens, text);
  }
  
}

#endif
