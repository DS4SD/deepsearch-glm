//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_DISTILL_CONFIG_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_DISTILL_CONFIG_H_

namespace andromeda
{
  namespace glm
  {
    class distill_config
    {
    public:

      const static inline std::string min_edge_count_lbl = "min-edge-count";
      
    public:

      distill_config();

      nlohmann::json get();
      void set(const nlohmann::json config);

      std::size_t get_min_edge_count();
      
    private:

      nlohmann::json configuration;
      
      std::size_t MIN_EDGE_COUNT;
    };

    distill_config::distill_config():
      MIN_EDGE_COUNT(2)	
    {}

    nlohmann::json distill_config::get()
    {
      nlohmann::json config;
      {
	config[min_edge_count_lbl] = MIN_EDGE_COUNT;
      }
      
      return config;
    }
    
    void distill_config::set(const nlohmann::json config)
    {
      configuration = config;
      
      MIN_EDGE_COUNT = config.value(min_edge_count_lbl, MIN_EDGE_COUNT);
    }

    std::size_t distill_config::get_min_edge_count()
    {
      return MIN_EDGE_COUNT;
    }
    
  }

}

#endif
