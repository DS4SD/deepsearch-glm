//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_REL_BASE_H_
#define ANDROMEDA_STRUCTS_ITEMS_REL_BASE_H_

namespace andromeda
{
  class base_relation: public base_types
  {
    const static inline std::vector<std::string> SHRT_HEADERS = { "flvr", "name", "conf",
								  "hash_i", "hash_j",
								  //"ihash_i", "ihash_j",
								  "name_i", "name_j"};

    inline static std::mutex mtx;
    
    inline static std::unordered_map<std::string, base_relation::flvr_type> to_flvr_map = {};
    inline static std::unordered_map<base_relation::flvr_type, std::string> to_name_map = {};
    
  public:

    static std::vector<std::string> headers() { return SHRT_HEADERS; }

    static bool update(const base_relation::flvr_type& flvr, const std::string& rel_name);
    
    static flvr_type   to_flvr(const std::string& rel_name);
    static std::string to_name(const flvr_type& rel_flvr);

    base_relation();
    
    base_relation(std::string name, val_type conf,
		  const base_instance& inst_i,
		  const base_instance& inst_j);

    friend bool operator<(const base_relation& lhs, const base_relation& rhs);
    
    nlohmann::json to_json_row();
    bool from_json_row(const nlohmann::json& row);
    
    std::vector<std::string> to_row(std::size_t col_width);

    std::string get_type() { return to_name(flvr); }
    std::string get_name() { return to_name(flvr); }

    hash_type get_hash_i() { return hash_i; }
    hash_type get_hash_j() { return hash_j; }
    
  private:

    flvr_type flvr;
    val_type  conf;

    hash_type hash_i, hash_j;
    std::string name_i, name_j;
  };

  bool base_relation::update(const base_relation::flvr_type& flvr,
			     const std::string& rel_name)
  {
    auto itr = to_flvr_map.find(rel_name);

    if(itr!=to_flvr_map.end() and itr->first==rel_name and itr->second==flvr)
      {
	return true;
      }
    else if(itr!=to_flvr_map.end() and itr->first==rel_name and itr->second!=flvr)
      {
	LOG_S(ERROR) << "inconsistent relation flvr";
	return false;
      }
    else
      {
	std::scoped_lock lock(mtx);

	to_flvr_map.insert({rel_name, flvr});
	to_name_map.insert({flvr, rel_name});

	return true;
      }
  }
  
  typename base_relation::flvr_type base_relation::to_flvr(const std::string& rel_name)
  {
    flvr_type flvr=-1;
    
    auto itr = to_flvr_map.find(rel_name);

    if(itr!=to_flvr_map.end())
      {
	flvr = itr->second;
      }
    else
      {
	std::scoped_lock lock(mtx);

	flvr = utils::to_flvr_hash(rel_name);

	to_flvr_map.insert({rel_name, flvr});
	to_name_map.insert({flvr, rel_name});
      }

    return flvr;
  }

  std::string base_relation::to_name(const base_relation::flvr_type& flvr)
  {
    std::string name="unknown";
    
    auto itr = to_name_map.find(flvr);
    if(itr!=to_name_map.end())
      {
	name = itr->second;
      }

    return name;
  }

  base_relation::base_relation()
  {}
  
  base_relation::base_relation(std::string name, val_type conf,
			       const base_instance& inst_i,
			       const base_instance& inst_j):
    flvr(to_flvr(name)),    
    conf(conf),

    //hash_i(inst_i.get_ehash()),
    hash_i(inst_i.get_ihash()),

    //hash_j(inst_j.get_ehash()),
    hash_j(inst_j.get_ihash()),

    name_i(inst_i.get_name()),
    name_j(inst_j.get_name())
  {
    /*
    LOG_S(INFO) << name << "\t" << conf << "\t("
		<< hash_i << ", "
		<< hash_j << ") => ["
		<< name_i << ", "
		<< name_j << "]"; 
    */
  }

  bool operator<(const base_relation& lhs, const base_relation& rhs)
  {
    if(lhs.flvr==rhs.flvr)
      {
	if(lhs.hash_i==rhs.hash_i)
	  {
	    return (lhs.hash_j<rhs.hash_j);
	  }
	else
	  {
	    return (lhs.hash_i<rhs.hash_i);
	  }
      }
    else
      {
	return (lhs.flvr<rhs.flvr);
      }
  }
  
  nlohmann::json base_relation::to_json_row()
  {
    nlohmann::json row = nlohmann::json::array({flvr, to_name(flvr),
	utils::round_conf(conf),
	hash_i, hash_j,
	name_i, name_j});
	  
    assert(row.size()==SHRT_HEADERS.size());

    return row;
  }

  bool base_relation::from_json_row(const nlohmann::json& row)
  {
    if((not row.is_array()) or row.size()!=9)
      {
	LOG_S(ERROR) << "inconsistent relation-row: " << row.dump();
	return false;
      }

    flvr = row.at(0).get<flvr_type>();
    std::string name = row.at(1).get<std::string>();

    update(flvr, name);
    
    conf = row.at(2).get<val_type>();

    hash_i = row.at(3).get<hash_type>();
    //ihash_i = row.at(4).get<hash_type>();

    hash_j = row.at(5).get<hash_type>();
    //ihash_j = row.at(6).get<hash_type>();

    name_i = row.at(7).get<std::string>();
    name_j = row.at(8).get<std::string>();

    return true;
  }
  
  std::vector<std::string> base_relation::to_row(std::size_t col_width)
  {
    std::vector<std::string> row =
      { std::to_string(flvr), to_name(flvr),
	std::to_string(utils::round_conf(conf)),
	std::to_string(hash_i), std::to_string(hash_j),
	//std::to_string(ihash_i), std::to_string(ihash_j),
	name_i, name_j};
  
    assert(row.size()==SHRT_HEADERS.size());

    return row;
  }

}

#endif
