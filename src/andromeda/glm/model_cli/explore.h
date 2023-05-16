//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_H_

#include <andromeda/glm/model_cli/explore/base.h>
#include <andromeda/glm/model_cli/explore/taxonomy.h>
#include <andromeda/glm/model_cli/explore/mask.h>

namespace andromeda
{
  namespace glm
  {

    template<typename model_type>
    class model_cli<EXPLORE, model_type>
    {
      const static inline std::string explore_lbl = to_string(EXPLORE);
      
      const static inline std::string io_lbl = io_base::io_lbl;
      
      const static inline std::string load_lbl = io_base::load_lbl;
      const static inline std::string save_lbl = io_base::save_lbl;

      const static inline std::string model_dir_lbl = io_base::root_lbl;
      
      typedef typename model_type::index_type index_type;

      typedef typename model_type::node_type node_type;
      typedef typename model_type::edge_type edge_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

    public:

      model_cli(std::shared_ptr<model_type> model);
      ~model_cli();

      nlohmann::json to_config();
      
      void interactive();

    private:

      bool execute_query();
      
    private:

      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    model_cli<EXPLORE, model_type>::model_cli(std::shared_ptr<model_type> model):
      model(model)
    {}

    template<typename model_type>
    model_cli<EXPLORE, model_type>::~model_cli()
    {}

    template<typename model_type>
    nlohmann::json model_cli<EXPLORE, model_type>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      config["mode"] = to_string(EXPLORE);

      {
	nlohmann::json item = model_op<LOAD>::to_config();
	config.merge_patch(item);
      }
      
      return config;
    }
    
    template<typename model_type>
    void model_cli<EXPLORE, model_type>::interactive()
    {
      std::string mode;
      while(true)
        {
          std::cout << "exploring mode [`quit`, `word`, `taxonomy`, `query`]: ";
          std::getline(std::cin, mode);

          if(mode=="quit")
            {
              break;
            }
          else if(mode=="word")
            {
	      taxonomy<model_type> tax(model);
	      tax.explore_word();
            }
          else if(mode=="taxonomy")
            {
	      taxonomy<model_type> tax(model);
	      tax.explore_taxonomy();	      
            }
          else if(mode=="query")
            {
	      this->execute_query();
            }	  
	  /*
          else if(mode=="related")
            {
	      taxonomy<model_type> tax(model);
	      tax.explore_related();	      
            }	  
          else if(mode=="mask")
            {
	      text_masker<model_type> masker(model);
	      masker.interactive();
            }
	  */
          else
            {}
        }
    }

    template<typename model_type>
    bool model_cli<EXPLORE, model_type>::execute_query()
    {
      model_cli<QUERY, model_type> querier(model);
      
      std::string filename;
      while(true)
        {
          std::cout << "read query from file: ";
          std::getline(std::cin, filename);

          if(filename=="quit")
            {
              break;
            }

	  nlohmann::json query, result;
	  
	  std::ifstream ifs(filename.c_str());
	  if(ifs.good())
	    {
	      ifs >> query;
	      querier.execute(query, result, true);
	    }
	  else
	    {
	      LOG_S(WARNING) << "could not read file: " << filename;
	    }
	}

      return true;
    }
    
  }

}

#endif
