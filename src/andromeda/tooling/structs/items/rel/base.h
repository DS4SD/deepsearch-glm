//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_REL_BASE_H_
#define ANDROMEDA_STRUCTS_ITEMS_REL_BASE_H_

namespace andromeda
{
  class base_relation: public base_types
  {
    const static inline std::vector<std::string> SHRT_HEADERS = { "flvr", "name", "conf",
								  "hash_i", "hash_j",
								  "ihash_i", "ihash_j",
								  "name_i", "name_j"};

    /*
    typedef typename word_token::fval_type fval_type;
    typedef typename word_token::flvr_type flvr_type;
    typedef typename word_token::hash_type hash_type;

    typedef typename word_token::index_type index_type;
    typedef typename word_token::range_type range_type;
    typedef typename word_token::coord_type coord_type;
    */
    
    inline static std::mutex mtx;
    
    inline static std::unordered_map<std::string, base_relation::flvr_type> to_flvr_map = {};
    inline static std::unordered_map<base_relation::flvr_type, std::string> to_name_map = {};
    
  public:

    static std::vector<std::string> headers() { return SHRT_HEADERS; }

    static flvr_type   to_flvr(const std::string& rel_name);
    static std::string to_name(const flvr_type& rel_flvr);

    base_relation(std::string name, val_type conf,
		  const base_entity& ent_i,
		  const base_entity& ent_j);

    nlohmann::json to_json_row();

    std::vector<std::string> to_row(std::size_t col_width);

    std::string get_name() { return to_name(flvr); }

    hash_type get_hash_i() { return hash_i; }
    hash_type get_hash_j() { return hash_j; }

    hash_type get_ihash_i() { return ihash_i; }
    hash_type get_ihash_j() { return ihash_j; }
    
  private:

    flvr_type flvr;
    val_type  conf;

    const hash_type hash_i, ihash_i;
    const hash_type hash_j, ihash_j;

    const std::string name_i, name_j;
  };

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

  base_relation::base_relation(std::string name, val_type conf,
			       const base_entity& ent_i,
			       const base_entity& ent_j):
    flvr(to_flvr(name)),    
    conf(conf),

    hash_i(ent_i.hash),
    ihash_i(ent_i.ihash),

    hash_j(ent_j.hash),
    ihash_j(ent_j.ihash),

    name_i(ent_i.name),
    name_j(ent_j.name)
  {}
  
  nlohmann::json base_relation::to_json_row()
  {
    nlohmann::json row = nlohmann::json::array({flvr, to_name(flvr), conf,
						hash_i, hash_j,
						ihash_i, ihash_j,
						name_i, name_j});
	  
    assert(row.size()==SHRT_HEADERS.size());

    return row;
  }

  std::vector<std::string> base_relation::to_row(std::size_t col_width)
  {
    std::vector<std::string> row =
      { std::to_string(flvr), to_name(flvr), std::to_string(conf),
	std::to_string(hash_i), std::to_string(hash_j),
	std::to_string(ihash_i), std::to_string(ihash_j),
	name_i, name_j};
  
    assert(row.size()==SHRT_HEADERS.size());

    return row;
  }

}

#endif
