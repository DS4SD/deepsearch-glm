//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_H_

namespace andromeda
{
  namespace glm
  {
    enum model_cli_name
      {
       UNDEF,
       CREATE_CONFIGS,
       CREATE, DISTILL,
       QUERY, EXPLORE
      };
    
    std::string to_string(model_cli_name name)
      {
	switch(name)
	  {
	  case UNDEF: return "undefined";
	  case CREATE_CONFIGS: return "create-configs";

	  case CREATE: return "create";
	  case DISTILL: return "distill";

	  case QUERY: return "query";
	  case EXPLORE: return "explore";
	  }

	return "undefined";
      }

    model_cli_name to_model_cli_name(std::string text)
    {
      if(text==to_string(CREATE_CONFIGS))
	{
	  return CREATE_CONFIGS;
	}
      else if(text==to_string(CREATE))
	{
	  return CREATE;
	}
      else if(text==to_string(DISTILL))
	{
	  return DISTILL;
	}
      else if(text==to_string(QUERY))
	{
	  return QUERY;
	}      
      else if(text==to_string(EXPLORE))
	{
	  return EXPLORE;
	}
      else
	{
	  return UNDEF;
	}

      return UNDEF;
    }
    
    template<model_cli_name name, typename model_type>
    class model_cli
    {};
    
  }
  
}

#include <andromeda/glm/model_cli/create.h>
#include <andromeda/glm/model_cli/distill.h>

#include <andromeda/glm/model_cli/query.h>
#include <andromeda/glm/model_cli/explore.h>

#endif
