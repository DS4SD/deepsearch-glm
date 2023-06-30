//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_CONFIG_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_CONFIG_H_

namespace andromeda
{
  namespace glm
  {
    class create_config
    {
      const static inline std::string create_lbl = to_string(CREATE);
      
      const static inline std::string io_lbl = io_base::io_lbl;
      
      const static inline std::string load_lbl = io_base::load_lbl;
      const static inline std::string save_lbl = io_base::save_lbl;

      const static inline std::string model_dir_lbl = io_base::root_lbl;

      const static inline std::string num_threads_lbl = "number-of-threads";
      const static inline std::string enforce_max_size_lbl = "enforce-max-size";

      const static inline std::string write_nlp_output_lbl = "write-nlp-output";
      
      const static inline std::string model_lbl = "model";
      const static inline std::string worker_lbl = "worker";

      const static inline std::string max_nodes_lbl = "max-nodes";
      const static inline std::string max_edges_lbl = "max-edges";
      //const static inline std::string max_paths_lbl = "max-paths";
      
      const static inline std::string local_reading_range_lbl = "local-reading-range";
      const static inline std::string local_reading_break_lbl = "local-reading-break";
      
    public:

      create_config();      
      create_config(nlohmann::json config);

      void from_json(nlohmann::json& config);
      
      nlohmann::json to_json();
      
    public:

      nlohmann::json configuration;

      std::string model_dir;

      std::size_t num_threads;

      bool enforce_max_size, local_reading_break;

      bool write_nlp_output;
      std::string nlp_output_dir;
      
      std::size_t min_local_line_count;
      std::size_t max_local_line_count;

      std::size_t max_total_nodes;
      std::size_t max_total_edges;

      std::size_t max_local_nodes;
      std::size_t max_local_edges;
    };

    create_config::create_config():
      configuration(),

      model_dir("./glm-model"),

      num_threads(4),

      enforce_max_size(false),
      local_reading_break(true),

      write_nlp_output(false),
      nlp_output_dir(model_dir+"/nlp-output"),
      
      min_local_line_count(256),
      max_local_line_count(10*min_local_line_count),

      max_total_nodes(1e7),
      max_total_edges(1e8),

      max_local_nodes(1e6),
      max_local_edges(1e7)
    {}
    
    create_config::create_config(nlohmann::json config):
      configuration(config),

      model_dir("./glm-model"),

      num_threads(1),

      enforce_max_size(false),
      local_reading_break(true),

      write_nlp_output(false),
      nlp_output_dir(model_dir+"/nlp-output"),
      
      min_local_line_count(256),
      max_local_line_count(10*min_local_line_count),

      max_total_nodes(1e7),
      max_total_edges(1e8),

      max_local_nodes(1e6),
      max_local_edges(1e7)
    {
      from_json(config);
    }

    void create_config::from_json(nlohmann::json& config)
    {
      if(config.count(create_lbl)==1)
        {
          nlohmann::json& create = config[create_lbl];

          num_threads = create.value(num_threads_lbl, num_threads);
          enforce_max_size = create.value(enforce_max_size_lbl, enforce_max_size);

          write_nlp_output = create.value(write_nlp_output_lbl, write_nlp_output);
	  
	  nlohmann::json& model = create[model_lbl];
	  {
	    max_total_nodes = model.value(max_nodes_lbl, max_total_nodes);
	    max_total_edges = model.value(max_edges_lbl, max_total_edges);
	  }

	  nlohmann::json& worker = create[worker_lbl];
	  {
	    max_local_nodes = worker.value(max_nodes_lbl, max_local_nodes);
	    max_local_edges = worker.value(max_edges_lbl, max_local_edges);

	    std::array<std::size_t, 2> rng = { min_local_line_count, max_local_line_count};
	    rng = worker.value(local_reading_range_lbl, rng);
	    
	    min_local_line_count = rng.at(0); //worker.value(min_local_reading_lbl, min_local_line_count);
	    max_local_line_count = rng.at(1); //worker.value(max_local_reading_lbl, max_local_line_count);

	    local_reading_break = worker.value(local_reading_break_lbl, local_reading_break);	  
	  }
	}

      if(config.count(io_lbl) and config[io_lbl].count(save_lbl))
        {
          nlohmann::json save = config[io_lbl][save_lbl];
          model_dir = save.value(model_dir_lbl, model_dir);

	  nlp_output_dir = model_dir + "/" + "nlp-output";
        }

      if(not std::filesystem::exists(model_dir))
	{
	  std::filesystem::create_directory(model_dir);
	}
      
      if(not std::filesystem::exists(nlp_output_dir))
	{
	  std::filesystem::create_directory(nlp_output_dir);	  
	}
    }
    
    nlohmann::json create_config::to_json()
    {
      nlohmann::json result;

      result["mode"] = to_string(CREATE);
      
      auto& create = result[create_lbl];
      {
	create[num_threads_lbl] = num_threads;
	create[enforce_max_size_lbl] = enforce_max_size;

	create[write_nlp_output_lbl] = write_nlp_output;
	
	nlohmann::json& model = create[model_lbl];
	{
	  model[max_nodes_lbl] = max_total_nodes;
	  model[max_edges_lbl] = max_total_edges;
	}

	nlohmann::json& worker = create[worker_lbl];
	{
	  worker[max_nodes_lbl] = max_local_nodes;
	  worker[max_edges_lbl] = max_local_edges;

	  std::array<std::size_t, 2> local_reading_range = {min_local_line_count, max_local_line_count};
	  worker[local_reading_range_lbl] = local_reading_range;
	  worker[local_reading_break_lbl] = local_reading_break;	  
	}
      }
      
      return result;
    }
    
  }

}

#endif
