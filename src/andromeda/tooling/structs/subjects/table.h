//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_TABLE_H_
#define ANDROMEDA_SUBJECTS_TABLE_H_

namespace andromeda
{
  
  template<>
  class subject<TABLE>: provenance
  {
  public:

    typedef table_element table_element_type;
    
  public:

    subject();
    subject(uint64_t dhash,
	    uint64_t index);
    
    ~subject();

    void clear();

    bool set_data(nlohmann::json& data);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
		    std::shared_ptr<utils::text_normaliser> text_normaliser);
    
    bool set(nlohmann::json& data,
	     std::shared_ptr<utils::char_normaliser> char_normaliser,
	     std::shared_ptr<utils::text_normaliser> text_normaliser);	     

    void sort();

    typename std::vector<base_entity>::iterator ents_beg(std::array<uint64_t, 2> coor);
    typename std::vector<base_entity>::iterator ents_end(std::array<uint64_t, 2> coor);
    
    void show(bool prps, bool ents, bool rels);

    std::string get_text() const;
    
    uint64_t num_rows() const { return nrows; }
    uint64_t num_cols() const { return ncols; }

    table_element_type& operator()(std::array<uint64_t,2> coor) { return data.at(coor.at(0)).at(coor.at(1)); }
    table_element_type& operator()(uint64_t i, uint64_t j) { return data.at(i).at(j); }
    
  public:

    uint64_t dhash;
    uint64_t index;

    std::string caption;
    
    uint64_t nrows, ncols;
    std::vector<std::vector<table_element_type> > data;
    
    std::set<std::string> applied_models;
    
    std::vector<base_property> properties;
    std::vector<base_entity> entities;
    std::vector<base_relation> relations;
  };

  subject<TABLE>::subject():
    dhash(-1),
    index(-1),

    caption(""),
    
    nrows(0), ncols(0),
    data(),

    applied_models(),
    
    properties({}),
    entities({}),
    relations({})
  {}

  subject<TABLE>::subject(uint64_t dhash,
			  uint64_t index):
    dhash(dhash),
    index(index),

    caption(""),
    
    nrows(0), ncols(0),
    data(),

    applied_models(),
    
    properties({}),
    entities({}),
    relations({})
  {}
  
  subject<TABLE>::~subject()
  {}

  bool subject<TABLE>::set_data(nlohmann::json& item)
  {
    if(item.count("data"))
      {
	nlohmann::json grid = item["data"];
	
	std::set<int> ncols={};
	for(uint64_t i=0; i<grid.size(); i++)
	  {
	    data.push_back({});
	    for(uint64_t j=0; j<grid[i].size(); j++)
	      {
		std::string text = "";
		if(grid[i][j].count("text"))
		  {		    
		    text = grid[i][j]["text"];
		  }
		data.back().emplace_back(i,j,text);
	      }	   	    
	  }
      }

    if(item.count("text"))
      {
	caption = item["text"].get<std::string>();
      }
    
    if(item.count("prov"))
      {
	provenance::set(item["prov"]);
      }
    
    if(data.size()>0)
      {
	nrows = data.size();
	ncols = data.at(0).size();
	
	return true;
      }
	
    return false;	
  }

  bool subject<TABLE>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
				  std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    bool valid = true;
    
    for(auto& row:data)
      {
	for(auto& cell:row)
	  {
	    valid = (valid and cell.set_tokens(char_normaliser, text_normaliser));
	  }
      }

    return valid;
  }  
  
  bool subject<TABLE>::set(nlohmann::json& grid,
			   std::shared_ptr<utils::char_normaliser> char_normaliser,
			   std::shared_ptr<utils::text_normaliser> text_normaliser)
  {       
    bool task_0 = set_data(grid);
    bool task_1 = set_tokens(char_normaliser, text_normaliser);

    return (task_0 and task_1);
  }

  void subject<TABLE>::sort()
  {
    std::sort(entities.begin(), entities.end());
  }

  typename std::vector<base_entity>::iterator subject<TABLE>::ents_beg(std::array<uint64_t, 2> coor)
  {
    base_entity fake(NULL_MODEL, "fake", "fake", "fake", coor, {1,1}, {0,0}, {0,0}, {0,0});
    return std::lower_bound(entities.begin(), entities.end(), fake);    
  }
  
  typename std::vector<base_entity>::iterator subject<TABLE>::ents_end(std::array<uint64_t, 2> coor)
  {
    if(coor.at(0)+1==num_rows() and
       coor.at(1)+1==num_cols())
      {
	return entities.end();
      }
    else if(coor.at(1)+1==num_cols())
      {
	coor.at(0) += 1;
	coor.at(1) = 0;	
      }
    else
      {
	coor.at(1) += 1;		
      }
    
    base_entity fake(NULL_MODEL, "fake", "fake", "fake", coor, {1,1}, {0,0}, {0,0}, {0,0});
    return std::lower_bound(entities.begin(), entities.end(), fake);
  }
  
  void subject<TABLE>::show(bool prps, bool ents, bool rels)
  {
    std::vector<std::vector<std::string> > grid={};
    for(uint64_t i=0; i<data.size(); i++)
      {
	grid.push_back({});
	for(uint64_t j=0; j<data.at(i).size(); j++)
	  {
	    grid.at(i).push_back(data.at(i).at(j).text);
	  }
      }

    std::stringstream ss;

    if(provenance::elements.size()>0)
      {
	ss << "prov: "
	   << provenance::elements.at(0).page << ", "
	   << " ["
	   << provenance::elements.at(0).bbox[0] << ", "
	   << provenance::elements.at(0).bbox[1] << ", "
	   << provenance::elements.at(0).bbox[2] << ", "
	   << provenance::elements.at(0).bbox[3]
	   << "]";
      }
    
    {
      ss << "\ntable: ";
      utils::show_table(grid, ss, 48);
    }
    
    //if(mdls)
    {
      ss << "\nmodels: ";
      for(auto model:applied_models)
	{
	  ss << model << ", ";
	}
      ss << "[done]\n";
    }
    
    if(prps)
      {
        ss << tabulate(properties);
      }

    if(ents)
      {
        ss << tabulate(entities);
      }

    if(rels)
      {
        ss << tabulate(entities, relations);
      }

    LOG_S(INFO) << "NLP-output: \n" << ss.str();
  }

  std::string subject<TABLE>::get_text() const
  {
    std::stringstream ss;
    
    for(uint64_t i=0; i<data.size(); i++)
      {
	for(uint64_t j=0; j<data.at(i).size(); j++)
	  {
	    ss << data.at(i).at(j).text << " ";
	  }
      }

    std::string text = ss.str();
    return text;      
  }
  
}

#endif
