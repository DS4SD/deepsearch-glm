//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_FILTER_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_FILTER_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class query_flowop<FILTER>: public query_baseop
    {
      const static flowop_name NAME = FILTER;

      const static inline std::string mode_lbl    = "mode";

      const static inline std::string flavors_lbl = "node-flavors";
      const static inline std::string regexes_lbl = "node-regex";
      const static inline std::string contains_lbl = "contains-flid";

    public:

      query_flowop(std::shared_ptr<model_type> model,
                   flow_id_type id, std::set<flow_id_type> dependencies,
                   const nlohmann::json& config);

      query_flowop(flow_id_type id, std::shared_ptr<model_type> model,
                   std::set<flow_id_type> source_ids, std::set<flvr_type> flavors);

      virtual ~query_flowop();

      virtual nlohmann::json to_config();// { return nlohmann::json::object({}); }
      virtual bool from_config(const nlohmann::json& config);// { return false;}

      virtual bool execute(results_type& results);

    private:

      bool filter_by_node_flavor(results_type& results);

      bool filter_by_node_text(results_type& results);

      bool filter_by_flid(results_type& results);
      
    private:

      std::string mode;

      std::set<flvr_type> flavors;
      
      std::vector<std::string> regexes;
      std::vector<pcre2_expr> exprs;

      flow_id_type filter_flid;
    };

    query_flowop<FILTER>::query_flowop(std::shared_ptr<model_type> model,
                                       flow_id_type flid, std::set<flow_id_type> dependencies,
                                       const nlohmann::json& config):
      query_baseop(model, NAME, flid, dependencies)
    {
      if((not config.is_null()) and
         (not from_config(config)))
        {
          LOG_S(WARNING) << "implement query_flowop<" << to_string(NAME) << "> "
                         << "with config: " << config.dump(2);
        }
    }

    query_flowop<FILTER>::query_flowop(flow_id_type flid, std::shared_ptr<model_type> model,
                                       std::set<flow_id_type> deps,
                                       std::set<flvr_type> flavors):
      query_baseop(model, NAME, flid, deps),

      mode(flavors_lbl),
      flavors(flavors)      
    {}

    query_flowop<FILTER>::~query_flowop()
    {}

    nlohmann::json query_flowop<FILTER>::to_config()
    {
      nlohmann::json config = query_baseop::to_config();

      nlohmann::json& params = config.at(parameters_lbl);
      {
        if(mode==flavors_lbl)
          {
            params[flavors_lbl]=nlohmann::json::array({});
            for(auto flvr:flavors)
              {
                params[flavors_lbl].push_back(node_names::to_name(flvr));
              }
          }
        else if(mode==regexes_lbl)
          {
            params[regexes_lbl]=regexes;
          }
	else if(mode==contains_lbl)
          {
            params[contains_lbl]=0;
          }
        else
          {
            params[mode_lbl] = "<node-flavor;node-regex;node-labels>";
            params[flavors_lbl] = {"term", "verb"};
            params[regexes_lbl] = nlohmann::json::array({});
	    params[regexes_lbl].push_back(R"(<string-example:[A-Z][a-z\s]*datasets?$>)");
          }
      }

      return config;
    }

    bool query_flowop<FILTER>::from_config(const nlohmann::json& config)
    {
      query_baseop::set_output_parameters(config);
      
      nlohmann::json params = config;
      if(config.count(parameters_lbl))
        {
          params = config.at(parameters_lbl);
        }
      
      mode = "undef";
      if(params.count(flavors_lbl))
        {
          mode = flavors_lbl;

          std::set<std::string> flvrs={};
          flvrs = params.value(flavors_lbl, flvrs);

          flavors = node_names::to_flavor(flvrs);
        }
      else if(params.count(regexes_lbl))
        {
          mode = regexes_lbl;

          regexes = {};
          regexes = params.value(regexes_lbl, regexes);

          for(auto regex_item:regexes)
            {
              try
                {
		  exprs.emplace_back("filter", "", regex_item);
                }
              catch(std::exception& exc)
                {
                  LOG_S(WARNING) << exc.what();
                }
            }
	}
      else if(params.count(contains_lbl))
	{
	  mode = contains_lbl;
	  
	  filter_flid = -1;
	  filter_flid = params.value(contains_lbl, filter_flid);

	  query_baseop::dependencies.insert(filter_flid);
	}
      else
        {
	  LOG_S(ERROR) << "unrecognised filter mode: " << mode;
	  return false;
	}

      return true;
    }

    bool query_flowop<FILTER>::execute(results_type& results)
    {
      if(mode==flavors_lbl)
        {
          return filter_by_node_flavor(results);
        }
      else if(mode==regexes_lbl)
        {
          return filter_by_node_text(results);
        }
      else if(mode==contains_lbl)
	{
	  return filter_by_flid(results);	  
	}
      else
        {
          return false;
        }
    }

    bool query_flowop<FILTER>::filter_by_node_flavor(results_type& results)
    {
      auto& target = results.at(query_baseop::flid);

      auto& nodes = query_baseop::model_ptr->get_nodes();

      base_node node;
      for(auto sid:query_baseop::dependencies)
        {
          auto& source = results.at(sid);
          for(auto itr_i=source->begin(); itr_i!=source->end(); itr_i++)
            {
              if(nodes.get(itr_i->hash, node))
                {
                  if(flavors.count(node.get_flvr()))
                    {
                      target->set(itr_i->hash, itr_i->count, itr_i->prob);
                    }
                }
            }
        }

      target->normalise();

      query_baseop::done = true;
      return query_baseop::done;
    }

    bool query_flowop<FILTER>::filter_by_node_text(results_type& results)
    {
      auto& target = results.at(query_baseop::flid);

      auto& nodes = query_baseop::model_ptr->get_nodes();

      base_node node;
      for(auto sid:query_baseop::dependencies)
        {
          auto& source = results.at(sid);
          for(auto itr_i=source->begin(); itr_i!=source->end(); itr_i++)
            {
              if(nodes.get(itr_i->hash, node))
                {
		  std::string text = node.get_text(nodes, false);

		  for(auto& expr:exprs)
		    {
		      if(expr.match(text))
			{
			  target->set(itr_i->hash, itr_i->count, itr_i->prob);
			}
		    }
		}
	    }

	}

      target->normalise();

      query_baseop::done = true;
      return query_baseop::done;
    }

    bool query_flowop<FILTER>::filter_by_flid(results_type& results)
    {
      if(results.count(filter_flid)==0)
	{
	  return false;
	}
      
      auto& filter = results.at(filter_flid);
      auto& target = results.at(query_baseop::flid);

      auto& nodes = query_baseop::model_ptr->get_nodes();

      std::vector<hash_type> nhashes={}, thashes={};
      
      base_node node;
      for(auto sid:query_baseop::dependencies)
        {
          auto& source = results.at(sid);
          for(auto itr_i=source->begin(); itr_i!=source->end(); itr_i++)
            {
              if(nodes.get(itr_i->hash, node))
                {
		  bool contains = filter->has_node(itr_i->hash);

		  nhashes={};
		  thashes={};
		  
		  if(not contains)
		    {
		      nhashes = node.get_nodes();
		    }

		  if(nhashes.size()>0)
		    {
		      for(auto hash:nhashes)
			{
			  contains = filter->has_node(hash);
			  
			  if(contains)
			    {
			      break;
			    }
			}
		    }
		  
		  if(not contains)
		    {
		      node.get_token_path(nodes, thashes);
		    }

		  if(thashes.size()>0)
		    {
		      for(auto hash:thashes)
			{
			  contains = filter->has_node(hash);
			  
			  if(contains)
			    {
			      break;
			    }
			}		      
		    }
		  
		  if(contains)
		    {
		      target->set(itr_i->hash, itr_i->count, itr_i->prob);		      
		    }
		}
	    }

	}

      target->normalise();

      query_baseop::done = true;
      return query_baseop::done;
    }
    
  }

}

#endif
