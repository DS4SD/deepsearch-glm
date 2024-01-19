//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_GLM_INTERFACE_MODEL_H
#define PYBIND_ANDROMEDA_GLM_INTERFACE_MODEL_H

#include "andromeda.h"

namespace andromeda_py
{
  class glm_model: public base_log,
		   public base_resources,
		   public andromeda::base_types,
                   public andromeda::glm::model_types
  {
    typedef andromeda::glm::model                      glm_model_type;
    typedef andromeda::glm::query_flow<glm_model_type> glm_flow_type;

    typedef typename glm_flow_type::flow_id_type flow_id_type;

    typedef andromeda::glm::model_op<andromeda::glm::LOAD> io_load_type;
    typedef andromeda::glm::model_op<andromeda::glm::SAVE> io_save_type;

  public:

    glm_model();
    ~glm_model();

    //void set_resource_dir(const std::string resources_dir);
    
    void load_dir(const std::string root_dir);
    void save_dir(const std::string root_dir);

    void load(nlohmann::json config);
    void save(nlohmann::json config);

    nlohmann::json get_topology();
    nlohmann::json get_configurations();

    nlohmann::json create(nlohmann::json params);

    nlohmann::json distill(nlohmann::json params);

    nlohmann::json apply_on_text(std::string& text);

    void explore(nlohmann::json params);

    nlohmann::json query(nlohmann::json params);

  private:

    void query_generic(const nlohmann::json& params,
                       nlohmann::json& result);

    void query_word(const nlohmann::json& params,
                    nlohmann::json& result);

    void query_subgraph(const nlohmann::json& params,
                        nlohmann::json& result);

    void query_context(const nlohmann::json& params,
                       nlohmann::json& result);

  private:

    std::shared_ptr<glm_model_type> model;
  };

  glm_model::glm_model():
    base_log::base_log(),
    base_resources::base_resources(),

    model(std::make_shared<glm_model_type>())
  {}
  
  glm_model::~glm_model()
  {}

  /*
  void glm_model::set_resource_dir(const std::string resources_dir)
  {
    //andromeda::RESOURCES_DIR = resources_dir;
    std::filesystem::path path(resources_dir.c_str());
    
    if(not andromeda::glm_variables::set_resources_dir(resources_dir))
      {
	LOG_S(FATAL) << "resource-dir `" << resources_dir << "` is invalid!";
      }
  }
  */
  
  void glm_model::load_dir(const std::string root_dir)
  {
    std::filesystem::path path(root_dir.c_str());

    io_load_type io;
    io.load(path, model);
  }

  void glm_model::save_dir(const std::string root_dir)
  {
    std::filesystem::path path(root_dir.c_str());

    io_save_type io;
    io.save(path, model);
  }

  void glm_model::load(nlohmann::json config)
  {
    io_load_type io;

    io.from_config(config);
    io.load(model);
  }

  void glm_model::save(nlohmann::json config)
  {
    io_save_type io;
    
    io.from_config(config);
    io.save(model);
  }

  nlohmann::json glm_model::get_topology()
  {
    auto& topo = model->get_topology();
    return topo.to_json();
  }
  
  nlohmann::json glm_model::get_configurations()
  {
    return andromeda::glm::get_configurations(model);
  }

  nlohmann::json glm_model::create(nlohmann::json config)
  {
    andromeda::glm::create_glm_model(config, model);

    auto& topo = model->get_topology();
    return topo.to_json();
  }

  nlohmann::json glm_model::distill(nlohmann::json config)
  {
    std::shared_ptr<glm_model_type> new_model=NULL;
    andromeda::glm::distill_glm_model(config, model, new_model);

    auto& topo = new_model->get_topology();
    return topo.to_json();
  }

  void glm_model::explore(nlohmann::json config)
  {
    andromeda::glm::explore_glm_model(config, model);
  }

  nlohmann::json glm_model::apply_on_text(std::string& text)
  {
    nlohmann::json result = nlohmann::json::object({});

    auto& nlp_models = (model->get_parameters()).models;

    andromeda::subject<andromeda::TEXT> subj;
    if(not subj.set_text(text))
      {
        LOG_S(WARNING) << "could not set text for paragraph ...";
        return result;
      }

    andromeda::producer<andromeda::TEXT> prod(nlp_models);
    if(not prod.apply(subj))
      {
        LOG_S(WARNING) << "could not set text for paragraph ...";
        return result;
      }

    return subj.to_json({});
  }

  nlohmann::json glm_model::query(nlohmann::json params)
  {
    nlohmann::json result = {{ "status", "error" }};

    std::string mode = "undefined";
    mode = params.value("mode", mode);

    if(mode=="word")
      {
        query_word(params, result);
      }
    else if(mode=="subgraph")
      {
        query_subgraph(params, result);
      }
    else if(mode=="context")
      {
        query_context(params, result);
      }
    else
      {
        query_generic(params, result);
      }

    return result;
  }

  void glm_model::query_generic(const nlohmann::json& config,
                                nlohmann::json& result)
  {
    andromeda::glm::query_flow<glm_model_type> flow(model);

    if(flow.execute(config))
      {
        result = flow.to_json();
        result["status"] = "success";
      }
    else
      {
        result["status"] = "error";
      }
  }

  void glm_model::query_word(const nlohmann::json& params,
                             nlohmann::json& result)
  {
    std::size_t max_nodes = 256;
    max_nodes = params.value("max-nodes", max_nodes);

    std::vector<std::string> words = {};
    words = params.value("words", words);

    std::vector<std::string> edges = { "prev", "next", "to-pos"};
    edges = params.value("edges", edges);

    andromeda::glm::query_flow<glm_model_type> flow(model);
    {
      auto op_0 = flow.add_select(words);

      for(std::string edge:edges)
        {
          auto op_x = flow.add_traverse(andromeda::glm::edge_names::to_flvr(edge),
                                        op_0->get_flid());
          op_x->get_nodeset()->set_name(edge);
        }

      flow.execute();
    }

    result = flow.to_json();
    result["status"]="success";
  }

  void glm_model::query_subgraph(const nlohmann::json& params,
                                 nlohmann::json& result)
  {
    std::size_t MAX_EDGES = 128;
    MAX_EDGES = params.value("max-edges", MAX_EDGES);

    std::vector<std::string> words = {};
    words = params.value("words", words);

    std::vector<std::string> trav_edges = { "prev", "next", "to-pos"};
    trav_edges = params.value("traverse-edges", trav_edges);

    
    std::vector<std::string> subg_edges = { "tax-up"};
    subg_edges = params.value("subgraph-edges", subg_edges);

    auto trav_flvrs = andromeda::glm::edge_names::to_flvr(trav_edges);
    auto subg_flvrs = andromeda::glm::edge_names::to_flvr(subg_edges);

    andromeda::glm::query_flow<glm_model_type> flow(model);
    {
      auto op_0 = flow.add_select(words);

      std::set<flow_id_type> sources={};
      for(flvr_type flvr:trav_flvrs)
        {
          auto op_x = flow.add_traverse(flvr, op_0->get_flid());
          op_x->get_nodeset()->set_name(andromeda::glm::edge_names::to_name(flvr));

          sources.insert(op_0->get_flid());
        }

      auto op_y = flow.add_subgraph(sources, true, subg_flvrs);

      flow.execute();

      flow.show();
    }

    /*
      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();

      std::set<hash_type> qhashes={};

      node_type node;

      for(auto op=flow.begin(); op!=flow.end(); op++)
      {
      auto nodeset = (*op)->get_nodeset();
      for(auto qnode=nodeset->begin(); qnode!=nodeset->end(); qnode++)
      {
      qhashes.insert(qnode->hash);
      }
      }

      auto& rnodes = result["nodes"];
      auto& redges = result["edges"];

      rnodes = nlohmann::json::object();
      redges = nlohmann::json::object();

      {
      rnodes["data"] = nlohmann::json::array();
      rnodes["headers"] = andromeda::glm::base_node::headers;
      }

      {
      redges["data"] = nlohmann::json::array();
      redges["headers"] = andromeda::glm::base_edge::headers;
      }

      std::vector<edge_type> tmp_edges={};

      for(auto qhash:qhashes)
      {
      if(nodes.get(qhash, node))
      {
      rnodes["data"].push_back(node.to_row());
      }

      for(flvr_type flvr:edge_flavors)
      {
      edges.traverse(flvr, qhash, tmp_edges, false);

      for(edge_type& tmp_edge:tmp_edges)
      {
      if(qhashes.count(tmp_edge.get_hash_i())==1 and
      qhashes.count(tmp_edge.get_hash_j())==1)
      {
      redges["data"].push_back(tmp_edge.to_row());
      }
      }
      }
      }
    */

    result = flow.to_json();
    result["status"]="success";
  }

  void glm_model::query_context(const nlohmann::json& params,
                                nlohmann::json& result)
  {
    std::size_t max_nodes = 256;
    max_nodes = params.value("max-nodes", max_nodes);

    std::vector<std::string> words = {};
    words = params.value("words", words);

    std::size_t mask_ind=words.size();
    for(std::size_t l=0; l<words.size(); l++)
      {
        if(words.at(l)=="???")
          {
            mask_ind=l;
          }
      }

    if(mask_ind==words.size())
      {
        result["status"]="error";
        result["message"]= "no word found with content `???` (:= mask)";

        return;
      }

    andromeda::glm::query_flow<glm_model_type> flow(model);
    {
      for(std::size_t i=0; i<words.size(); i++)
        {
	  flvr_type flvr = 0;	  
          if(i==mask_ind)
            {
              continue;
            }
	  else
	    {
	      std::size_t delta = mask_ind>i? mask_ind-i:i-mask_ind;

	      flvr = static_cast<flvr_type>(delta);
	      flvr *= (mask_ind>i? 1:-1);
	    }
	  
          auto op_i = flow.add_select(words.at(i));
          op_i->get_nodeset()->set_name("search "+words.at(i));
	  
          auto op_x = flow.add_traverse(flvr, op_i->get_flid());
          op_x->get_nodeset()->set_name(andromeda::glm::edge_names::to_name(flvr));
        }

      flow.execute();
    }

    result = flow.to_json();
    result["status"]="success";
  }

}

#endif
