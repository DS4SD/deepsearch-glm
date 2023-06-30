//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_EDGES_H_
#define ANDROMEDA_MODELS_GLM_EDGES_H_

#include <andromeda/glm/model/edges/base.h>
#include <andromeda/glm/model/edges/base_edge.h>

namespace andromeda
{
  namespace glm
  {
    class glm_edges: public base_types
    {
    public:

      const static index_type NOT_FOUND=-1;

      typedef base_edge edge_type;

      typedef std::pair<flvr_type, ind_type> key_type;

      typedef          std::vector<edge_type>   edge_coll_type;
      typedef typename edge_coll_type::iterator edge_itr_type;

      typedef std::map<flvr_type, edge_coll_type> flvr_map_type;
      typedef typename flvr_map_type::iterator    flvr_itr_type;

      typedef std::unordered_map<hash_type, key_type> hash_map_type;

    public:

      glm_edges();
      ~glm_edges();

      double load_factor() { return hash_to_key.load_factor(); }
      double max_load_factor() { return hash_to_key.max_load_factor(); }

      std::size_t number_of_flavors() { return flvr_colls.size(); }

      std::size_t size() { return hash_to_key.size(); }
      std::size_t size(flvr_type flvr) { return flvr_colls.at(flvr).size(); }

      edge_type& at(key_type key) { return flvr_colls.at(key.first).at(key.second); }
      edge_coll_type& at(flvr_type flvr) { return flvr_colls.at(flvr); }

      flvr_itr_type begin() { return flvr_colls.begin(); }
      flvr_itr_type end() { return flvr_colls.end(); }

      edge_itr_type begin(flvr_type flvr) { return flvr_colls.at(flvr).begin(); }
      edge_itr_type end(flvr_type flvr) { return flvr_colls.at(flvr).end(); }

      void show_bucket_distribution();

      void clear();
      void reserve(std::size_t N);

      void initialise();

      edge_type& push_back(edge_type& edge, bool update_hashmap);

      bool has(flvr_type flavor);
      bool has(const edge_type& edge);

      bool has(flvr_type flavor, hash_type hash_i, hash_type hash_j);

      bool get(hash_type& hash, edge_type& edge);

      edge_type& insert(edge_type& edge, bool check_size);

      edge_type& insert(flvr_type flavor, hash_type hash_i, hash_type hash_j,
                        bool check_size);

      edge_type& insert(flvr_type flavor, hash_type hash_i, hash_type hash_j,
                        cnt_type count, bool check_size);

      void init_hashmap();
      
      void sort();
      void sort(flvr_type flvr);

      bool is_sorted(flvr_type flvr);
      void set_sorted(flvr_type flvr, bool sorted);

      std::pair<cnt_type, cnt_type> get_number_of_edges(flvr_type flavor,
                                                        hash_type hash_i);

      edge_itr_type traverse(flvr_type flavor, hash_type hash_i);

      cnt_type traverse(flvr_type flavor, hash_type hash_i,
                        std::vector<edge_type>& edges, bool sorted);

    private:

      std::size_t max_allowed_size;

      std::map<flvr_type, bool> flvr_sorted;

      flvr_map_type flvr_colls;
      hash_map_type hash_to_key;
    };

    glm_edges::glm_edges():
      max_allowed_size(-1)
    {
      initialise();
    }

    glm_edges::~glm_edges()
    {}

    void glm_edges::show_bucket_distribution()
    {
      std::map<std::size_t, std::size_t> cnt={};
      for(std::size_t i=0; i<hash_to_key.bucket_count(); i++)
        {
          std::size_t n=hash_to_key.bucket_size(i);
          if(n>0)
            {
              auto itr = cnt.find(n);
              if(itr==cnt.end())
                {
                  cnt.insert({n,1});
                }
              else
                {
                  itr->second += 1;
                }
            }
        }

      LOG_S(INFO) << __FUNCTION__ << " (bucket-size versus count): " << hash_to_key.max_load_factor();
      if(cnt.size()>0)
        {
          for(auto itr=cnt.begin(); itr!=cnt.end(); itr++)
            {
              LOG_S(INFO) << "\t" << itr->first << ": " << itr->second;
            }
        }
    }

    void glm_edges::clear()
    {
      hash_to_key.clear();
      flvr_colls.clear();
    }

    void glm_edges::initialise()
    {
      clear();

      reserve(1e7);
    }

    void glm_edges::reserve(std::size_t N)
    {
      hash_to_key.reserve(N);
      hash_to_key.max_load_factor(32.0);

      //for(auto item:edge_names::flvr_to_name_map)
      for(auto itr=edge_names::begin(); itr!=edge_names::end(); itr++)
        {
	  flvr_type flvr = itr->first;
	  
          if(flvr!=edge_names::UNKNOWN_FLVR)
            {
              flvr_sorted[flvr]=false;
              flvr_colls[flvr].reserve(1e3);
            }
        }
    }

    typename glm_edges::edge_type& glm_edges::push_back(edge_type& edge, bool update_hashmap)
    {
      flvr_type flvr = edge.get_flvr();
      hash_type hash = edge.get_hash();

      if(flvr_colls.count(flvr)==0)
        {
          flvr_colls[flvr].reserve(1e3);
        }

      auto& flvr_coll = flvr_colls.at(flvr);

      if(update_hashmap)
	{
	  ind_type ind = flvr_coll.size();
	  key_type key(flvr, ind);
	  
	  hash_to_key.insert(std::make_pair(hash, key));
	}
      
      flvr_coll.push_back(edge);
      flvr_sorted[flvr]=false;

      return flvr_coll.back();
    }

    bool glm_edges::has(flvr_type flavor)
    {
      return (flvr_colls.count(flavor)==1);
    }

    bool glm_edges::has(const edge_type& edge)
    {
      auto itr = hash_to_key.find(edge.get_hash());
      return (itr!=hash_to_key.end() and itr->first==edge.get_hash());
    }

    bool glm_edges::has(flvr_type flvr, hash_type hash_i, hash_type hash_j)
    {
      edge_type edge(flvr, hash_i, hash_j, 0);
      return has(edge);
    }

    bool glm_edges::get(hash_type& hash, edge_type& edge)
    {
      auto itr = hash_to_key.find(hash);

      if(itr!=hash_to_key.end() and itr->first==hash)
        {
          edge = this->at(itr->second);
          return true;
        }

      return false;
    }

    typename glm_edges::edge_type& glm_edges::insert(edge_type& other, bool check_size)
    {
      auto itr = hash_to_key.find(other.get_hash());

      if(itr!=hash_to_key.end() and itr->first==other.get_hash())
        {
          auto& edge = this->at(itr->second);
          edge.update(other);

          return edge;
        }
      else if((not check_size) or size()<max_allowed_size)
        {
          return push_back(other, true);
        }
      else
        {
          static bool warned=false;

          if(not warned)
            {
              LOG_S(WARNING) << "exceeding reserved edge-size (" << max_allowed_size << ")";
              warned=true;
            }

          return other;
        }
    }

    typename glm_edges::edge_type& glm_edges::insert(flvr_type flvr, hash_type hash_i, hash_type hash_j,
                                                     bool check_size)
    {
      return this->insert(flvr, hash_i, hash_j, 1, check_size);
    }

    typename glm_edges::edge_type& glm_edges::insert(flvr_type flvr, hash_type hash_i, hash_type hash_j,
                                                     cnt_type count, bool check_size)
    {
      edge_type edge(flvr, hash_i, hash_j, count);
      return insert(edge, check_size);
    }

    bool glm_edges::is_sorted(flvr_type flvr)
    {
      auto itr = flvr_sorted.find(flvr);

      if(itr!=flvr_sorted.end() and flvr_colls.count(flvr)>0)
        {
          return (itr->second);
        }
      else if(itr==flvr_sorted.end() and flvr_colls.count(flvr)>0)
        {
          flvr_sorted[flvr]=false;
          return flvr_sorted.at(flvr);
        }
      else
        {
          return false;
        }
    }

    void glm_edges::set_sorted(flvr_type flvr, bool sorted)
    {
      if(flvr_colls.count(flvr)>0)
        {
          flvr_sorted[flvr] = sorted;
        }
      else
        {
          //LOG_S(WARNING) << "trying to update unknown edge-flvr (" << flvr << ") ...";
        }
    }

    void glm_edges::sort(flvr_type flvr)
    {
      if(flvr_sorted.count(flvr)==0)
        {
          flvr_sorted[flvr] = false;
        }
      else if(flvr_sorted.at(flvr))
        {
          return;
        }
      else
        {}

      if(flvr_colls.count(flvr)==0)
        {
          //LOG_S(WARNING) << "no edge with name: " << edge_names::to_string(flvr);
          return;
        }

      auto& coll = flvr_colls.at(flvr);

      if(coll.size()>0)
        {
          LOG_S(INFO) << "sorting edge [" << std::setw(20) << edge_names::to_name(flvr) << "]: "
                      << std::setw(12) << coll.size();
        }

      {
        std::sort(coll.begin(), coll.end());

        for(std::size_t ind=0; ind<coll.size(); ind++)
          {
            edge_type& edge = coll.at(ind);

            flvr_type flvr = edge.get_flvr();
            hash_type hash = edge.get_hash();

            hash_to_key.at(hash) = key_type({flvr, ind});
          }
      }

      {
        auto edge_itr = coll.begin();
        while(edge_itr!=coll.end())
          {
            hash_type hash_i = edge_itr->get_hash_i();

            val_type total=0.0;

            auto tmp = edge_itr;
            while(tmp!=coll.end() and
                  tmp->get_hash_i()==hash_i)
              {
                total += tmp->get_count();
                tmp++;
              }

            while(edge_itr!=tmp)
              {
                val_type cnt = edge_itr->get_count();
                val_type tot = total;

                edge_itr->set_prob(cnt/(tot+1.e-6));
                edge_itr++;
              }
          }
      }

      flvr_sorted.at(flvr)=true;
    }

    void glm_edges::sort()
    {
      LOG_S(INFO) << __FUNCTION__;

      for(auto itr=flvr_colls.begin(); itr!=flvr_colls.end(); itr++)
        {
          sort(itr->first);
        }
    }

    void glm_edges::init_hashmap()
    {
      LOG_S(INFO) << __FUNCTION__;

      for(auto itr=flvr_colls.begin(); itr!=flvr_colls.end(); itr++)
        {
	  flvr_type flvr = itr->first; 
	  auto& flvr_coll = itr->second;

	  LOG_S(INFO) << flvr << " [" << flvr_coll.size() << "]";
	  for(std::size_t ind=0; ind<flvr_coll.size(); ind++)
	    {
	      assert(flvr==flvr_coll.at(ind).get_flvr());

	      key_type key(flvr, ind);
	      hash_type hash = flvr_coll.at(ind).get_hash();
	      
	      hash_to_key[hash] = key;
	    }
        }
    }
    
    std::pair<typename glm_edges::cnt_type,
              typename glm_edges::cnt_type> glm_edges::get_number_of_edges(flvr_type flvr,
                                                                           hash_type hash_i)
    {
      if(flvr_sorted.count(flvr)==0 or (not flvr_sorted.at(flvr)))
        {
          sort(flvr);
        }

      auto& coll = flvr_colls.at(flvr);

      edge_type edge(flvr, hash_i, 0, -1);
      auto itr = std::lower_bound(coll.begin(), coll.end(), edge);

      cnt_type count=0, weight=0;
      while(itr!=coll.end())
        {
          if(itr->get_hash_i()!=hash_i)
            {
              break;
            }

          count += 1;
          weight += itr->get_count();

          itr++;
        }

      return std::pair<cnt_type, cnt_type>(count, weight);
    }

    typename glm_edges::cnt_type glm_edges::traverse(flvr_type flvr, hash_type hash_i,
                                                     std::vector<edge_type>& tmps, bool sorted)
    {
      tmps.clear();

      if(flvr_sorted.count(flvr)==0 or (not flvr_sorted.at(flvr)))
        {
          LOG_S(ERROR) << "flvr " << flvr << " is not sorted: aborting traversal ...";
          return tmps.size();
        }

      if(flvr_colls.count(flvr)==0)
        {
          LOG_S(WARNING) << "unknown flvr: " << flvr;
          return tmps.size();
        }

      auto& coll = flvr_colls.at(flvr);

      std::size_t total=0;
      {
        edge_type edge;
	edge.set_lowerbound(flvr, hash_i);
	
        auto itr = std::lower_bound(coll.begin(), coll.end(), edge);
        while(itr!=coll.end() and
              itr->get_hash_i()==hash_i and
              itr->get_hash_j()!=edge_names::UNKNOWN_HASH and
              itr->get_count()>0)
          {
            tmps.push_back(*itr);
            total += itr->get_count();

            itr++;
          }
      }
      
      for(auto& tmp:tmps)
        {
          val_type cnt = tmp.get_count();
          val_type tot = total;

          tmp.set_prob(cnt/(tot+1.e-6));
        }

      return tmps.size();
    }

  }

}

#endif
