//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOW_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOW_H_

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class query_flow
    {
    public:

      const static inline std::string name_lbl = "flow-name";
      const static inline std::string flow_lbl = "flow";

      const static inline std::string overview_lbl = "overview";
      const static inline std::string result_lbl = "result";

      typedef typename model_type::hash_type hash_type;
      typedef typename model_type::flvr_type flvr_type;

      typedef typename model_type::cnt_type cnt_type;
      typedef typename model_type::ind_type ind_type;
      typedef typename model_type::val_type val_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef typename nodes_type::node_type glm_node_type;
      typedef typename edges_type::edge_type glm_edge_type;

      typedef query_node qry_node_type;
      typedef query_edge qry_edge_type;

      typedef query_baseop                 flowop_type;
      typedef std::shared_ptr<flowop_type> flowop_ptr_type;

      typedef typename std::vector<flowop_ptr_type>::iterator itr_type;

      typedef typename flowop_type::flow_id_type  flow_id_type;
      typedef typename flowop_type::flow_res_type flow_res_type;

      typedef std::unordered_map<flow_id_type, std::shared_ptr<flow_res_type> > nodesets_type;

    public:

      query_flow(std::shared_ptr<model_type> model);

      nlohmann::json to_json();

      void show();
      bool done();

      double time() { return delta_t.count(); }

      itr_type begin() { return ops.begin(); }
      itr_type end() { return ops.end(); }

      std::size_t size() { return ops.size(); }
      std::shared_ptr<flowop_type> back() { return (ops.size()==0? NULL:ops.back()); }

      bool from_config(const nlohmann::json& config);
      nlohmann::json to_config();
      
      bool execute(const nlohmann::json& config);
      
      void clear();
      void push_back(std::shared_ptr<flowop_type> op);

      bool validate(std::string& error);
      bool execute();

      std::shared_ptr<flowop_type> add_select(std::string& word); // search single word
      std::shared_ptr<flowop_type> add_select(std::vector<std::string>& words); // search word set
      std::shared_ptr<flowop_type> add_select(std::vector<std::vector<std::string> >& paths); // search path set

      std::shared_ptr<flowop_type> add_select(glm_node_type& node);
      //std::shared_ptr<flowop_type> add_select(glm_path_type& path);

      std::shared_ptr<flowop_type> add_select(std::vector<glm_node_type>& nodes);
      std::shared_ptr<flowop_type> add_select(std::vector<qry_node_type>& nodes);

      std::shared_ptr<flowop_type> add_filter(std::set<flvr_type> flavors);
      std::shared_ptr<flowop_type> add_filter(std::set<flvr_type> flavors,
					      std::set<flow_id_type> flow_ids);

      std::shared_ptr<flowop_type> add_traverse(short edge_flavor);
      std::shared_ptr<flowop_type> add_traverse(short edge_flavor, flow_id_type flow_id);

      std::shared_ptr<flowop_type> add_join(std::set<flow_id_type> flow_ids);

      std::shared_ptr<flowop_type> add_intersect(std::set<flow_id_type> flow_ids);

      std::shared_ptr<flowop_type> set_uniform_weight(flow_id_type pos_id);

      std::shared_ptr<flowop_type> add_neg_filter(flow_id_type source_id,
                                                  flow_id_type filter_id);

      std::shared_ptr<flowop_type> add_subgraph(std::set<flow_id_type> sources,
						bool dynamic_expansion,
						std::set<flvr_type> edge_flvrs);
      
    private:

      void clear_flow();

      void execute_flow();

      bool execute_flow(std::shared_ptr<flowop_type> op);

    private:

      std::shared_ptr<model_type> model;

      std::chrono::time_point<std::chrono::system_clock> t0, t1;
      std::chrono::duration<double, std::milli> delta_t;

      std::unordered_map<std::size_t, std::size_t> opid_to_index;
      std::vector<std::shared_ptr<flowop_type> > ops;

      nodesets_type nodesets;
    };

    template<typename model_type>
    query_flow<model_type>::query_flow(std::shared_ptr<model_type> model):
      model(model),
      t0(std::chrono::system_clock::now()),
      t1(std::chrono::system_clock::now()),
      delta_t(t1-t0),
      opid_to_index({}),
      ops({})
    {}

    template<typename model_type>
    nlohmann::json query_flow<model_type>::to_json()
    {
      nlohmann::json result = nlohmann::json::object({});

      {
	auto& overview = result[overview_lbl];
	overview["time"] = delta_t.count();

	const std::vector<std::string> headers
	  = { "flid", "flop", "done", "name", "time [msec]",
	      "#-nodes", "#-edges",
	      "prob-avg", "prob-std", "prob-ent"};

	overview["headers"] = headers;
	overview["data"] = nlohmann::json::array({});

	for(auto& op:ops)
	  {
	    auto result = op->get_nodeset();
	    
	    nlohmann::json row = nlohmann::json::array({});
	    {
	      row.push_back(op->get_flid());
	      row.push_back(to_string(op->get_flop()));
	      row.push_back(op->is_done());
	      row.push_back(result->get_name());	      
	      row.push_back(std::to_string(op->get_time()));

	      row.push_back(result->get_num_nodes());
	      row.push_back(result->get_num_edges());

	      row.push_back(result->get_prob_avg());
	      row.push_back(result->get_prob_std());
	      row.push_back(result->get_prob_ent());
	    }
	    assert(row.size()==headers.size());
	    overview["data"].push_back(row);
	  }
      }

      {
	auto& flow = result[flow_lbl];
	flow = nlohmann::json::array({});

	for(auto& op:ops)
	  {
	    flow.push_back(op->to_config());
	  }
      }
      
      {
	auto& output = result[result_lbl];
	output = nlohmann::json::array({});
	
	for(auto& op:ops)
	  {
	    if(op->is_done() and op->get_num_nodes()>0)
	      {
		auto nodeset = op->get_nodeset();

		cnt_type num_nodes = op->get_num_nodes();
		cnt_type num_edges = op->get_num_edges();
		cnt_type ind_nodes = op->get_ind_nodes();
		cnt_type ind_edges = op->get_ind_edges();
		
		output.push_back(nodeset->to_json(num_nodes, num_edges,
						  ind_nodes, ind_edges));
	      }
	    else
	      {
		output.push_back(nlohmann::json::value_t::null);
	      }
	  }
      }

      return result;
    }

    template<typename model_type>
    bool query_flow<model_type>::from_config(const nlohmann::json& config)
    {
      //LOG_S(INFO) << "config: " << config.dump(2);
      
      if(config.count(flow_lbl)==0 or
	 (not config[flow_lbl].is_array()))
	{
	  LOG_S(WARNING) << "config does not contain `flow` of type array";
	  return false;	  
	}

      this->clear();
      
      const nlohmann::json& flow = config[flow_lbl];
      for(std::size_t l=0; l<flow.size(); l++)
        {
          auto op = to_flowop(flow.at(l), model);

	  if(op!=NULL)
	    {
	      this->push_back(op);
	    }
	  else	    
	    {
	      LOG_S(WARNING) << "incorrect flow-op config: "
			     << flow.at(l).dump(2);

	      this->clear();
	      return false;
	    }
	}

      return true;
    }

    template<typename model_type>
    nlohmann::json query_flow<model_type>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      {
	config[name_lbl] = "<optional:name>";
	config[flow_lbl] = nlohmann::json::array({});
      }

      {
        for(auto& op:ops)
          {
            config[flow_lbl].push_back(op->to_config());
          }
      }

      return config;
    }

    template<typename model_type>
    void query_flow<model_type>::show()
    {
      std::vector<std::string> header
        = { "id", "done", "name", "time [msec]", "#-nodes",
            "prob-avg", "prob-std", "prob-ent"};

      std::vector<std::vector<std::string> > data={};
      for(auto& op:ops)
        {
          auto result = op->get_nodeset();

          std::vector<std::string> row
            = { std::to_string(op->get_flid()),
                (op->is_done()? "true":"false"),
                result->get_name(), std::to_string(op->get_time()),
                std::to_string(result->size()),
                std::to_string(result->get_prob_avg()),
                std::to_string(result->get_prob_std()),
                std::to_string(result->get_prob_ent())
          };

          data.push_back(row);
        }
      
      std::stringstream capt;
      capt << "TTS [msec]: " << delta_t.count();
      
      LOG_S(INFO) << utils::to_string(capt.str(), header, data);
    }

    template<typename model_type>
    bool query_flow<model_type>::done()
    {
      bool success=true;
      for(auto& op:ops)
        {
          if(op->is_done()==false)
            {
              success=false;
            }
        }

      return success;
    }

    template<typename model_type>
    void query_flow<model_type>::clear()
    {
      opid_to_index.clear();
      ops.clear();
    }

    template<typename model_type>
    void query_flow<model_type>::push_back(std::shared_ptr<flowop_type> op)
    {
      opid_to_index[op->get_flid()] = ops.size();
      ops.push_back(op);
    }

    template<typename model_type>
    bool query_flow<model_type>::validate(std::string& error)
    {
      for(auto& op:ops)
        {
          if(opid_to_index.count(op->get_flid())==0)
            {
	      //LOG_S(WARNING) << "could not find flid: " << op->get_flid();
	      std::stringstream ss;
	      ss << "could not find flid: " << op->get_flid();
	      
	      error = ss.str();
              return false;
            }

          for(auto& dep:op->get_dependencies())
            {
              if(opid_to_index.count(dep)==0 or
                 opid_to_index[dep]>=ops.size())
                {
		  std::stringstream ss;
		  ss<< "could not find the dependency " << dep
		    << " of flid: " << op->get_flid();

		  error = ss.str();
                  return false;
                }
            }
        }

      error="";
      return true;
    }

    template<typename model_type>
    bool query_flow<model_type>::execute(const nlohmann::json& config)
    {
      if(from_config(config))
	{
	  return execute();
	}

      return false;
    }
    
    template<typename model_type>
    bool query_flow<model_type>::execute()
    {
      t0 = std::chrono::system_clock::now();

      std::string error="";
      if(not validate(error))
        {
	  LOG_S(WARNING) << error;
          return false;
        }

      clear_flow();

      execute_flow();

      t1 = std::chrono::system_clock::now();
      delta_t = t1-t0;

      return this->done();
    }

    template<typename model_type>
    void query_flow<model_type>::clear_flow()
    {
      nodesets.clear();
      for(auto& op:ops)
        {
          op->get_nodeset()->clear();
          nodesets.emplace(op->get_flid(), op->get_nodeset());
        }
    }

    template<typename model_type>
    void query_flow<model_type>::execute_flow()
    {
      int itr=0, max_itr=32;//, cnt=0;

      while(itr++<max_itr)
        {
          bool done = true;
	  
	  //LOG_S(INFO) << "itr: " << itr;	  
	  //cnt=0;
          for(auto& op:ops)
            {
              if(execute_flow(op))
                {
                  done = false;
                }

	      //LOG_S(INFO) << "\t" << cnt++ << ": " << op->is_done();
            }

          if(done)
            {
              break;
            }
        }

      if(itr==max_itr)
        {
          LOG_S(WARNING) << "exceeded max iterations in `execute_flow`";
        }
      else if(not done())
	{
          LOG_S(WARNING) << "could not finish executing the flow ...";

	  int cnt=0;
	  for(auto& op:ops)
	    {
	      LOG_S(INFO) << "\t" << cnt++ << ": " << op->is_done();
	    }
	}
      else
	{
	  //LOG_S(INFO) << "executed flow!";	  
	}
    }

    template<typename model_type>
    bool query_flow<model_type>::execute_flow(std::shared_ptr<flowop_type> op)
    {
      if(op->is_done())
        {
          return false;
        }

      for(auto dep_id:op->get_dependencies())
        {
          std::size_t dep_ind = opid_to_index.at(dep_id);

          if(not ops.at(dep_ind)->is_done())
            {
              return false;
            }
        }

      op->set_t0();

      bool done = op->execute(nodesets);

      op->set_t1();

      return done;
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_select(std::string& word)
    {
      std::vector<std::vector<std::string> > nodes={{word}};
      return this->add_select(nodes);
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_select(std::vector<std::string>& words)
    {
      std::vector<std::vector<std::string> > nodes={words};
      return this->add_select(nodes);
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_select(std::vector<std::vector<std::string> >& paths)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<SELECT> >(cid, model, paths);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_select(glm_node_type& node)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<SELECT> >(cid, model, node);
      this->push_back(op);

      return this->back();
    }

    /*
    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_select(glm_path_type& path)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<SELECT> >(cid, model, path);
      this->push_back(op);

      return this->back();
    }
    */
    
    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_select(std::vector<glm_node_type>& nodes)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<SELECT> >(cid, model, nodes);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_select(std::vector<qry_node_type>& nodes)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<SELECT> >(cid, model, nodes);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_filter(std::set<flvr_type> flavors)
    {
      flow_id_type sid = (this->back())->get_flid();
      return add_filter(flavors, {sid});
    }
    
    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_filter(std::set<flvr_type> flavors,
								     std::set<flow_id_type> flow_ids)
    {

      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<FILTER> >(cid, model, flow_ids, flavors);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_traverse(short edge_flavor)
    {
      flow_id_type sid = (this->back())->get_id();

      return add_traverse(edge_flavor, sid);
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_traverse(short edge_flavor,
                                                                       flow_id_type flow_id)
    {
      flow_id_type           flid = this->size();
      std::set<flow_id_type> deps = {flow_id};
      
      //auto op = std::make_shared<query_flowop<TRAVERSE> >(cid, model, edge_flavor, flow_id);
      auto op = std::make_shared<query_flowop<TRAVERSE> >(model, flid, deps, edge_flavor);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_join(std::set<flow_id_type> flow_ids)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<JOIN> >(cid, model, flow_ids);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_intersect(std::set<flow_id_type> flow_ids)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<INTERSECT> >(cid, model, flow_ids);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::set_uniform_weight(flow_id_type pos_id)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<UNIFORM> >(cid, model, pos_id);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_neg_filter(flow_id_type source_id,
                                                                         flow_id_type filter_id)
    {
      flow_id_type cid = this->size();

      auto op = std::make_shared<query_flowop<FILTER> >(cid, model, false, source_id, filter_id);
      this->push_back(op);

      return this->back();
    }

    template<typename model_type>
    std::shared_ptr<query_baseop> query_flow<model_type>::add_subgraph(std::set<flow_id_type> sources,
								       bool dynamic_expansion,
								       std::set<flvr_type> edge_flvrs)
    {
      flow_id_type cid = this->size();

      std::set<flow_id_type> deps = sources;
      
      auto op = std::make_shared<query_flowop<SUBGRAPH> >(cid, model, deps, dynamic_expansion,  edge_flvrs);
      this->push_back(op);

      return this->back();
    }
    
  }

}

#endif
