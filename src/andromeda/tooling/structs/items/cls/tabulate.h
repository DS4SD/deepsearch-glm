//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_CLS_TABULATE_H_
#define ANDROMEDA_STRUCTS_ITEMS_CLS_TABULATE_H_

namespace andromeda
{
  void to_json(std::vector<base_property>& properties, nlohmann::json& props)
  {
    props = nlohmann::json::object({});

    props["headers"] = base_property::HEADERS;

    nlohmann::json& data = props["data"];      
    data = nlohmann::json::array({});
    
    for(std::size_t l=0; l<properties.size(); l++)
      {
	data.push_back(properties.at(l).to_json_row());
      }
  }

  bool from_json(std::vector<base_property>& properties, const nlohmann::json& props)
  {
    //nlohmann::json& hdrs = props["headers"];      
    auto& data = props["data"];      

    bool success=true;
    
    base_property prop;
    for(auto& row:data)
      {
	if(prop.from_json_row(row))
	  {
	    properties.push_back(prop);
	  }
	else
	  {
	    success=false;
	  }
      }

    return success;
  }
  
  std::string tabulate(std::vector<base_property>& properties)
  {
    std::vector<std::string> header = base_property::HEADERS;
    std::vector<std::vector<std::string> > data={};

    for(auto& prop:properties)
      {
	data.push_back(prop.to_row());
      }

    std::stringstream ss;
    if(properties.size()==0)
      {
	ss << "\nproperties: " << properties.size() << "\n";
      }
    else
      {
	ss << "\nproperties: " << properties.size() << "\n"
	   << utils::to_string(header, data);    
      }

    return ss.str();
  }
  
}

#endif
