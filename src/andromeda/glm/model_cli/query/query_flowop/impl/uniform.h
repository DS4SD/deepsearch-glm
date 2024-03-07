//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_UNIFORM_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_UNIFORM_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class query_flowop<UNIFORM>: public query_baseop
    {
      const static flowop_name NAME = UNIFORM;

      typedef query_baseop baseop_type;

      typedef typename baseop_type::results_type results_type;

    public:

      query_flowop(std::shared_ptr<model_type> model,
		   flow_id_type flid, std::set<flow_id_type> dependencies,
		   const nlohmann::json& config);
      
      query_flowop(flow_id_type id, std::shared_ptr<model_type> model,
		   flow_id_type source_id);

      virtual ~query_flowop();

      virtual nlohmann::json to_config();// { return nlohmann::json::object({}); }
      virtual bool from_config(const nlohmann::json& config);// { return false;}
      
      virtual bool execute(results_type& results);

    private:

      flow_id_type source_id;
    };

    query_flowop<UNIFORM>::query_flowop(std::shared_ptr<model_type> model,
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

    nlohmann::json query_flowop<UNIFORM>::to_config()
    {
      nlohmann::json config = query_baseop::to_config();

      nlohmann::json& params = config.at(parameters_lbl);
      {
	params["source"] = source_id;
      }
      
      return config;
    }
    
    bool query_flowop<UNIFORM>::from_config(const nlohmann::json& config)
    {
      query_baseop::set_output_parameters(config);
      
      try
	{
	  const nlohmann::json& params = config.at(parameters_lbl);
	  source_id = params["source"].get<flow_id_type>();
	}
      catch(std::exception& exc)
	{
	  LOG_S(WARNING) << exc.what();
	  return false;
	}

      return true;      
    }
    
    query_flowop<UNIFORM>::query_flowop(flow_id_type flid, std::shared_ptr<model_type> model,
					flow_id_type source_id):
      query_baseop(model, NAME, flid, {source_id}),
      source_id(source_id)
    {
      //node_hashes.clear();
    }

    query_flowop<UNIFORM>::~query_flowop()
    {}

    bool query_flowop<UNIFORM>::execute(results_type& results)
    {
      auto& source = results.at(source_id);      
      auto& target = results.at(baseop_type::flid);

      for(auto itr_i=source->begin(); itr_i!=source->end(); itr_i++)
	{
          target->set(itr_i->hash, 1, 1.0);
        }
      target->normalise();

      baseop_type::done = true;
      return baseop_type::done;
    }

  }

}

#endif
