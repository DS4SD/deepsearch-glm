//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_TRAVERSE_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_TRAVERSE_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class query_flowop<TRAVERSE>: public query_baseop
    {
      const static flowop_name NAME = TRAVERSE;

      typedef query_baseop baseop_type;

      typedef typename baseop_type::flow_id_type flow_id_type;
      typedef typename baseop_type::results_type results_type;

    public:

      query_flowop(std::shared_ptr<model_type> model,
		   flow_id_type flid, std::set<flow_id_type> dependencies,
		   const nlohmann::json& config);

      query_flowop(std::shared_ptr<model_type> model,
		   flow_id_type flid, std::set<flow_id_type> dependencies,
		   flvr_type edge_type);

      virtual ~query_flowop();

      virtual nlohmann::json to_config();
      virtual bool from_config(const nlohmann::json& config);
      
      virtual bool execute(results_type& results);

    private:

      flvr_type edge_flavor;
    };

    query_flowop<TRAVERSE>::query_flowop(std::shared_ptr<model_type> model,
					 flow_id_type flid, std::set<flow_id_type> dependencies,
					 const nlohmann::json& config):
      query_baseop(model, NAME, flid, dependencies),
      edge_flavor(1)
    {
      if((not config.is_null()) and (not from_config(config)))
	{
	  LOG_S(WARNING) << "implement query_flowop<" << to_string(NAME) << "> "
			 << "with config: " << config.dump(2);
	}
    }

    query_flowop<TRAVERSE>::query_flowop(std::shared_ptr<model_type> model,
					 flow_id_type flid, std::set<flow_id_type> dependencies,
					 flvr_type edge_flavor):
      query_baseop(model, NAME, flid, dependencies),
      edge_flavor(edge_flavor)
    {}
    
    nlohmann::json query_flowop<TRAVERSE>::to_config()
    {
      nlohmann::json config = query_baseop::to_config();

      nlohmann::json& params = config.at(parameters_lbl);
      {
	params["edge"] = edge_names::to_name(edge_flavor);	
	params["sources"] = query_baseop::dependencies;
      }
      
      return config;
    }
    
    bool query_flowop<TRAVERSE>::from_config(const nlohmann::json& config)
    {
      query_baseop::set_output_parameters(config);
      
      nlohmann::json params = config;
      if(config.count(parameters_lbl))
	{
	  params = config.at(parameters_lbl);
	}
      
      try
	{
	  std::string edge_name = params["edge"].get<std::string>();
	  edge_flavor = edge_names::to_flvr(edge_name);
	}
      catch(std::exception& exc)
	{
	  LOG_S(WARNING) << "traverse parameters: " << config.dump(2) << "\n"
			 << " -> error: " << exc.what();
	  return false;
	}

      return true;
    }    

    query_flowop<TRAVERSE>::~query_flowop()
    {}

    bool query_flowop<TRAVERSE>::execute(results_type& results)
    {
      auto& edges = baseop_type::model_ptr->get_edges();

      auto& target = results.at(baseop_type::flid);

      for(auto sid:baseop_type::dependencies)
	{      
	  auto& source = results.at(sid);	  
	  source->normalise();
	  
	  for(auto itr=source->begin(); itr!=source->end(); itr++)      
	    {
	      std::vector<typename model_type::edge_type> _edges;
	      edges.traverse(edge_flavor, itr->hash, _edges, true);
	      
	      for(auto& _edge:_edges)
		{
		  target->add(_edge.get_hash_j(), _edge.get_count(), (itr->prob)*_edge.get_prob());
		}
	    }
	}

      target->normalise();
      
      baseop_type::done = true;
      return baseop_type::done;
    }

  }

}

#endif
