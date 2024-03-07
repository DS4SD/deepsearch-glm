//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_SUBGRAPH_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_SUBGRAPH_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class query_flowop<SUBGRAPH>: public query_baseop
    {
      const static flowop_name NAME = SUBGRAPH;

      const static inline std::string dynamic_expansion_lbl = "dynamic-node-expansion";
      const static inline std::string edges_lbl = "edges";

    public:

      query_flowop(std::shared_ptr<model_type> model,
                   flow_id_type flid, std::set<flow_id_type> dependencies,
                   const nlohmann::json& config);

      query_flowop(flow_id_type flid,
		   std::shared_ptr<model_type> model,                   
		   std::set<flow_id_type> dependencies,
		   bool dynamic_expansion,
		   std::set<flvr_type> edge_flvrs);
      
      virtual ~query_flowop();

      virtual nlohmann::json to_config();
      virtual bool from_config(const nlohmann::json& config);

      virtual bool execute(results_type& results);

    private:

      bool dynamic_expansion;
      std::set<flvr_type> edge_flvrs;      
    };

    query_flowop<SUBGRAPH>::query_flowop(std::shared_ptr<model_type> model,
                                         flow_id_type flid,
					 std::set<flow_id_type> dependencies,
                                         const nlohmann::json& config):
      query_baseop(model, NAME, flid, dependencies)
    {
      if((not config.is_null()) and
         (not from_config(config)))
        {
          LOG_S(WARNING) << "implement query_flowop<" << to_string(NAME) << "> "
                         << "with config: " << config.dump(2);
        }
    }

    query_flowop<SUBGRAPH>::query_flowop(flow_id_type flid,
					 std::shared_ptr<model_type> model,
					 std::set<flow_id_type> dependencies,
					 bool dynamic_expansion,
					 std::set<flvr_type> edge_flvrs):
      query_baseop(model, NAME, flid, dependencies),
      
      dynamic_expansion(dynamic_expansion),
      edge_flvrs(edge_flvrs)
    {}
    
    query_flowop<SUBGRAPH>::~query_flowop()
    {}
    
    nlohmann::json query_flowop<SUBGRAPH>::to_config()
    {
      nlohmann::json config = query_baseop::to_config();

      nlohmann::json& params = config.at(parameters_lbl);
      {
	params[dynamic_expansion_lbl] = dynamic_expansion;

	if(edge_flvrs.size()==0)
	  {
	    params[edges_lbl] = std::vector<std::string>({"tax-up"});
	  }
	else
	  {
	    params[edges_lbl] = std::vector<std::string>({});
	    for(flvr_type flvr:edge_flvrs)
	      {
		params[edges_lbl].push_back(edge_names::to_name(flvr));
	      }
	  }
      }

      return config;
    }

    bool query_flowop<SUBGRAPH>::from_config(const nlohmann::json& config)
    {
      query_baseop::set_output_parameters(config);
      
      nlohmann::json params = config;
      if(config.count(parameters_lbl))
	{
	  params = config.at(parameters_lbl);
	}
      
      try
        {
	  dynamic_expansion = params.value(dynamic_expansion_lbl, false);

	  std::vector<std::string> edge_names = params[edges_lbl].get<std::vector<std::string> >();

	  edge_flvrs.clear();
	  for(auto edge_name:edge_names)
	    {
	      edge_flvrs.insert(edge_names::to_flvr(edge_name));
	    }
        }
      catch(std::exception& exc)
        {
          LOG_S(WARNING) << exc.what();
          return false;
        }

      return true;
    }

    bool query_flowop<SUBGRAPH>::execute(results_type& results)
    {
      //auto& nodes = model->get_nodes();
      auto& edges = model_ptr->get_edges();
      
      auto& target = results.at(query_baseop::flid);
      target->clear();
      
      for(auto& sid:query_baseop::dependencies)
        {
          auto& source = results.at(sid);
	  for(auto node_itr=source->begin(); node_itr!=source->end(); node_itr++)
	    {
	      target->add(*node_itr);
	    }
	}

      for(auto node_itr=target->begin(); node_itr!=target->end(); node_itr++)
	{
	  for(flvr_type flvr:edge_flvrs)
	    {
	      std::vector<base_edge> bedges={};
	      edges.traverse(flvr, node_itr->hash, bedges, false);
	      
	      for(const auto& bedge:bedges)
		{
		  if(target->has_node(bedge.get_hash_i())==1 and
		     target->has_node(bedge.get_hash_j())==1)
		    {
		      query_edge qedge(bedge.get_hash(), bedge.get_prob()); 
		      target->add(qedge);
		    }
		  else if(target->has_node(bedge.get_hash_i())==1)
		    {
		      query_node qnode(bedge.get_hash_j(), 1, bedge.get_prob()); 
		      target->add(qnode);
		      
		      query_edge qedge(bedge.get_hash(), bedge.get_prob()); 
		      target->add(qedge);
		    }
		  else
		    {}
		}
	    }
	}	

      target->normalise();

      query_baseop::done = true;      
      return query_baseop::done;
    }
    
  }

}

#endif
