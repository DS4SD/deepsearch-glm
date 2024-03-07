//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_INTERSECT_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_FLOWOP_INTERSECT_H_

namespace andromeda
{
  namespace glm
  {
    template<>
    class query_flowop<INTERSECT>: public query_baseop
    {
      const static flowop_name NAME = INTERSECT;

    public:

      query_flowop(std::shared_ptr<model_type> model,
		   flow_id_type flid, std::set<flow_id_type> dependencies,
		   const nlohmann::json& config);
      
      virtual ~query_flowop();

      virtual nlohmann::json to_config();
      virtual bool from_config(const nlohmann::json& config);
      
      virtual bool execute(results_type& results);

    private:

      std::string mode;
      //std::set<flow_id_type> sources;
    };

    query_flowop<INTERSECT>::query_flowop(std::shared_ptr<model_type> model,
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

    nlohmann::json query_flowop<INTERSECT>::to_config()
    {
      nlohmann::json config = query_baseop::to_config();

      nlohmann::json& params = config.at(parameters_lbl);
      {
	params["mode"] = mode;
	params["sources"] = query_baseop::dependencies;
      }
      
      return config;
    }
    
    bool query_flowop<INTERSECT>::from_config(const nlohmann::json& config)
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

    query_flowop<INTERSECT>::~query_flowop()
    {}

    bool query_flowop<INTERSECT>::execute(results_type& results)
    {
      auto& target = results.at(query_baseop::flid);

      bool first=true;
      for(auto sid:query_baseop::dependencies)
        {
          auto& source = results.at(sid);
          source->normalise();

          if(first)
            {
              for(auto itr=source->begin(); itr!=source->end(); itr++)
                {
                  target->set(itr->hash, itr->count, itr->prob);
                }

              first = false;
            }
          else
            {
              std::vector<index_type> to_be_erased={};
              for(auto itr_i=target->begin(); itr_i!=target->end(); itr_i++)
                {
                  auto itr_j = source->find(itr_i->hash);

                  if(itr_j==source->end())
                    {
                      to_be_erased.push_back(itr_i->hash);
                    }
                  else
                    {
                      assert(itr_i->hash==itr_j->hash);

                      // weight stores the prob of the first source!
                      val_type val = std::min(itr_i->weight, itr_j->prob);
                      ind_type cnt = std::min(itr_i->count, itr_j->count);

                      target->set(itr_i->hash, cnt, val);

                      if(std::abs(val)<1.e-6)
                        {
                          to_be_erased.push_back(itr_i->hash);
                        }
                    }
                }

              target->erase(to_be_erased);
            }
        }

      target->normalise();

      query_baseop::done = true;
      return query_baseop::done;
    }

  }

}

#endif
