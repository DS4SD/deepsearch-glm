//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_QUERY_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_QUERY_H_

#include <andromeda/glm/model_cli/query/query_result/query_node.h>
#include <andromeda/glm/model_cli/query/query_result/query_edge.h>
#include <andromeda/glm/model_cli/query/query_result.h>

#include <andromeda/glm/model_cli/query/query_flowop.h>
#include <andromeda/glm/model_cli/query/query_flow.h>

namespace andromeda
{
  namespace glm
  {

    template<typename model_type>
    class model_cli<QUERY, model_type>
    {
      const static inline std::string mode_lbl = "mode";
      const static inline std::string query_lbl = to_string(QUERY);

      const static inline std::string queries_lbl = "queries";
      
      typedef typename model_type::index_type index_type;

      typedef typename model_type::node_type node_type;
      typedef typename model_type::edge_type edge_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef query_flow<model_type> qflow_type;

    public:

      model_cli(std::shared_ptr<model_type> model);
      ~model_cli();

      nlohmann::json to_config();

      void execute(const nlohmann::json& queries,
		   nlohmann::json& result, bool verbose);
      
    private:

      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    model_cli<QUERY, model_type>::model_cli(std::shared_ptr<model_type> model):
      model(model)
    {}

    template<typename model_type>
    model_cli<QUERY, model_type>::~model_cli()
    {}

    template<typename model_type>
    nlohmann::json model_cli<QUERY, model_type>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      config["mode"] = to_string(QUERY);

      {
	nlohmann::json item = model_op<SAVE>::to_config();
	config.merge_patch(item);
      }
      
      {
	nlohmann::json item = model_op<LOAD>::to_config();
	config.merge_patch(item);
      }
      
      // Queries
      {
	nlohmann::json queries = nlohmann::json::array({});
	
	query_flow<model_type> qflow(model);
	for(flowop_name flop:FLOWOP_NAMES)
	  {
	    nlohmann::json config = qflow.to_config();
	    
	    nlohmann::json params;
	    auto op = to_flowop(model, flop, 0, {}, params);
	    
	    if(op!=NULL)
	      {
		config[qflow_type::flow_lbl].push_back(op->to_config());
		queries.push_back(config);
	      }
	  }

	config[queries_lbl] = queries;
      }
      
      return config;
    }

    template<typename model_type>
    void model_cli<QUERY, model_type>::execute(const nlohmann::json& config,
					       nlohmann::json& result, bool verbose)
    {
      if(config.count(queries_lbl)==1)
	{
	  execute(config.at(queries_lbl), result, verbose);
	}
      else if(config.is_array())
	{
	  result = nlohmann::json::array({});
	  for(const auto& query:config)
	    {
	      nlohmann::json item = nlohmann::json::object({});
	      execute(query, item, verbose);

	      result.push_back(item);
	    }
	}
      else if(config.count(qflow_type::flow_lbl)==1)
	{
	  query_flow<model_type> qflow(model);
	  bool success = qflow.execute(config);
	  
	  if(success and verbose)
	    {
	      qflow.show();

	      for(auto itr=qflow.begin(); itr!=qflow.end(); itr++)
		{		  
		  ((*itr)->get_nodeset())->show();
		}
	    }
	  else if(verbose)
	    {
	      LOG_S(WARNING) << "could not execute with config: \n"
			     << config.dump(2);
	    }
	  else
	    {}

	  if(success)
	    {
	      result = qflow.to_json();
	    }
	  else
	    {}
	}
      else
	{
	  LOG_S(WARNING) << "could not parse config: \n"
			 << config.dump(2);
	}
    }
    
  }

}

#endif
