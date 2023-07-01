//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_OPS_H_
#define ANDROMEDA_MODELS_GLM_MODEL_OPS_H_

namespace andromeda
{
  namespace glm
  {
    enum model_op_name
      {
       SAVE,
       LOAD,

       MERGE
      };
    
    std::string to_string(model_op_name name)
      {
	switch(name)
	  {
	  case SAVE: return "SAVE";
	  case LOAD: return "LOAD";

	  case MERGE: return "MERGE";
	  }

	return "UNKNOWN_MODELOP";
      }

    template<model_op_name name>
    class model_op
    {};
    
  }

}

#include <andromeda/glm/model_ops/io.h>
#include <andromeda/glm/model_ops/merge.h>

#endif
