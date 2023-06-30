//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_REL_TABULATE_H_
#define ANDROMEDA_STRUCTS_ITEMS_REL_TABULATE_H_

namespace andromeda
{

  void to_json(std::vector<base_relation>& relations, nlohmann::json& rels)
  {
    rels = nlohmann::json::object({});

    rels["headers"] = base_relation::headers();

    nlohmann::json& data = rels["data"];      
    data = nlohmann::json::array({});
    
    for(std::size_t l=0; l<relations.size(); l++)
      {
	data.push_back(relations.at(l).to_json_row());
      }
  }

  bool from_json(std::vector<base_relation>& relations, const nlohmann::json& rels)
  {
    auto& data = rels["data"];      

    bool success=true;
    
    base_relation rel;
    for(auto& row:data)
      {
	if(rel.from_json_row(row))
	  {
	    relations.push_back(rel);
	  }
	else
	  {
	    success=false;
	  }
      }

    return success;
  }
  
  std::string tabulate(std::vector<base_instance>& instances,
		       std::vector<base_relation>& relations)
  {
    std::vector<std::string> header = base_relation::headers();
    std::vector<std::vector<std::string> > data={};
    
    std::size_t col_width=64;
    for(auto& rel:relations)
      {
	data.push_back(rel.to_row(col_width));
      }

    std::stringstream ss;
    if(relations.size()==0)
      {
	ss << "\nrelations: " << relations.size() << "\n";
      }
    else
      {
	ss << "\nrelations: " << relations.size() << "\n"
	   << utils::to_string(header, data);    
      }

    return ss.str();
  }
  
}

#endif
