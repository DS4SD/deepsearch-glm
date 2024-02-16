//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_DS_STRUCTS_TABLE_H
#define PYBIND_ANDROMEDA_DS_STRUCTS_TABLE_H

namespace andromeda_py
{

  /*
   *
   *
   */
  class ds_table
  {
    typedef andromeda::subject<andromeda::TABLE> subject_type;

  public:

    ds_table();
    ~ds_table();

    nlohmann::json to_json(const std::set<std::string>& filters={});
    bool from_json(nlohmann::json& data);

    std::shared_ptr<andromeda::subject<andromeda::TABLE> > get_ptr() { return subj_ptr; }
    
    void clear();

    bool set_data(std::vector<std::vector<std::string> > grid);
    
  private:

    std::shared_ptr<andromeda::subject<andromeda::TABLE> > subj_ptr;
  };

  ds_table::ds_table():
    subj_ptr(std::make_shared<subject_type>())
  {}

  ds_table::~ds_table()
  {}

  nlohmann::json ds_table::to_json(const std::set<std::string>& filters)
  {
    if(subj_ptr==NULL)
      {
        return nlohmann::json::value_t::null;
      }
    
    return subj_ptr->to_json(filters);
  }

  bool ds_table::from_json(nlohmann::json& data)
  {
    if(subj_ptr==NULL)
      {
        subj_ptr = std::make_shared<subject_type>();
      }

    if(not subj_ptr->from_json(data))
      {
        subj_ptr = NULL;
        return false;
      }

    return true;
  }

  bool ds_table::set_data(std::vector<std::vector<std::string> > grid)
  {
    auto nrows = grid.size();

    if(nrows==0)
      {
	return false;
      }
    
    auto ncols = grid.at(0).size();

    for(auto& row:grid)
      {
	if(row.size()!=ncols)
	  {
	    return false;
	  }
      }
    
    nlohmann::json data = nlohmann::json::array({});
    for(auto& grid_row:grid)
      {
	nlohmann::json row = nlohmann::json::array({});

	for(std::string text:grid_row)
	  {
	    nlohmann::json cell = nlohmann::json::object({});
	    cell["text"] = text;

	    row.push_back(cell);
	  }

	data.push_back(row);
      }

    nlohmann::json obj = nlohmann::json::object({});
    obj["data"] = data;
    
    return subj_ptr->set_data(obj);
  }
  
  void ds_table::clear()
  {
    if(subj_ptr!=NULL)
      {
	subj_ptr->clear();
      }
  }

}

#endif
