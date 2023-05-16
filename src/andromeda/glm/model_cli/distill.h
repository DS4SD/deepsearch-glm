//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_DISTILL_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_DISTILL_H_

#include <andromeda/glm/model_cli/distill/config.h>

namespace andromeda
{
  namespace glm
  {

    template<typename model_type>
    class model_cli<DISTILL, model_type>
    {
      typedef typename model_type::index_type index_type;

      typedef typename model_type::node_type node_type;
      typedef typename model_type::edge_type edge_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

    public:

      model_cli(std::shared_ptr<model_type> model);
      ~model_cli();
      
      nlohmann::json to_config();
      void from_config(const nlohmann::json& config);
      
      std::shared_ptr<model_type> distill();

    private:

      std::shared_ptr<model_type> old_model, new_model;

      distill_config configuration;
    };

    template<typename model_type>
    model_cli<DISTILL, model_type>::model_cli(std::shared_ptr<model_type> model):
      
      old_model(model),
      new_model(NULL),

      configuration()
    {}

    template<typename model_type>
    model_cli<DISTILL, model_type>::~model_cli()
    {}

    template<typename model_type>
    nlohmann::json model_cli<DISTILL, model_type>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      config["mode"] = to_string(DISTILL);
      
      {
	auto item = configuration.get();      
	config.merge_patch(item);
      }

      {
	auto item = model_op<SAVE>::to_config();
	config.merge_patch(item);
      }
      
      {
	auto item = model_op<LOAD>::to_config();
	config.merge_patch(item);
      }    
      
      return config;
    }

    template<typename model_type>
    void model_cli<DISTILL, model_type>::from_config(const nlohmann::json& config)
    {
      configuration.set(config);
    }
    
    template<typename model_type>
    std::shared_ptr<model_type> model_cli<DISTILL, model_type>::distill()
    {
      new_model = std::make_shared<model_type>(old_model->get_parameters());
      new_model->initialise();

      std::unordered_set<std::size_t> hashes={}, skipping={};

      {
        auto& old_nodes = old_model->get_nodes();

	std::set<std::string> protected_names={};
	{
	  for(auto name:node_names::TOKEN_NAMES)
	    {
	      protected_names.insert(name);
	    }
	  
	  for(auto name:node_names::LABEL_NAMES)
	    {
	      protected_names.insert(name);
	    }	
	}
	
        for(auto& flvr_coll:old_nodes)
          {
	    for(auto& node:flvr_coll.second)
	      {
		if(protected_names.count(node.get_text())==1)
		  {
		    continue;
		  }
	      }
	  }

	LOG_S(INFO) << "#-skipping: " << skipping.size();
      }
      
      {
        auto& old_edges = old_model->get_edges();
        auto& new_edges = new_model->get_edges();

        for(auto& flvr_coll:old_edges)
          {
	    for(auto& edge:flvr_coll.second)
	      {        
		if(edge.get_count()>=configuration.get_min_edge_count() and
		   skipping.count(edge.get_hash_i())==0 and
		   skipping.count(edge.get_hash_j())==0 )
		  {
		    new_edges.insert(edge, false);

		    hashes.insert(edge.get_hash_i());
		    hashes.insert(edge.get_hash_j());
		  }
	      }
	  }

	LOG_S(INFO) << "old #-edges: " << old_edges.size();
	LOG_S(INFO) << "new #-edges: " << new_edges.size();	
      }

      {
        auto& old_nodes = old_model->get_nodes();
        auto& new_nodes = new_model->get_nodes();

        for(auto& flvr_coll:old_nodes)
          {
	    for(auto& node:flvr_coll.second)
	      {
		if(hashes.count(node.get_hash())==1)
		  {
		    assert(skipping.count(node.get_hash())==0);
		    new_nodes.insert(node, false);
		  }
	      }
	  }

	LOG_S(INFO) << "old #-nodes: " << old_nodes.size();
	LOG_S(INFO) << "new #-nodes: " << new_nodes.size();
      }

      {
        new_model->finalise();
      }

      return new_model;
    }

  }

}
  
#endif
