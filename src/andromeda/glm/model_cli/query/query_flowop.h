//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_H_

namespace andromeda
{
  namespace glm
  {
    enum flowop_name
      {
       FLOWOP_DEFAULT,
       SELECT,

       TRAVERSE, 

       FILTER,
       JOIN, INTERSECT,
       
       UNIFORM,

       SUBGRAPH
      };

    const static std::vector<flowop_name> FLOWOP_NAMES = 
      {
       SELECT,

       TRAVERSE, 

       FILTER, 
       JOIN, INTERSECT,
       
       UNIFORM,

       SUBGRAPH
      };
    
    std::string to_string(flowop_name name)
    {
      switch(name)
        {
        case FLOWOP_DEFAULT: { return "FLOWOP_DEFAULT"; }

        case SELECT: { return "SELECT"; }
	  
        case TRAVERSE: { return "TRAVERSE"; }

        case FILTER: { return "FILTER"; }

        case JOIN: { return "JOIN"; }
        case INTERSECT: { return "INTERSECT"; }

        case UNIFORM: { return "UNIFORM"; }

	case SUBGRAPH: { return "SUBGRAPH"; }
        }

      return "FLOWOP_DEFAULT";
    }

    flowop_name to_flowop_name(const std::string& name)
    {
      std::string upper_name = utils::to_upper(name);
      
      for(auto item_name:FLOWOP_NAMES)
	{
	  if(to_string(item_name)==upper_name)
	    {
	      return item_name;
	    }
	}

      return FLOWOP_DEFAULT;
    }
    
  }

}

#include <andromeda/glm/model_cli/query/query_flowop/base.h>
#include <andromeda/glm/model_cli/query/query_flowop/impl.h>

#include <andromeda/glm/model_cli/query/query_flowop/utils.h>

#endif
