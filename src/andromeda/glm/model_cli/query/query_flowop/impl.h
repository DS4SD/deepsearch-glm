//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_IMPL_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_IMPL_H_

namespace andromeda
{
  namespace glm
  {

    template<flowop_name name>
    class query_flowop: public query_baseop
    {
    public:

      query_flowop(std::set<flow_id_type> dependencies);

      virtual bool execute(results_type& results);

    private:

    };

    template<flowop_name name>
    query_flowop<name>::query_flowop(std::set<flow_id_type> dependencies):
      query_baseop(NULL, name, -1, dependencies)
    {
      LOG_S(WARNING);
    }

    template<flowop_name name>
    bool query_flowop<name>::execute(results_type& results)
    {
      LOG_S(WARNING);
      return false;
    }

  }

}

#include <andromeda/glm/model_cli/query/query_flowop/impl/select.h>
#include <andromeda/glm/model_cli/query/query_flowop/impl/uniform.h>
#include <andromeda/glm/model_cli/query/query_flowop/impl/filter.h>
#include <andromeda/glm/model_cli/query/query_flowop/impl/traverse.h>
#include <andromeda/glm/model_cli/query/query_flowop/impl/join.h>
#include <andromeda/glm/model_cli/query/query_flowop/impl/intersect.h>
#include <andromeda/glm/model_cli/query/query_flowop/impl/subgraph.h>

#endif
