//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_REL_TABULATE_H_
#define ANDROMEDA_STRUCTS_ITEMS_REL_TABULATE_H_

namespace andromeda
{
  
  std::string tabulate(std::vector<base_entity>& entities,
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
