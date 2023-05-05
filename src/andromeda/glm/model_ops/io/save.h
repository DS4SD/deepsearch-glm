//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODELOPS_IO_SAVE_H_
#define ANDROMEDA_MODELS_GLM_MODELOPS_IO_SAVE_H_

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class model_op<SAVE, model_type>: public io_base
    {
      //typedef typename model_type::hash_type hash_type;
      //typedef typename model_type::ind_type ind_type;

    public:

      model_op(std::shared_ptr<model_type> model);
      ~model_op();

      static nlohmann::json to_config();

      bool from_config(nlohmann::json& config);

      bool save(std::filesystem::path path);

    private:

      bool to_csv(std::filesystem::path path);

      bool to_json(std::filesystem::path path);

    private:

      bool save_to_json, save_to_csv;
      bool save_resolved_text;

      std::filesystem::path model_path;
      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    model_op<SAVE, model_type>::model_op(std::shared_ptr<model_type> model):
      io_base(),

      save_to_json(false),
      save_to_csv(false),

      save_resolved_text(true),

      model_path(),
      model(model)
    {}

    template<typename model_type>
    model_op<SAVE, model_type>::~model_op()
    {}

    template<typename model_type>
    nlohmann::json model_op<SAVE, model_type>::to_config()
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

    template<typename model_type>
    bool model_op<SAVE, model_type>::from_config(nlohmann::json& config)
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

          result = this->save(model_path);
        }
      else
        {
          LOG_S(ERROR) << "`io.save.root` key does not exist in config: \n"
                       << config.dump(2);
        }

      return result;
    }

    template<typename model_type>
    bool model_op<SAVE, model_type>::save(std::filesystem::path path)
    {
      LOG_S(INFO) << "writing started ...";

      if(not io_base::create_paths(path))
        {
          return false;
        }

      {
        auto& param = model->get_parameters();

        LOG_S(INFO) << "writing " << param_file.string();
        std::ofstream ofs(param_file.c_str());

        nlohmann::json data = param.to_json();
        ofs << std::setw(2) << data;
      }

      {
        auto& topology = model->get_topology();

        LOG_S(INFO) << "writing " << topo_file.string();
        std::ofstream ofs_a(topo_file.c_str());

        nlohmann::json data = topology.to_json();
        ofs_a << std::setw(2) << data;

        LOG_S(INFO) << "writing " << topo_file_text.string();

        std::ofstream ofs_b(topo_file_text.c_str());
        topology.to_txt(ofs_b);
      }

      {
        auto& nodes = model->get_nodes();

        LOG_S(INFO) << "writing " << nodes_file.string();
        std::ofstream ofs(nodes_file.c_str(), std::ios::binary);

        std::size_t tot=nodes.size();
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

        assert(cnt==tot);
      }

      {
        auto& edges = model->get_edges();

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

      if(save_to_json)
        {
          to_json(path);
        }

      if(save_to_csv)
        {
          to_csv(path);
        }

      return true;
    }

    template<typename model_type>
    bool model_op<SAVE, model_type>::to_json(std::filesystem::path path)
    {
      LOG_S(INFO) << "writing JSON started ...";

      if(not io_base::create_paths(path))
        {
          return false;
        }

      {
        auto& nodes = model->get_nodes();

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
    bool model_op<SAVE, model_type>::to_csv(std::filesystem::path path)
    {
      LOG_S(INFO) << "writing CSV started ...";

      if(not io_base::create_paths(path))
        {
          return false;
        }

      //ind_type ind=0;
      //std::unordered_map<hash_type, ind_type> to_index={};

      {
        auto& nodes = model->get_nodes();
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
	
	/*
        ofs << "hash" << ","
            << "flavor" << ","
            << "word-count" << ","
            << "sent-count" << ","
            << "text-count" << ","
          //<< "tags" << ","
            << "text" << "\n";

        for(auto itr=nodes.begin(); itr!=nodes.end(); itr++)
          {
            if(itr->first==node_names::TOKEN or
               itr->first==node_names::CONT or
               itr->first==node_names::CONN or
               itr->first==node_names::VERB or
               itr->first==node_names::TERM)
              {
                for(auto& node:itr->second)
                  {
                    std::string text = node.get_text(nodes);

                    if(text.find("\"")!=std::string::npos)
                      {
                        text = utils::replace(text, "\"", "'");
                      }

                    to_index[node.get_hash()] = ind++;

                    ofs << node.get_hash() << ","
                        << node_names::to_name.at(node.get_flvr()) << ","
                        << node.get_word_cnt() << ","
                        << node.get_sent_cnt() << ","
                        << node.get_text_cnt() << ","
                      //<< "\"" << utils::to_string(itr->tags) << "\","
                        << "\"" << text << "\"\n";
                  }
              }
          }
	*/
      }

      {
        auto& edges = model->get_edges();
        LOG_S(INFO) << "writing " << edges_file_csv.string();

        std::ofstream ofs(edges_file_csv.c_str());

        ofs << "name" << ","
            << "flavor" << ","
            << "hash_i" << ","
            << "hash_j" << ","
	  //<< "ind_i" << ","
	  //<< "ind_j" << ","
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
                    << edge_names::to_string(edge.get_flvr()) << ","
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
