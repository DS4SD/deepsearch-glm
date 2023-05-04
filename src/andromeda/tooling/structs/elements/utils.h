//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ELEMENTS_UTILS_H_
#define ANDROMEDA_STRUCTS_ELEMENTS_UTILS_H_

namespace andromeda
{
  void show_list(std::vector<list_element>& data,
		 std::stringstream& ss)
  {
    std::vector<std::string> grid;
    
    for(auto& item:data)
      {
	grid.push_back(item.text);
      }

    utils::show_list(grid, ss, 70);
  }
  
  void show_table(std::vector<std::vector<table_element> >& data,
		  std::stringstream& ss)
  {
    std::vector<std::vector<std::string> > grid;
    
    for(auto& row:data)
      {
	grid.push_back({});
	for(auto& item:row)
	  {
	    grid.back().push_back(item.text);
	  }
      }

    utils::show_table(grid, ss, 32);
  }
  
}

#endif
