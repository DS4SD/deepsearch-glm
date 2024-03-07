//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_BASE_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_BASE_H_

namespace andromeda
{
  namespace glm
  {
    class query_baseop: public base_types
    {
    public:

      const static inline std::string flop_lbl = "flop";
      const static inline std::string flid_lbl = "flid";
      const static inline std::string deps_lbl = "deps";
      const static inline std::string name_lbl = "name";

      const static inline std::string parameters_lbl = "parameters";

      const static inline std::string output_lbl = "output";

      const static inline std::string ind_nodes_lbl = "ind-nodes";
      const static inline std::string ind_edges_lbl = "ind-edges";            
      const static inline std::string num_nodes_lbl = "num-nodes";
      const static inline std::string num_edges_lbl = "num-edges";      
      
      typedef model model_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef typename nodes_type::node_type glm_node_type;
      typedef typename edges_type::edge_type glm_edge_type;
      
      typedef query_node qry_node_type;

      typedef flowop_name flow_op_type;
      typedef std::size_t flow_id_type;

      typedef query_result<model_type> flow_res_type;

      typedef std::unordered_map<flow_id_type, std::shared_ptr<flow_res_type> > results_type;

    public:

      query_baseop(std::shared_ptr<model_type> model_ptr,
                   flow_op_type flop, flow_id_type flid,
		   std::set<flow_id_type> dependencies);

      virtual ~query_baseop() {};

      virtual nlohmann::json to_config();
      virtual bool from_config(const nlohmann::json& config);

      bool set_output_parameters(const nlohmann::json& config);
      
      bool is_done() { return done; }

      flow_op_type get_flop() { return flop; }
      flow_id_type get_flid() { return flid; }

      cnt_type get_num_nodes() { return num_nodes; }
      cnt_type get_num_edges() { return num_edges; }
      cnt_type get_ind_nodes() { return ind_nodes; }
      cnt_type get_ind_edges() { return ind_edges; }
      
      std::shared_ptr<model_type> get_model() { return model_ptr; }

      double get_time() { return delta_t.count(); }

      std::set<flow_id_type> get_dependencies() { return dependencies; }

      std::shared_ptr<flow_res_type> get_nodeset() { return nodeset; }

      virtual bool execute(results_type& results)=0;

      void set_t0();
      void set_t1();

    protected:

      bool done;

      std::shared_ptr<model_type> model_ptr;
      
      flow_op_type flop;
      flow_id_type flid;

      std::set<flow_id_type> dependencies;

      cnt_type num_nodes, num_edges, ind_nodes, ind_edges;
      
      std::shared_ptr<flow_res_type> nodeset;

      std::chrono::time_point<std::chrono::system_clock> t0, t1;
      std::chrono::duration<double, std::milli> delta_t;
    };

    query_baseop::query_baseop(std::shared_ptr<model_type> model_ptr,
                               flow_op_type flop, flow_id_type flid,
			       std::set<flow_id_type> dependencies):
      done(false),
      model_ptr(model_ptr),
      
      flop(flop),
      flid(flid),

      dependencies(dependencies),

      num_nodes(1e3),
      num_edges(1e3),
      ind_nodes(0),
      ind_edges(0),

      nodeset(std::make_shared<flow_res_type>(model_ptr)),
      
      t0(std::chrono::system_clock::now()),
      t1(std::chrono::system_clock::now()),
      delta_t(t1-t0)
    {
      //nodeset = std::make_shared<flow_res_type>(model);
    }

    nlohmann::json query_baseop::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      {
        config[flid_lbl] = flid;
	config[flop_lbl] = to_string(flop);
	
        config[deps_lbl] = dependencies;
        config[parameters_lbl] = nlohmann::json::object({});

	config[output_lbl] = nlohmann::json::object({});
	{
	  config[output_lbl][ind_nodes_lbl] = ind_nodes;
	  config[output_lbl][ind_edges_lbl] = ind_edges;

	  config[output_lbl][num_nodes_lbl] = num_nodes;
	  config[output_lbl][num_edges_lbl] = num_edges;	  
	}
      }

      return config;
    }

    bool query_baseop::from_config(const nlohmann::json& config)
    {
      //LOG_S(INFO) << "from-config: " << config.dump(2);
      
      try
	{
	  done = false;
	  
	  flop = to_flowop_name(config[flop_lbl].get<std::string>());
          flid = config[flid_lbl].get<flow_id_type>();
	  
	  dependencies = config.value(deps_lbl, dependencies);

	  if(config.count(output_lbl))
	    {
	      ind_nodes = config[output_lbl].value(ind_nodes_lbl, ind_nodes);
	      ind_edges = config[output_lbl].value(ind_edges_lbl, ind_edges);

	      num_nodes = config[output_lbl].value(num_nodes_lbl, num_nodes);
	      num_edges = config[output_lbl].value(num_edges_lbl, num_edges);	  	      
	    }
        }
      catch(std::exception& exc)
	{
	  LOG_S(ERROR) << exc.what();
	  return false;
	}
      
      return true;
    }

    bool query_baseop::set_output_parameters(const nlohmann::json& config)
    {
      if(config.count(output_lbl))
	{
	  ind_nodes = config[output_lbl].value(ind_nodes_lbl, ind_nodes);
	  ind_edges = config[output_lbl].value(ind_edges_lbl, ind_edges);
	  
	  num_nodes = config[output_lbl].value(num_nodes_lbl, num_nodes);
	  num_edges = config[output_lbl].value(num_edges_lbl, num_edges);	  	      

	  return true;
	}

      return false;
    }
    
    void query_baseop::set_t0()
    {
      t0 = std::chrono::system_clock::now();
    }

    void query_baseop::set_t1()
    {
      t1 = std::chrono::system_clock::now();
      delta_t = t1-t0;
    }

  }

}

#endif
