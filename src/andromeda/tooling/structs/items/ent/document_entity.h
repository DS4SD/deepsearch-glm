//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_ENT_DOCUMENT_H_
#define ANDROMEDA_STRUCTS_ITEMS_ENT_DOCUMENT_H_

namespace andromeda
{
  class document_entity: public base_entity
  {
  public:

    document_entity(const base_entity& ent,
		    subject_name element_type,
		    index_type element_index);
    
    static std::vector<std::string> headers();

    std::vector<std::string> to_row(std::size_t col_width);
    
    std::vector<std::string> to_row(std::string& text,
				    std::size_t col_width);

    nlohmann::json to_json();

  public:

    subject_name element_type;
    index_type element_index;
  };

  document_entity::document_entity(const base_entity& ent,
				   subject_name element_type,
				   index_type element_index):
      base_entity(ent),
      element_type(element_type),
      element_index(element_index)
  {}
  
  nlohmann::json document_entity::to_json()
  {
    nlohmann::json result = base_entity::to_json();
    {
      result["element-type"] = to_string(element_type);
      result["element-index"] = element_type;
    }
    
    return result;
  }
  
  std::vector<std::string> document_entity::headers()
  {
    std::vector<std::string> row =
      { "subject",
	"type", "subtype", "conf", "index",
	"i", "j", "ctok_i", "ctok_j", "wtok_i", "wtok_j",
	"name", "original" };
    
    return row;
  }
  
  std::vector<std::string> document_entity::to_row(std::size_t col_width)
  {
    std::stringstream ss;
    ss << to_string(element_type) << "." << std::to_string(element_index);
    
    std::vector<std::string> row =
      { //to_string(element_type), std::to_string(element_index),
       ss.str(), to_key(model_type), model_subtype,
       std::to_string(conf), std::to_string(hash),
       std::to_string(char_range[0]), std::to_string(char_range[1]),
       std::to_string(ctok_range[0]), std::to_string(ctok_range[1]),
       std::to_string(wtok_range[0]), std::to_string(wtok_range[1]),
       utils::to_fixed_size(name, col_width),
       utils::to_fixed_size(orig, col_width)
      };

    return row;
  }
    
  std::vector<std::string> document_entity::to_row(std::string& text, std::size_t col_width)
  {
    std::string tmp=orig;
    if(orig.size()==0)
      {
	tmp = text.substr(char_range[0], char_range[1]-char_range[0]);
      }

    std::stringstream ss;
    ss << to_string(element_type) << "." << std::to_string(element_index);
    
    std::vector<std::string> row =
      {
       //to_string(element_type), std::to_string(element_index),
       ss.str(), to_key(model_type), model_subtype,
       std::to_string(conf), std::to_string(hash),
       std::to_string(char_range[0]), std::to_string(char_range[1]),
       std::to_string(ctok_range[0]), std::to_string(ctok_range[1]),
       std::to_string(wtok_range[0]), std::to_string(wtok_range[1]),
       utils::to_fixed_size(name, col_width),
       utils::to_fixed_size(tmp, col_width)
      };

    return row;
  }
  
}

#endif
