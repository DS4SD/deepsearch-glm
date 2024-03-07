//-*-C++-*-

#ifndef ANDROMEDA_GLM_MODEL_OPS_QUERY_FLOWOP_UTILS_H
#define ANDROMEDA_GLM_MODEL_OPS_QUERY_FLOWOP_UTILS_H

namespace andromeda
{
  namespace glm
  {
    std::shared_ptr<query_baseop> to_flowop(std::shared_ptr<model> model_ptr,
					    typename query_baseop::flow_op_type flop,
					    typename query_baseop::flow_id_type flid,
					    std::set<typename query_baseop::flow_id_type> deps,
					    const nlohmann::json& config)
    {
      //LOG_S(INFO) << "config: " << config.dump(2);
      
      std::shared_ptr<query_baseop> op(NULL);
      
      switch(flop)
	{
        case SELECT:
	  {	   	    
	    typedef query_flowop<SELECT> flowop_type;
	    op = std::make_shared<flowop_type>(model_ptr, flid, deps, config);
	  }
	  break;
	  
        case TRAVERSE:
	  {
	    typedef query_flowop<TRAVERSE> flowop_type;
	    op = std::make_shared<flowop_type>(model_ptr, flid, deps, config);
	  }
	  break;
	  
        case FILTER: 
	  {
	    typedef query_flowop<FILTER> flowop_type;
	    op = std::make_shared<flowop_type>(model_ptr, flid, deps, config);
	  }
	  break;

        case INTERSECT:
	  {
	    typedef query_flowop<INTERSECT> flowop_type;
	    op = std::make_shared<flowop_type>(model_ptr, flid, deps, config);
	  }
	  break;
	  
        case JOIN:
	  {
	    typedef query_flowop<JOIN> flowop_type;
	    op = std::make_shared<flowop_type>(model_ptr, flid, deps, config);
	  }
	  break;
	  
        case UNIFORM: 
	  {
	    typedef query_flowop<UNIFORM> flowop_type;
	    op = std::make_shared<flowop_type>(model_ptr, flid, deps, config);
	  }
	  break;

        case SUBGRAPH: 
	  {
	    typedef query_flowop<SUBGRAPH> flowop_type;
	    op = std::make_shared<flowop_type>(model_ptr, flid, deps, config);
	  }
	  break;
	  
	default:
	  {
	    LOG_S(ERROR) << "can not make a query flow-operations with config: "
			 << config.dump(2);
	  }
	}

      return op;
    }
    
    std::shared_ptr<query_baseop> to_flowop(const nlohmann::json& config,
					    std::shared_ptr<model> model_ptr)
    {
      typedef query_baseop::flow_op_type flow_op_type;
      typedef query_baseop::flow_id_type flow_id_type;

      try
	{
	  std::string flop_name = config[query_baseop::flop_lbl].get<std::string>();
	  
	  flow_op_type flop = to_flowop_name(flop_name);
	  flow_id_type flid = config[query_baseop::flid_lbl].get<flow_id_type>();
      
	  std::set<flow_id_type> deps={};      
	  deps = config.value(query_baseop::deps_lbl, deps);      
      
	  return to_flowop(model_ptr, flop, flid, deps, config);
	}
      catch(std::exception& exc)
	{
	  LOG_S(ERROR) << exc.what();
	}
      
      return NULL;
    }
    
  }
  
}

#endif
