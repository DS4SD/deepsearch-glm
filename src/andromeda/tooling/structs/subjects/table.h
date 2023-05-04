//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_TABLE_H_
#define ANDROMEDA_SUBJECTS_TABLE_H_

namespace andromeda
{
  
  template<>
  class subject<TABLE>
  {
  public:

    typedef table_element table_element_type;
    
  public:

    subject();
    ~subject();

    void clear();

    bool set(nlohmann::json& data,
	     std::shared_ptr<utils::char_normaliser> char_normaliser,
	     std::shared_ptr<utils::text_normaliser> text_normaliser);	     

    void show();

  public:

    uint64_t dhash;
    uint64_t index;
    
    std::size_t nrows, ncols;
    std::vector<std::vector<table_element_type> > data;
    
    std::set<std::string> applied_models;
    
    std::vector<base_property> properties;
    std::vector<base_entity> entities;
    std::vector<base_relation> relations;
  };

  subject<TABLE>::subject():
    dhash(-1),
    index(-1),
    
    nrows(0), ncols(0),
    data(),

    applied_models(),
    
    properties({}),
    entities({}),
    relations({})
  {}

  subject<TABLE>::~subject()
  {}

  bool subject<TABLE>::set(nlohmann::json& grid,
			   std::shared_ptr<utils::char_normaliser> char_normaliser,
			   std::shared_ptr<utils::text_normaliser> text_normaliser)
  {       
    std::set<int> ncols={};
    for(std::size_t i=0; i<grid.size(); i++)
      {
	data.push_back({});
	for(std::size_t j=0; j<grid[i].size(); j++)
	  {
	    std::string text = grid[i][j]["text"];
	    data.back().emplace_back(i,j, text);
	  }

	ncols.insert(data.back().size());
      }
    
    if(data.size()>0/* and ncols.size()==1*/)
      {
	//show();

	//std::string tmp;
	//std::cin >> tmp;
	
	return true;
      }

    return false;
  }

  void subject<TABLE>::show()
  {
    std::vector<std::vector<std::string> > grid={};
    for(std::size_t i=0; i<data.size(); i++)
      {
	grid.push_back({});
	for(std::size_t j=0; j<data[i].size(); j++)
	  {
	    grid[i].push_back(data[i][j].text);
	  }
      }

    std::stringstream ss;
    utils::show_table(grid, ss, 48);

    //LOG_S(INFO) << "NLP-output: \n" << ss.str();
    LOG_S(INFO) << "\n" << ss.str();
  }
  
}

#endif
