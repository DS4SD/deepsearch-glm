//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODELOPS_IO_SAVE_H_
#define ANDROMEDA_MODELS_GLM_MODELOPS_IO_SAVE_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class model_op<SAVE>: public io_base
    {
    public:

      model_op();
      ~model_op();

      static nlohmann::json to_config();

      bool from_config(nlohmann::json& config);

      template<typename model_type>
      bool save(std::shared_ptr<model_type> model_ptr);

      template<typename model_type>
      bool save(std::filesystem::path path,
		std::shared_ptr<model_type> model_ptr);
      
    private:

      template<typename model_type>
      bool to_bin(std::filesystem::path path,
		  std::shared_ptr<model_type> model_ptr);

      template<typename model_type>
      bool to_csv(std::filesystem::path path,
		  std::shared_ptr<model_type> model_ptr);

      template<typename model_type>
      bool to_json(std::filesystem::path path,
		   std::shared_ptr<model_type> model_ptr);

    private:

      bool save_to_json, save_to_csv;
      bool save_resolved_text;

      std::filesystem::path model_path;
    };

    model_op<SAVE>::model_op():
      io_base(),

      save_to_json(false),
      save_to_csv(false),

      save_resolved_text(true),

      model_path()
    {}

    model_op<SAVE>::~model_op()
    {}

    nlohmann::json model_op<SAVE>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      {
        auto&   io = config[io_base::io_lbl];
        auto& save = io[io_base::save_lbl];

        save[io_base::root_lbl] = "<path-to-root-dir>";

        save[io_base::write_json_lbl] = false;
        save[io_base::write_csv_lbl] = false;

        save[io_base::save_rtext_lbl] = false;
      }

      return config;
    }

    bool model_op<SAVE>::from_config(nlohmann::json& config)
    {
      bool result=false;

      if(config.count(io_base::io_lbl) and
         config[io_base::io_lbl].count(io_base::save_lbl))
        {
          auto& io = config[io_base::io_lbl];
          auto& save = io[io_base::save_lbl];

          std::string root="./glm-model-default";
          root = save.value(io_base::root_lbl, root);

          model_path = root;
          if(not std::filesystem::exists(model_path))
            {
              LOG_S(ERROR) << "path to model does not exists: " << model_path;
            }

          save_to_json = save.value(io_base::write_json_lbl, save_to_json);
          save_to_csv = save.value(io_base::write_csv_lbl, save_to_csv);

          save_resolved_text = save.value(io_base::save_rtext_lbl, save_resolved_text);
        }
      else
        {
          LOG_S(ERROR) << "`io.save.root` key does not exist in config: \n"
                       << config.dump(2);
        }

      return result;
    }

    template<typename model_type>
    bool model_op<SAVE>::save(std::shared_ptr<model_type> model_ptr)
    {
      bool result = this->to_bin(model_path, model_ptr);

      if(save_to_json)
        {
          to_json(model_path, model_ptr);
        }

      if(save_to_csv)
        {
          to_csv(model_path, model_ptr);
        }

      return result;
    }

    template<typename model_type>
    bool model_op<SAVE>::save(std::filesystem::path path,
			      std::shared_ptr<model_type> model_ptr)
    {
      return this->to_bin(model_path, model_ptr);
    }
    
    template<typename model_type>
    bool model_op<SAVE>::to_bin(std::filesystem::path path,
				std::shared_ptr<model_type> model_ptr)
    {
      LOG_S(INFO) << "writing started ...";

      if(not io_base::create_paths(path))
        {
          return false;
        }

      {
        auto& param = model_ptr->get_parameters();

        LOG_S(INFO) << "writing " << param_file.string();
        std::ofstream ofs(param_file.c_str());

        nlohmann::json data = param.to_json();
        ofs << std::setw(2) << data;
      }

      {
        auto& topology = model_ptr->get_topology();

        LOG_S(INFO) << "writing " << topo_file.string();
        std::ofstream ofs_a(topo_file.c_str());

        nlohmann::json data = topology.to_json();
        ofs_a << std::setw(2) << data;

        LOG_S(INFO) << "writing " << topo_file_text.string();

        std::ofstream ofs_b(topo_file_text.c_str());
        topology.to_txt(ofs_b);
      }

      {
        auto& nodes = model_ptr->get_nodes();

	nodes.sort();
	
        LOG_S(INFO) << "writing " << nodes_file.string();
        std::ofstream ofs(nodes_file.c_str(), std::ios::binary);

        std::size_t tot = nodes.size();
        ofs.write((char*)&tot, sizeof(tot));

        std::size_t cnt=0;
        for(auto flvr_itr=nodes.begin(); flvr_itr!=nodes.end(); flvr_itr++)
          {
            for(const auto& node:flvr_itr->second)
              {
                ofs << node;
                cnt += 1;
              }
          }

	if(cnt!=tot)
	  {
	    LOG_S(ERROR) << "node-size: " << tot << " versus counted size: " << cnt;
	  }	
        assert(cnt==tot);
      }

      {
        auto& edges = model_ptr->get_edges();

        LOG_S(INFO) << "writing " << edges_file.string();
        std::ofstream ofs(edges_file.c_str(), std::ios::binary);

        std::size_t M=edges.number_of_flavors();
        ofs.write((char*)&M, sizeof(M));
	
        for(auto flvr_itr=edges.begin(); flvr_itr!=edges.end(); flvr_itr++)
          {
	    flvr_type flvr = flvr_itr->first;
	    std::size_t K = (flvr_itr->second).size();
	    bool sorted = edges.is_sorted(flvr);

	    ofs.write((char*)&flvr, sizeof(flvr));
	    ofs.write((char*)&K, sizeof(K));
	    ofs.write((char*)&sorted, sizeof(sorted));	    
	  }

        std::size_t N=edges.size();
        ofs.write((char*)&N, sizeof(N));
	
        for(auto flvr_itr=edges.begin(); flvr_itr!=edges.end(); flvr_itr++)
          {
            auto& edge_coll = flvr_itr->second;
            for(const auto& edge:edge_coll)
              {
                ofs << edge;
              }
          }
      }

      return true;
    }

    template<typename model_type>
    bool model_op<SAVE>::to_json(std::filesystem::path path,
				 std::shared_ptr<model_type> model_ptr)
    {
      LOG_S(INFO) << "writing JSON started ...";

      if(not io_base::create_paths(path))
        {
          return false;
        }

      {
        auto& nodes = model_ptr->get_nodes();

        LOG_S(INFO) << "writing " << nodes_file_json.string();
        std::ofstream ofs(nodes_file_json.c_str());

        for(auto itr=nodes.begin(); itr!=nodes.end(); itr++)
          {
            for(auto& node:itr->second)
              {
                if(save_resolved_text)
                  {
                    nlohmann::json data = node.to_json(nodes);
                    ofs << data << "\n";
                  }
                else
                  {
                    nlohmann::json data = node.to_json();
                    ofs << data << "\n";
                  }
              }
          }
      }

      return true;
    }

    template<typename model_type>
    bool model_op<SAVE>::to_csv(std::filesystem::path path,
				std::shared_ptr<model_type> model_ptr)
    {
      LOG_S(INFO) << "writing CSV started ...";

      if(not io_base::create_paths(path))
        {
          return false;
        }

      {
        auto& nodes = model_ptr->get_nodes();
        LOG_S(INFO) << "writing " << nodes_file_csv.string();

        std::ofstream ofs(nodes_file_csv.c_str());

	std::size_t N = base_node::headers.size();
	for(std::size_t l=0; l<N; l++)
	  {
	    std::string conn = (l+1==N? "\n" : ",");	    
	    ofs << base_node::headers.at(l) << conn;	    
	  }

	for(auto itr=nodes.begin(); itr!=nodes.end(); itr++)
          {
	    for(auto& node:itr->second)
	      {
		if(utils::contains(node.get_text(), "\""))
		  {
		    LOG_S(WARNING) << "skip node with text '" << node.get_text() << "' for pandas compatibility";
		    continue;
		  }
		
		nlohmann::json row = node.to_row(nodes);
		assert(row.size()==N);
		
		for(std::size_t l=0; l<base_node::headers.size(); l++)
		  {
		    std::string conn = (l+1==N? "\n" : ",");	    
		    if(not row.at(l).is_null())
		      {
			ofs << row.at(l) << conn;
		      }
		  }
	      }
	  }
      }

      {
        auto& edges = model_ptr->get_edges();
        LOG_S(INFO) << "writing " << edges_file_csv.string();

        std::ofstream ofs(edges_file_csv.c_str());

        ofs << "flavor" << ","
	    << "name" << ","
            << "hash_i" << ","
            << "hash_j" << ","
            << "count" << ","
            << "prob" << ","
            << "hash" << "\n";

        for(auto flvr_itr=edges.begin(); flvr_itr!=edges.end(); flvr_itr++)
          {
            auto& edge_coll = flvr_itr->second;
            for(const auto& edge:edge_coll)
              {
                // skipped nodes and paths
                //if(to_index.find(edge.get_hash_i())==to_index.end() or
		//to_index.find(edge.get_hash_j())==to_index.end() )
		//{
		//continue;
		//}

                ofs << edge.get_flvr() << ","
                    << edge_names::to_name(edge.get_flvr()) << ","
                    << std::fixed
                    << edge.get_hash_i() << ","
                    << edge.get_hash_j() << ","
		  //<< to_index.at(edge.get_hash_i()) << ","
		  //<< to_index.at(edge.get_hash_j()) << ","
                    << edge.get_count() << ","
                    << std::scientific << std::setprecision(6)
                    << edge.get_prob() << ","
                    << std::fixed
                    << edge.get_hash() << "\n";
              }
          }

        LOG_S(INFO) << "writing done!";
      }
      return false;
    }

  }

}

#endif
