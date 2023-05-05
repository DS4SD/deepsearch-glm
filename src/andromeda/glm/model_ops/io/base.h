#ifndef ANDROMEDA_MODELS_GLM_MODELOPS_SAVE_H_
#define ANDROMEDA_MODELS_GLM_MODELOPS_SAVE_H_

namespace andromeda
{
  namespace glm
  {

    class io_base: public base_types
    {
    public:
      
      const static inline std::filesystem::path DEFAULT_ROOT = "glm-model-default";

      const static inline std::string io_lbl = "IO";
      
      const static inline std::string load_lbl = "load";
      const static inline std::string save_lbl = "save";

      const static inline std::string root_lbl = "root";

      const static inline std::string write_json_lbl = "write-JSON";      
      const static inline std::string write_csv_lbl = "write-CSV";
      
      const static inline std::string save_rtext_lbl = "write-path-text";
      
    public:

      io_base();

      static bool has_save(const nlohmann::json& config);
      static bool has_load(const nlohmann::json& config);
      
      bool paths_exists();
      
      bool set_paths(std::filesystem::path path);

      bool create_paths(std::filesystem::path path);

    protected:

      std::filesystem::path model_root_dir;
      
      std::filesystem::path param_file;
      std::filesystem::path topo_file;
      
      std::filesystem::path nodes_file;
      std::filesystem::path edges_file;

      // additional files (not needed for LOAD, but useful for additional analysis)
      std::filesystem::path topo_file_text;

      std::filesystem::path nodes_file_json;
      
      std::filesystem::path nodes_file_csv;
      std::filesystem::path edges_file_csv;
    };

    io_base::io_base()
    {}

    bool io_base::has_save(const nlohmann::json& config)
    {
      if(config.count(io_lbl) and
	 config[io_lbl].count(save_lbl) and
	 config[io_lbl][save_lbl].count(root_lbl))
	{
	  return true;
	}

      return false;
    }

    bool io_base::has_load(const nlohmann::json& config)
    {
      if(config.count(io_lbl) and
	 config[io_lbl].count(load_lbl) and
	 config[io_lbl][load_lbl].count(root_lbl))
	{
	  return true;
	}

      return false;
    }
    
    bool io_base::set_paths(std::filesystem::path path)
    {
      model_root_dir = path;
      
      param_file = model_root_dir / "parameters.json";
      topo_file = path / "topology.json";
      
      nodes_file = model_root_dir / "nodes.bin";
      edges_file = model_root_dir / "edges.bin";

      // additional files (not needed for LOAD, but useful for additional analysis)
      topo_file_text = path / "topology.txt";

      nodes_file_json = model_root_dir / "nodes.jsonl";
      
      nodes_file_csv = model_root_dir / "nodes.csv";
      edges_file_csv = model_root_dir / "edges.csv";
      
      return true;
    }
    
    bool io_base::paths_exists()
    {
      std::vector<std::filesystem::path> paths = { model_root_dir,
						   param_file, topo_file,
						   nodes_file, edges_file};

      bool result = true;
      for(auto _:paths)
	{
	  if(not std::filesystem::exists(_))
	    {
	      LOG_S(ERROR) << "path does not exist: " << _;
	      result = false;
	    }
	}

      return result;
    }
    
    bool io_base::create_paths(std::filesystem::path path)
    {
      set_paths(path);

      if(not std::filesystem::exists(model_root_dir))
	{
	  LOG_S(WARNING) << "creating path: " << model_root_dir; 
	  std::filesystem::create_directory(model_root_dir);
	}

      return true;
    }
    
  }

}

#endif
