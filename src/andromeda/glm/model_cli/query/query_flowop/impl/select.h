//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_SELECT_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_SELECT_H_

namespace andromeda
{
  namespace glm
  {
    
    template<>
    class query_flowop<SELECT>: public query_baseop
    {
      const static flowop_name NAME = SELECT;

      typedef query_baseop baseop_type;

    public:

      query_flowop(std::shared_ptr<model_type> model,
		   flow_id_type id, std::set<flow_id_type> dependencies,
		   const nlohmann::json& config);
      
      query_flowop(flow_id_type id, std::shared_ptr<model_type> model,
                   const std::vector<std::vector<std::string> >& paths);

      query_flowop(flow_id_type id, std::shared_ptr<model_type> model,
                   const glm_node_type& node);

      query_flowop(flow_id_type id, std::shared_ptr<model_type> model,
                   std::vector<glm_node_type>& nodes);

      query_flowop(flow_id_type id, std::shared_ptr<model_type> model,
                   std::vector<qry_node_type>& nodes);

      virtual ~query_flowop();

      virtual nlohmann::json to_config();// { return nlohmann::json::object({}); }
      virtual bool from_config(const nlohmann::json& config);// { return false;}

      virtual bool execute(results_type& results);

    private:
      
      bool set_hashes_from_nodes();
      
    private:

      std::vector<std::vector<std::string> > nodes;           
      std::vector<std::pair<hash_type, val_type> > hashes;      
    };

    query_flowop<SELECT>::query_flowop(std::shared_ptr<model_type> model,
				       flow_id_type flid, std::set<flow_id_type> dependencies,
				       const nlohmann::json& config):
      query_baseop(model, NAME, flid, dependencies),
      nodes({}),
      hashes({})
    {
      if((not config.is_null()) and
	 (not from_config(config)))
	{
	  LOG_S(WARNING) << "implement query_flowop<" << to_string(NAME) << "> "
			 << "with config: " << config.dump(2);
	}
    }

    nlohmann::json query_flowop<SELECT>::to_config()
    {
      nlohmann::json config = query_baseop::to_config();

      nlohmann::json& params = config.at(parameters_lbl);
      {
	if(nodes.size()>0 or (nodes.size()==0 and hashes.size()==0))
	  {
	    params["nodes"] = nodes;
	  }
	else if(hashes.size()>0)
	  {
	    params["hashes"] = hashes;	
	  }
	else
	  {
	    LOG_S(WARNING) << __FILE__ << ":" << __LINE__ << "\t"
			   << "nodes and hashes are empty ...";
	  }
      }
      
      return config;
    }
    
    bool query_flowop<SELECT>::from_config(const nlohmann::json& config)
    {
      query_baseop::set_output_parameters(config);
      
      nlohmann::json params = config;
      if(config.count(parameters_lbl))
	{
	  params = config.at(parameters_lbl);
	}
      
      nodes.clear();
      hashes.clear();

      try
	{
	  hashes = params.value("hashes", hashes);
	}
      catch(std::exception& exc)
	{
	  hashes.clear();
	}

      if(hashes.size()>0)
	{
	  return true;
	}
      
      try
	{
	  nodes = params.value("nodes", nodes);
	}
      catch(std::exception& exc)
	{
	  LOG_S(WARNING) << exc.what();
	  return false;
	}

      return true;
    }
    
    query_flowop<SELECT>::query_flowop(flow_id_type flid, std::shared_ptr<model_type> model,
                                       const std::vector<std::vector<std::string> >& nodes):
      query_baseop(model, NAME, flid, {}),
      nodes(nodes)
    {}

    query_flowop<SELECT>::query_flowop(flow_id_type flid,
                                       std::shared_ptr<model_type> model,
                                       const glm_node_type& node):
      query_baseop(model, NAME, flid, {}),
      nodes({})
    {
      hashes.clear();
      hashes.emplace_back(node.get_hash(), 1.0);
    }

    query_flowop<SELECT>::query_flowop(flow_id_type flid,
                                       std::shared_ptr<model_type> model,
                                       std::vector<glm_node_type>& nodes):
      query_baseop(model, NAME, flid, {}),
      nodes({})
    {
      hashes.clear();
      for(auto& node:nodes)
        {
          hashes.emplace_back(node.get_hash(), 1.0);
        }
    }

    query_flowop<SELECT>::query_flowop(flow_id_type flid,
                                       std::shared_ptr<model_type> model,
                                       std::vector<qry_node_type>& nodes):
      query_baseop(model, NAME, flid, {}),
      nodes({})
    {
      hashes.clear();
      for(auto& node:nodes)
        {
          hashes.emplace_back(node.hash, node.weight);
        }
    }
    
    query_flowop<SELECT>::~query_flowop()
    {}
    
    bool query_flowop<SELECT>::execute(results_type& results)
    {
      if(nodes.size()>0 and hashes.size()==0)
	{
	  if(not set_hashes_from_nodes())
	    {
	      baseop_type::done = false;
	      return baseop_type::done;
	    }
	}
      
      auto& target = results.at(baseop_type::flid);
      for(auto& hash:hashes)
        {
          target->set(hash.first, 1, hash.second);
        }

      target->normalise();

      baseop_type::done = true;
      return baseop_type::done;
    }

    bool query_flowop<SELECT>::set_hashes_from_nodes()
    {
      if(model_ptr==NULL)
	{
	  return false;
	}
      
      auto& model_nodes = model_ptr->get_nodes();
      
      hashes.clear();      
      for(const std::vector<std::string>& node:nodes)
        {
	  if(node.size()==1)
	    {
	      // we are looking for TOKEN, SYNTX or LABEL
	      std::vector<hash_type> token_hashes={};	      

	      //for(auto keyval:node_names::to_name)
	      for(auto itr=node_names::begin(); itr!=node_names::end(); itr++)
		{
		  base_node bnode(itr->first, node.at(0));
		  if(model_nodes.has(bnode.get_hash()))
		    {
		      hashes.emplace_back(bnode.get_hash(), 1.0);
		      token_hashes.push_back(bnode.get_hash());
		    }
		}

	      // we are looking for TERM, VERB, CONC or CONT with a length==1
	      for(auto thash:token_hashes)
		{
		  //for(auto keyval:node_names::to_name)
		  for(auto itr=node_names::begin(); itr!=node_names::end(); itr++)
		    {
		      std::vector<hash_type> path={thash};
		      base_node bnode(itr->first, path);

		      if(model_nodes.has(bnode.get_hash()))
			{ 
			  hashes.emplace_back(bnode.get_hash(), 1.0);	  
			}
		    }
		}
	    }
	  else
	    {
	      std::vector<hash_type> phashes={};
	      for(const std::string& token:node)
		{
		  base_node bnode(node_names::WORD_TOKEN, token);
		  phashes.push_back(bnode.get_hash());
		}
	      
	      //for(auto keyval:node_names::to_name)
	      for(auto itr=node_names::begin(); itr!=node_names::end(); itr++)
		{		      
		  base_node bnode(itr->first, phashes);
		  if(model_nodes.has(bnode.get_hash()))
		    { 
		      hashes.emplace_back(bnode.get_hash(), 1.0);	  
		    }
		}
	    }
	}
      
      return (hashes.size()>0);
    }
    
  }

}

#endif
