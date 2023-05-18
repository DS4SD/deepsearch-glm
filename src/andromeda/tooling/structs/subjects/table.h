//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_TABLE_H_
#define ANDROMEDA_SUBJECTS_TABLE_H_

namespace andromeda
{
  
  template<>
  class subject<TABLE>: provenance
  {
    typedef float    fval_type;
    typedef uint16_t flvr_type;    
    typedef uint64_t hash_type;
    
    typedef            uint64_t     index_type;
    typedef std::array<uint64_t, 2> range_type;
    typedef std::array<uint64_t, 2> coord_type;
    
  public:

    typedef table_element table_element_type;
    
  public:

    subject();
    subject(uint64_t dhash,
	    uint64_t index);
    
    ~subject();

    void clear();

    nlohmann::json to_json();
    bool from_json(const nlohmann::json& data);
    
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

    uint64_t get_hash() const { return hash; } 
    std::string get_text() const;
    
    uint64_t num_rows() const { return nrows; }
    uint64_t num_cols() const { return ncols; }

    table_element_type& operator()(std::array<uint64_t,2> coor) { return data.at(coor.at(0)).at(coor.at(1)); }
    table_element_type& operator()(uint64_t i, uint64_t j) { return data.at(i).at(j); }

  private:

    void set_hash();
    
  public:

    bool valid;
    uint64_t hash;
    
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
    valid(false),
    
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
    valid(false),
    
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

  void subject<TABLE>::clear()
  {
    valid = false;
    
    dhash=-1;
    index=-1;

    caption="";
    
    nrows=0;
    ncols=0;

    data.clear();

    applied_models.clear();
    
    properties.clear();
    entities.clear();
    relations.clear();
  }

  nlohmann::json subject<TABLE>::to_json()
  {
    nlohmann::json result = nlohmann::json::object({});

    {
      result["hash"] = hash;
    }

    result["applied-models"] = applied_models;
    
    {
      nlohmann::json& props = result["properties"];
      andromeda::to_json(properties, props);
    }

    {
      nlohmann::json& ents = result["entities"];
      ents = nlohmann::json::object({});

      ents["headers"] = base_entity::headers(TABLE);

      nlohmann::json& ents_data = ents["data"];      
      ents_data = nlohmann::json::array({});

      for(std::size_t l=0; l<entities.size(); l++)
	{
	  ents_data.push_back(entities.at(l).to_json_row(TABLE));
	}
    }

    {
      nlohmann::json& rels = result["relations"];
      rels = nlohmann::json::object({});

      rels["headers"] = base_relation::headers();

      nlohmann::json& rels_data = rels["data"];      
      rels_data = nlohmann::json::array({});
      
      for(std::size_t l=0; l<relations.size(); l++)
	{
	  rels_data.push_back(relations.at(l).to_json_row());
	}      
    }

    return result;
  }
  
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

	set_hash();
	
	return true;
      }
	
    return false;	
  }
  
  void subject<TABLE>::set_hash()
  {
    std::vector<uint64_t> hashes={dhash};
    for(std::size_t i=0; i<data.size(); i++)
      {
	for(std::size_t j=0; j<data.at(i).size(); j++)
	  {
	    hashes.push_back(data.at(i).at(j).hash);
	  }
      }

    hash = utils::to_hash(hashes);
  }
  
  bool subject<TABLE>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
				  std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    valid = true;
    
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
    range_type min_range = {0, 0};
    
    base_entity fake(NULL_MODEL, "fake", "fake", "fake", coor, {1,1},
		     min_range, min_range, min_range);

    return std::lower_bound(entities.begin(), entities.end(), fake);    
  }
  
  typename std::vector<base_entity>::iterator subject<TABLE>::ents_end(std::array<uint64_t, 2> coor)
  {
    range_type max_range =
      { std::numeric_limits<uint64_t>::max(),
	std::numeric_limits<uint64_t>::max()};
    
    base_entity fake(NULL_MODEL, "fake", "fake", "fake", coor, {1,1},
		     max_range, max_range, max_range);

    return std::upper_bound(entities.begin(), entities.end(), fake);    
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
	    if(j+1==data.at(i).size())
	      {
		ss << data.at(i).at(j).text << "\n";
	      }
	    else
	      {
		ss << data.at(i).at(j).text << ", ";
	      }
	  }
      }
    
    std::string text = ss.str();
    return text;      
  }
  
}

#endif
