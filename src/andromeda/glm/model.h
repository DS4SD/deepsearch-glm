//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_H
#define ANDROMEDA_MODELS_GLM_MODEL_H

#include <andromeda/glm/model/base.h>

#include <andromeda/glm/model/nodes.h>
#include <andromeda/glm/model/edges.h>

#include <andromeda/glm/model/utils/parameters.h>
#include <andromeda/glm/model/utils/topology.h>

namespace andromeda
{
  namespace glm
  {

    class model_types
    {
    public:

      typedef glm_parameters parameters_type;
      typedef glm_topology topology_type;

      typedef glm_nodes nodes_type;
      typedef glm_edges edges_type;

      typedef typename glm_nodes::node_type node_type;
      typedef typename glm_edges::edge_type edge_type;
    };
    
    class model: public base_types,
		 public model_types
    {
    public:
      
      typedef model this_type;
      
    public:
      
      model();
      model(parameters_type& params);

      model(nlohmann::json config, bool verbose);
      
      ~model();
      
      parameters_type& get_parameters() { return parameters; }
      topology_type& get_topology() { return topology; }
      
      nodes_type& get_nodes() { return nodes; }
      edges_type& get_edges() { return edges; }

      std::vector<node_type>& get_nodes(flvr_type flvr) { return nodes.at(flvr); }
      
      bool configure(nlohmann::json& config, bool verbose);

      bool initialise();

      bool initialise(index_type reserved_nodes,
		      index_type reserved_edges);

      bool finalise(bool compute_topo=true);
      
    private:
      
      parameters_type parameters;
      topology_type topology;
      
      nodes_type nodes;
      edges_type edges;
    };

    model::model():
      parameters(),
      topology(),
      
      nodes(),
      edges()
    {}

    model::model(nlohmann::json config, bool verbose):
      parameters(config, verbose),
      topology(),
      
      nodes(),
      edges()
    {}    
        
    model::model(parameters_type& params):
      parameters(params),
      topology(),
      
      nodes(),
      edges()
    {}    

    model::~model()
    {}
    
    bool model::configure(nlohmann::json& config, bool verbose)
    {      
      return parameters.from_json(config, verbose);
    }

    bool model::initialise()
    {
      nodes.initialise();
      edges.initialise();
      
      return true;
    }

    bool model::initialise(index_type reserved_nodes,
			   index_type reserved_edges)
    {
      nodes.initialise();
      edges.initialise();

      nodes.reserve(reserved_nodes);
      edges.reserve(reserved_edges);

      return true;
    }
    
    bool model::finalise(bool compute_topo)
    {
      nodes.sort();
      edges.sort();

      if(compute_topo)
	{
	  topology.compute(*this);
	}
      
      return true;
    }
    
  }

}

#endif
