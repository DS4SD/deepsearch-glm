//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_AUGMENT_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_AUGMENT_H_

#include <andromeda/glm/model_cli/augment/config.h>

#include <andromeda/glm/model_cli/augment/singplur.h>
#include <andromeda/glm/model_cli/augment/taxtree.h>

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class model_cli<AUGMENT, model_type>
    {
    public:

      model_cli(std::shared_ptr<model_type> model);

      model_cli(std::shared_ptr<model_type> model,
                nlohmann::json config);

      ~model_cli();

      nlohmann::json to_config();

      void augment();

    private:

      void initialise();
      void finalise();

    private:

      std::shared_ptr<model_type> model_ptr;

      augment_config configuration;
    };

    template<typename model_type>
    model_cli<AUGMENT, model_type>::model_cli(std::shared_ptr<model_type> model_ptr):
      model_ptr(model_ptr),
      configuration()
    {}

    template<typename model_type>
    model_cli<AUGMENT, model_type>::model_cli(std::shared_ptr<model_type> model,
                                              nlohmann::json config):
      model_ptr(model_ptr),
      configuration(config)
    {}

    template<typename model_type>
    model_cli<AUGMENT, model_type>::~model_cli()
    {}

    template<typename model_type>
    nlohmann::json model_cli<AUGMENT, model_type>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      config["mode"] = to_string(AUGMENT);

      {
        config["parameters"] = (model_ptr->get_parameters()).to_json();
      }

      {
        nlohmann::json item = configuration.to_json();
        config.merge_patch(item);
      }

      {
        nlohmann::json item = model_op<SAVE>::to_config();
        config.merge_patch(item);
      }

      {
        nlohmann::json item = model_op<LOAD>::to_config();
        config.merge_patch(item);
      }

      return config;
    }

    template<typename model_type>
    void model_cli<AUGMENT, model_type>::initialise()
    {}

    template<typename model_type>
    void model_cli<AUGMENT, model_type>::finalise()
    {
      auto& nodes = model_ptr->get_nodes();
      auto& edges = model_ptr->get_edges();

      nodes.sort();
      edges.sort();
      
      model_ptr->finalise(true);
    }    
    
    template<typename model_type>
    void model_cli<AUGMENT, model_type>::augment()
    {
      initialise();

      {
	singplur_augmenter<model_type> augmenter(model_ptr);
	augmenter.augment(2);
      }

      {
	taxtree_augmenter<model_type> augmenter(model_ptr);
	augmenter.augment();
      }      
      
      finalise();
    }

  }

}


#endif
