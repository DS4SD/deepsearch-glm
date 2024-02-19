//-*-C++-*-

#ifndef ANDROMEDA_GLM_MODEL_EDGES_BASE_EDGE_H
#define ANDROMEDA_GLM_MODEL_EDGES_BASE_EDGE_H

namespace andromeda
{
  namespace glm
  {
    class base_edge: public base_types
    {
    public:

      const static inline std::string hash_lbl = "hash";
      const static inline std::string flvr_lbl = "flvr";
      const static inline std::string name_lbl = "name";

      const static inline std::string hash_i_lbl = "hash_i";
      const static inline std::string hash_j_lbl = "hash_j";

      const static inline std::string count_lbl = "total-count";
      const static inline std::string prob_lbl = "probability";
      
      const static inline std::vector<std::string> headers = {hash_lbl, flvr_lbl, name_lbl,
							      hash_i_lbl, hash_j_lbl,
							      count_lbl, prob_lbl};
      
    public:

      base_edge();

      base_edge(flvr_type flavor,
		hash_type hash_i,
		hash_type hash_j);
      
      base_edge(flvr_type flavor,
		hash_type hash_i,
		hash_type hash_j,
		cnt_type count);

      hash_type get_hash() const { return hash; };
      flvr_type get_flvr() const { return flvr; };
      
      hash_type get_hash_i() const { return hash_i; };
      hash_type get_hash_j() const { return hash_j; };
      
      std::string get_name() const { return edge_names::to_name(flvr); };

      cnt_type get_count() const { return count; }
      void     set_count(cnt_type cnt) { count = cnt; }
      void     incr_count() { count++; }

      val_type get_prob() const { return prob; }
      void     set_prob(val_type val) { prob = val; }
      
      void from_json(const nlohmann::json& data);
      
      nlohmann::json to_json();
      nlohmann::json to_row();

      void update(const base_edge& other);

      void set_lowerbound(flvr_type flavor, hash_type hash_i);

      friend bool operator<(const base_edge& lhs, const base_edge& rhs);
      
      friend std::ofstream& operator<<(std::ofstream& os, const base_edge& edge);
      friend std::ifstream& operator>>(std::ifstream& os, base_edge& edge);
      
    private:

      hash_type to_hash();
      
    private:

      hash_type hash;
      flvr_type flvr;

      hash_type hash_i;
      hash_type hash_j;

      cnt_type count;
      val_type prob;
    };

    base_edge::base_edge():
      hash(edge_names::UNKNOWN_HASH),
      
      flvr(edge_names::UNKNOWN_FLVR),
      hash_i(edge_names::UNKNOWN_HASH),
      hash_j(edge_names::UNKNOWN_HASH),

      count(0),
      prob(0.0)
    {}

    base_edge::base_edge(flvr_type flvr,
			 hash_type hash_i,
			 hash_type hash_j):
      hash(edge_names::UNKNOWN_HASH),

      flvr(flvr),      
      hash_i(hash_i),
      hash_j(hash_j),

      count(0),
      prob(0.0)
    {
      hash = to_hash();
    }
    
    base_edge::base_edge(flvr_type flvr,
			 hash_type hash_i,
			 hash_type hash_j,
			 cnt_type count):
      hash(edge_names::UNKNOWN_HASH),
      flvr(flvr),
      
      hash_i(hash_i),
      hash_j(hash_j),

      count(count),
      prob(0.0)
    {
      hash = to_hash();
    }

    typename base_edge::hash_type base_edge::to_hash()
    {
      if(hash==edge_names::UNKNOWN_HASH)
        {
	  hash = flvr;
	  hash = utils::murmerhash3(hash);

	  hash = utils::combine_hash(hash, this->hash_i);
	  hash = utils::combine_hash(hash, this->hash_j);
        }

      return hash;
    }

    void base_edge::from_json(const nlohmann::json& data)
    {
      hash = data[hash_lbl].get<hash_type>();
      flvr = data[flvr_lbl].get<flvr_type>();

      hash_i = data[hash_i_lbl].get<hash_type>();
      hash_j = data[hash_j_lbl].get<hash_type>();

      count = data[count_lbl].get<cnt_type>();
      prob = data[prob_lbl].get<val_type>();      

      assert(hash==to_hash());      
    }
    
    nlohmann::json base_edge::to_json()
    {
      nlohmann::json data;
      {
        data[hash_lbl] = hash;
        data[flvr_lbl] = flvr;

        data[hash_i_lbl] = hash_i;
        data[hash_j_lbl] = hash_j;

        data[count_lbl] = count;
        data[prob_lbl] = prob;
      }

      return data;
    }

    nlohmann::json base_edge::to_row()
    {
      nlohmann::json row = nlohmann::json::array({});
      {
        row.push_back(hash);
	row.push_back(flvr);
        row.push_back(edge_names::to_name(flvr));

        row.push_back(hash_i);
        row.push_back(hash_j);

        row.push_back(count);
        row.push_back(prob);
      }

      return row;
    }

    void base_edge::update(const base_edge& other)
    {
      //if(flvr==other.flvr and hash==other.hash and
      //hash_i==other.hash_i and hash_j==other.hash_j)
      if(hash==other.get_hash())
        {
          count += other.get_count();
        }
      else
        {
	  std::stringstream ss;
	  ss << "updating wrong edge (with same hash) ... \n"
	     << "this : " << flvr       << ", " <<       hash_i << " -> " <<       hash_j << ":= " << hash       << "\n"
	     << "other: " << other.flvr << ", " << other.hash_i << " -> " << other.hash_j << ":= " << other.hash << "\n";
	    
	  LOG_S(ERROR) << ss.str();
        }
    }

    void base_edge::set_lowerbound(flvr_type flavor, hash_type hash_i)
    {
      this->flvr = flavor;
      this->hash_i = hash_i;
      this->hash_j = edge_names::UNKNOWN_HASH;
      this->count = std::numeric_limits<cnt_type>::max();
    }
    
    bool operator<(const base_edge& lhs, const base_edge& rhs)
    {
      if(lhs.flvr==rhs.flvr)
        {
          if(lhs.hash_i==rhs.hash_i)
	    {
	      return (lhs.count>rhs.count);
	    }

          return (lhs.hash_i<rhs.hash_i);
        }

      return (lhs.flvr<rhs.flvr);
    }

    std::ofstream& operator<<(std::ofstream& os, const base_edge& edge)
    {
      os.write((char*)&edge.hash, sizeof(edge.hash));      
      os.write((char*)&edge.flvr, sizeof(edge.flvr));

      os.write((char*)&edge.hash_i, sizeof(edge.hash_i));
      os.write((char*)&edge.hash_j, sizeof(edge.hash_j));
      
      os.write((char*)&edge.count , sizeof(edge.count));
      os.write((char*)&edge.prob  , sizeof(edge.prob));

      return os;
    }

    std::ifstream& operator>>(std::ifstream& is, base_edge& edge)
    {
      is.read((char*)&edge.hash, sizeof(edge.hash));      
      is.read((char*)&edge.flvr, sizeof(edge.flvr));

      is.read((char*)&edge.hash_i, sizeof(edge.hash_i));
      is.read((char*)&edge.hash_j, sizeof(edge.hash_j));

      is.read((char*)&edge.count , sizeof(edge.count));
      is.read((char*)&edge.prob  , sizeof(edge.prob));

      return is;
    }
    
  }

}

#endif
