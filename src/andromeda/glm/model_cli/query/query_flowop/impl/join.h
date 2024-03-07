//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_JOIN_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_JOIN_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class query_flowop<JOIN>: public query_baseop
    {
      const static flowop_name NAME = JOIN;

    public:

      query_flowop(std::shared_ptr<model_type> model,
		   flow_id_type id, std::set<flow_id_type> dependencies,
		   const nlohmann::json& config);

      virtual ~query_flowop();

      virtual nlohmann::json to_config();// { return nlohmann::json::object({}); }
      virtual bool from_config(const nlohmann::json& config);// { return false;}
      
      virtual bool execute(results_type& results);

    private:

      std::string mode;
      
      //std::set<flow_id_type> sources;
    };

    query_flowop<JOIN>::query_flowop(std::shared_ptr<model_type> model,
				     flow_id_type flid, std::set<flow_id_type> dependencies,
				     const nlohmann::json& config):
      query_baseop(model, NAME, flid, dependencies),

      mode("default")
    {
      if((not config.is_null()) and
	 (not from_config(config)))
	{
	  LOG_S(WARNING) << "implement query_flowop<" << to_string(NAME) << "> "
			 << "with config: " << config.dump(2);
	}
    }

    nlohmann::json query_flowop<JOIN>::to_config()
    {
      nlohmann::json config = query_baseop::to_config();

      nlohmann::json& params = config.at(parameters_lbl);
      {
	params["mode"] = mode;
	params["sources"] = query_baseop::dependencies;
      }

      return config;
    }
    
    bool query_flowop<JOIN>::from_config(const nlohmann::json& config)
    {
      query_baseop::set_output_parameters(config);
            
      nlohmann::json params = config;
      if(config.count(parameters_lbl))
	{
	  params = config.at(parameters_lbl);
	}
      
      try
	{
	  mode = params.value("mode", mode);
	}
      catch(std::exception& exc)
	{
	  LOG_S(WARNING) << exc.what();
	  return false;
	}

      return true;
    }    

    query_flowop<JOIN>::~query_flowop()
    {}

    bool query_flowop<JOIN>::execute(results_type& results)
    {
      auto& target = results.at(query_baseop::flid);

      for(auto sid:query_baseop::dependencies)
        {
          auto& source = results.at(sid);
          source->normalise();

          for(auto itr_i=source->begin(); itr_i!=source->end(); itr_i++)
            {
              auto itr_j = target->find(itr_i->hash);

              if(itr_j==target->end())
                {
                  target->set(itr_i->hash, itr_i->count, itr_i->prob);
                }
              else
                {
                  val_type val = std::max(itr_i->prob, itr_j->prob);
                  ind_type cnt = std::max(itr_i->count, itr_j->count);

                  target->set(itr_j->hash, cnt, val);
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
