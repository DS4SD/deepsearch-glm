//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODELOPS_IO_LOAD_H_
#define ANDROMEDA_MODELS_GLM_MODELOPS_IO_LOAD_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class model_op<LOAD>: public io_base
    {
      typedef andromeda::glm::model model_type;
      
      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef typename nodes_type::node_type node_type;
      typedef typename edges_type::edge_type edge_type;

    public:

      model_op();
      ~model_op();

      static nlohmann::json to_config();

      bool from_config(nlohmann::json& config);

      void set_incremental(bool incr);

      bool load(std::shared_ptr<model_type> model_ptr);
      
      bool load(std::filesystem::path path,
		std::shared_ptr<model_type> model_ptr);

    private:

      std::filesystem::path model_path;

      bool read_nodes_incremental;
      bool read_edges_incremental;
    };

    model_op<LOAD>::model_op():
      model_path(),

      read_nodes_incremental(false),
      read_edges_incremental(false)    
    {}

    model_op<LOAD>::~model_op()
    {}

    void model_op<LOAD>::set_incremental(bool incr)
    {
      read_nodes_incremental = incr;
      read_edges_incremental = incr;
    }
    
    nlohmann::json model_op<LOAD>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      {
        auto&   io = config[io_base::io_lbl];
        auto& load = io[io_base::load_lbl];

        load[io_base::root_lbl] = "<path-to-root-dir>";
      }

      return config;
    }

    bool model_op<LOAD>::from_config(nlohmann::json& config)
    {
      bool result=false;

      if(config.count(io_base::io_lbl) and
         config[io_base::io_lbl].count(io_base::load_lbl))
        {
          auto& io = config[io_base::io_lbl];
          auto& load = io[io_base::load_lbl];

          std::string root="./glm-model-default";
          root = load.value(io_base::root_lbl, root);

          model_path = root;
          if(not std::filesystem::exists(model_path))
            {
              LOG_S(ERROR) << "path to model does not exists: " << model_path;
              return false;
            }
          //result = this->load(model_path);
        }
      else
        {
          LOG_S(ERROR) << "`io.load.root` key does not exist in config: \n"
                       << config.dump(2);
        }

      return result;
    }


    bool model_op<LOAD>::load(std::shared_ptr<model_type> model_ptr)
    {
      return this->load(model_path, model_ptr);
    }
    
    bool model_op<LOAD>::load(std::filesystem::path path, std::shared_ptr<model_type> model_ptr)
    {
      LOG_S(INFO) << "reading started ...";

      if(not io_base::set_paths(path))
        {
          return false;
        }

      {
        auto& param = model_ptr->get_parameters();
        param.clear();

        LOG_S(INFO) << "reading " << param_file.string();
        std::ifstream ifs(param_file.c_str());

        nlohmann::json data;
        ifs >> data;

        param.from_json(data, true);
      }

      {
        auto& topology = model_ptr->get_topology();
        topology.clear();

        LOG_S(INFO) << "reading " << topo_file.string();
        std::ifstream ifs(topo_file.c_str());

        nlohmann::json data;
        ifs >> data;

        topology.from_json(data);
      }

      {
        auto& nodes = model_ptr->get_nodes();

	if(not read_nodes_incremental)
	  {
	    nodes.clear();
	  }
	
        LOG_S(INFO) << "reading " << nodes_file.string();
        std::ifstream ifs(nodes_file.c_str(), std::ios::binary);
	
        std::size_t N=0;
        ifs.read((char*)&N, sizeof(N));
	
        LOG_S(INFO) << "#-nodes: " << N << " => start reading ...";
        for(std::size_t i=0; i<N; i++)
          {
            base_node node;
            ifs >> node;

	    if(read_nodes_incremental)
	      {
		nodes.insert(node, false);		
	      }
	    else
	      {
		nodes.push_back(node);
	      }
	  }
      }

      {
        auto& edges = model_ptr->get_edges();
        edges.clear();

        LOG_S(INFO) << "reading " << edges_file.string();
        std::ifstream ifs(edges_file.c_str(), std::ios::binary);

	// number of flavors
        std::size_t M=0;
        ifs.read((char*)&M, sizeof(M));

	LOG_S(INFO) << "#-flavor-edges: " << M;
	
	std::map<flvr_type, std::pair<std::size_t, bool> > overview;
        for(std::size_t i=0; i<M; i++)
	  {
	    flvr_type flvr;
	    std::size_t K;
	    bool sorted;

	    ifs.read((char*)&flvr, sizeof(flvr));
	    ifs.read((char*)&K, sizeof(K));
	    ifs.read((char*)&sorted, sizeof(sorted));

	    overview[flvr] = std::pair<std::size_t, bool>(K,sorted);
	  }
	
        std::size_t N=0;	
        ifs.read((char*)&N, sizeof(N));
	
        LOG_S(INFO) << "#-edges: " << N << " => start reading ...";
        for(std::size_t i=0; i<N; i++)
          {
	    base_edge edge;
            ifs >> edge;

	    if(read_edges_incremental)
	      {
		edges.insert(edge, false);		
	      }
	    else
	      {
		edges.push_back(edge, read_edges_incremental);
	      }	    
          }
	
	for(auto itr=overview.begin(); itr!=overview.end(); itr++)
	  {
	    LOG_S(INFO) << "edge-flvr: " << itr->first << " -> "
			<< (itr->second).first << ";" << (itr->second).second;
	    edges.set_sorted(itr->first, (itr->second).second);
	  }
      }
      
      {
        LOG_S(INFO) << "reading done!";

        auto& topology = model_ptr->get_topology();
        topology.to_shell();
      }

      return true;
    }

  }

}

#endif
