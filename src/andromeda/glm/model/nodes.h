//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_NODES_H_
#define ANDROMEDA_MODELS_GLM_NODES_H_

#include <andromeda/glm/model/nodes/base.h>
#include <andromeda/glm/model/nodes/base_node.h>

namespace andromeda
{
  namespace glm
  {

    class glm_nodes: public base_types
    {
    public:

      typedef base_node node_type;

      typedef std::pair<flvr_type, ind_type> key_type;

      typedef          std::vector<node_type>   node_coll_type;      
      typedef typename node_coll_type::iterator node_itr_type;

      typedef std::map<flvr_type, node_coll_type> flvr_map_type;
      typedef typename flvr_map_type::iterator    flvr_itr_type;

      typedef std::unordered_map<hash_type, key_type> hash_map_type;
      
    public:

      glm_nodes();
      ~glm_nodes();

      bool is_consistent();
      
      double load_factor() { return hash_to_key.load_factor(); }
      double max_load_factor() { return hash_to_key.max_load_factor(); }

      std::size_t size() { return hash_to_key.size(); }
      std::size_t size(flvr_type flvr) { return flvr_colls.at(flvr).size(); }

      node_type& at(key_type key) { return flvr_colls.at(key.first).at(key.second); }
      node_coll_type& at(flvr_type flvr) { return flvr_colls.at(flvr); }
      
      flvr_itr_type begin() { return flvr_colls.begin(); }
      flvr_itr_type end() { return flvr_colls.end(); }

      node_itr_type begin(flvr_type flvr) { return flvr_colls.at(flvr).begin(); }
      node_itr_type end(flvr_type flvr) { return flvr_colls.at(flvr).end(); }

      void show_bucket_distribution();
      
      void clear();
      void reserve(std::size_t N);
      
      void initialise();
      
      node_type& push_back(node_type& node);

      bool       has(hash_type hash);      
      node_type& get(hash_type hash);
      
      bool get(hash_type hash, node_type& node);

      flvr_type get_flvr(hash_type hash);
      
      node_type& insert(flvr_type flavor, std::string text);
      node_type& insert(flvr_type flavor, std::vector<hash_type> path);
      node_type& insert(node_type& other, bool check_size);
      
      void sort();
      void sort(flvr_type flavor);
      
    private:

      std::size_t max_allowed_size;
      
      flvr_map_type flvr_colls;      
      hash_map_type hash_to_key;
    };

    glm_nodes::glm_nodes():
      max_allowed_size(-1)
    {
      initialise();
    }

    glm_nodes::~glm_nodes()
    {}

    bool glm_nodes::is_consistent()
    {
      //LOG_S(INFO);

      std::size_t tot=0;
      std::set<hash_type> hashes={};

      for(auto itr=begin(); itr!=end(); itr++)
	{
	  tot += (itr->second).size();
	  
	  for(auto& node:itr->second)
	    {
	      if(not has(node.get_hash()))
		{
		  LOG_S(WARNING) << "node-hash not known: " << node.to_json().dump(2);
		}
	      
	      if(hashes.count(node.get_hash()))
		{
		  LOG_S(WARNING) << "node-hash duplicated: " << node.to_json().dump(2);
		}
	      
	      hashes.insert(node.get_hash());
	    }
	}
      LOG_S(INFO) << __FUNCTION__ << " --> total nodes: " << tot << " [" << size() << "]";

      return (tot==size());
    }
    
    void glm_nodes::show_bucket_distribution()
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

    void glm_nodes::clear()
    {
      hash_to_key.clear();
      flvr_colls.clear();
    }
    
    void glm_nodes::initialise()
    {
      clear();
      
      for(auto itr=node_names::begin(); itr!=node_names::end(); itr++)
	{
	  flvr_colls[itr->first].reserve(1e6);
	}

      for(std::string name:node_names::TOKEN_NAMES)	
	{
	  auto& node = this->insert(node_names::WORD_TOKEN, name);
	  node_names::to_hash[name] = node.get_hash();
	}
      
      for(std::string name:node_names::LABEL_NAMES)	
	{
	  auto& node = this->insert(node_names::LABEL, name);
	  node_names::to_hash[name] = node.get_hash();
	}
      
      reserve(1e7);
    }

    void glm_nodes::reserve(std::size_t N)
    {      
      hash_to_key.reserve(N);
      hash_to_key.max_load_factor(32.0);
    }
    
    typename glm_nodes::node_type& glm_nodes::push_back(node_type& node)
    {
      flvr_type flvr = node.get_flvr();
      hash_type hash = node.get_hash();

      if(flvr_colls.count(flvr)==0)
	{
	  flvr_colls[flvr].reserve(1e3);
	}
      
      auto& flvr_coll = flvr_colls.at(flvr);

      ind_type ind = flvr_coll.size();
      key_type key(flvr,ind);
      
      hash_to_key.insert(std::make_pair(hash, key));
      flvr_coll.push_back(node);

      return flvr_coll.back();
    }

    bool glm_nodes::has(index_type hash)
    {
      return (hash_to_key.count(hash)>0);
    }

    typename glm_nodes::node_type& glm_nodes::get(hash_type hash)
    {
      return this->at(hash_to_key.at(hash));
    }
    
    bool glm_nodes::get(hash_type hash, node_type& node)
    {
      auto itr = hash_to_key.find(hash);      
      
      if(itr!=hash_to_key.end() and itr->first==hash)
        {
	  node = this->at(itr->second);
	  return true;
        }

      return false;
    }

    typename glm_nodes::flvr_type glm_nodes::get_flvr(hash_type hash)
    {
      auto itr = hash_to_key.find(hash);      
      
      if(itr!=hash_to_key.end() and itr->first==hash)
	{
	  return (itr->second).first;
	}

      return node_names::UNKNOWN_FLVR;
    }
    
    typename glm_nodes::node_type& glm_nodes::insert(flvr_type flavor, std::string text)
    {
      node_type node(flavor, text);
      hash_type hash = node.get_hash();
      
      auto itr = hash_to_key.find(hash);
      if(itr!=hash_to_key.end() and itr->first==hash)
	{
	  return this->at(itr->second);      
	}
      else
	{
	  return push_back(node);
	}            
    }

    typename glm_nodes::node_type& glm_nodes::insert(flvr_type flavor, std::vector<hash_type> path)
    {
      node_type node(flavor, path);
      hash_type hash = node.get_hash();
      
      auto itr = hash_to_key.find(hash);
      if(itr!=hash_to_key.end() and itr->first==hash)
	{
	  return this->at(itr->second);      
	}
      else
	{
	  return push_back(node);
	}            
    }    

    typename glm_nodes::node_type& glm_nodes::insert(node_type& other, bool check_size)
    {
      auto itr = hash_to_key.find(other.get_hash());

      if(itr!=hash_to_key.end() and itr->first==other.get_hash())
        {
	  auto& node = this->at(itr->second);
	  node.update(other);

	  return node;
	}
      else if((not check_size) or size()<max_allowed_size)
	{
	  return push_back(other);
	}
      else
        {
	  static bool warned=false;

	  if(not warned)
	    {
	      LOG_S(WARNING) << "exceeding reserved node-size (" << max_allowed_size << ")";
	      warned=true;
	    }

	  return other;
        }
    }

    void glm_nodes::sort(flvr_type flvr)
    {
      if(flvr_colls.count(flvr)==0)
	{
	  return;
	}
      
      auto& coll = flvr_colls.at(flvr);
      std::sort(coll.begin(), coll.end());

      for(std::size_t ind=0; ind<coll.size(); ind++)
	{
	  node_type& node = coll.at(ind);
	  
	  flvr_type flvr = node.get_flvr(); 
	  hash_type hash = node.get_hash();
	  
	  hash_to_key.at(hash) = key_type({flvr, ind});
	}	  
    }
    
    void glm_nodes::sort()
    {
      LOG_S(INFO) << __FUNCTION__;
      for(auto itr=flvr_colls.begin(); itr!=flvr_colls.end(); itr++)
	{
	  sort(itr->first);
	}
      
      /*
      {
	LOG_S(INFO) << " --> sorting nodes ... ";
	for(auto itr=flvr_colls.begin(); itr!=flvr_colls.end(); itr++)
	  {
	    auto& coll = itr->second;
	    std::sort(coll.begin(), coll.end());
	  }
      }

      {
	LOG_S(INFO) << " --> updating node hash-table ... ";

	for(auto itr=flvr_colls.begin(); itr!=flvr_colls.end(); itr++)
	  {
	    auto& coll = itr->second;
	    for(std::size_t ind=0; ind<coll.size(); ind++)
	      {
		node_type& node = coll.at(ind);

		flvr_type flvr = node.get_flvr(); 
		hash_type hash = node.get_hash();
		
		hash_to_key.at(hash) = key_type({flvr, ind});
	      }
	  }
      }    
      */  
    }
    
  }

}

#endif
