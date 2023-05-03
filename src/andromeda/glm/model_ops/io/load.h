//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODELOPS_IO_LOAD_H_
#define ANDROMEDA_MODELS_GLM_MODELOPS_IO_LOAD_H_

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class model_op<LOAD, model_type>: public io_base
    {
      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef typename nodes_type::node_type node_type;
      typedef typename edges_type::edge_type edge_type;

    public:

      model_op(std::shared_ptr<model_type> model);
      ~model_op();

      static nlohmann::json to_config();

      bool from_config(nlohmann::json& config);

      bool load(std::filesystem::path path);

    private:

      std::filesystem::path model_path;
      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    model_op<LOAD, model_type>::model_op(std::shared_ptr<model_type> model):
      model_path(),
      model(model)
    {}

    template<typename model_type>
    model_op<LOAD, model_type>::~model_op()
    {}

    template<typename model_type>
    nlohmann::json model_op<LOAD, model_type>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      {
        auto&   io = config[io_base::io_lbl];
        auto& load = io[io_base::load_lbl];

        load[io_base::root_lbl] = "<path-to-root-dir>";
      }

      return config;
    }

    template<typename model_type>
    bool model_op<LOAD, model_type>::from_config(nlohmann::json& config)
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

          result = this->load(model_path);
        }
      else
        {
          LOG_S(ERROR) << "`io.load.root` key does not exist in config: \n"
                       << config.dump(2);
        }

      return result;
    }

    template<typename model_type>
    bool model_op<LOAD, model_type>::load(std::filesystem::path path)
    {
      LOG_S(INFO) << "reading started ...";

      if(not io_base::set_paths(path))
        {
          return false;
        }

      {
        auto& param = model->get_parameters();
        param.clear();

        LOG_S(INFO) << "reading " << param_file.string();
        std::ifstream ifs(param_file.c_str());

        nlohmann::json data;
        ifs >> data;

        param.from_json(data, true);
      }

      {
        auto& topology = model->get_topology();
        topology.clear();

        LOG_S(INFO) << "reading " << topo_file.string();
        std::ifstream ifs(topo_file.c_str());

        nlohmann::json data;
        ifs >> data;

        topology.from_json(data);
      }

      {
        auto& nodes = model->get_nodes();
        nodes.initialise();

        LOG_S(INFO) << "reading " << nodes_file.string();
        std::ifstream ifs(nodes_file.c_str(), std::ios::binary);

        std::size_t N=0;
        ifs.read((char*)&N, sizeof(N));

	std::size_t D = N/1000;
	
        LOG_S(INFO) << "  #-nodes: " << N << " => start reading ...";

        for(std::size_t i=0; i<N; i++)
          {
	    if(((i%D)==0) or (i+1)==N)
	      {
		std::cout << "\r completion: " << std::fixed
			  << 100*double(i+1)/double(N) << std::flush;
	      }
	    
            base_node node;
            ifs >> node;
	    
            nodes.push_back(node);
	    
            /*
              nlohmann::json data_ = node.to_json(nodes);
              LOG_S(INFO) << "data_: " << data_.dump(2);
            */
          }
	std::cout << "\n";

	
	/*
        for(auto flvr_itr=nodes.begin(); flvr_itr!=nodes.end(); flvr_itr++)
          {
            for(const auto& node:flvr_itr->second)
	      {
		LOG_S(INFO) << (flvr_itr->first) << ": " << node.get_hash();
		LOG_S(INFO) << " => " << node.get_text(nodes, true);
	      }
	  }
	*/
      }

      {
        auto& edges = model->get_edges();
        edges.clear();

        LOG_S(INFO) << "reading " << edges_file.string();
        std::ifstream ifs(edges_file.c_str(), std::ios::binary);

        std::size_t N=0;
        ifs.read((char*)&N, sizeof(N));

        LOG_S(INFO) << "  #-edges: " << N;
	std::size_t D = N/1000;
	//std::size_t M = N*1.1;
	//LOG_S(INFO) << "  reserving #-edges: " << M;	
        //edges.reserve(M);

	LOG_S(INFO) << "  reading edges:";
        for(std::size_t i=0; i<N; i++)
          {
	    if(((i%D)==0) or (i+1)==N)
	      {
		std::cout << "\r completion: " << std::fixed << 100*double(i+1)/double(N) << std::flush;
	      }
	    
	    base_edge edge;
            ifs >> edge;

	    edges.push_back(edge);
          }
	std::cout << "\n";

	edges.set_sorted();
      }

     
      {
        LOG_S(INFO) << "reading done!";

        auto& topology = model->get_topology();
        topology.to_shell();
      }

      return true;
    }

  }

}

#endif
