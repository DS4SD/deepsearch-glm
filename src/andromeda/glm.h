//-*-C++-*-

#ifndef ANDROMEDA_GLM_H
#define ANDROMEDA_GLM_H

#include <andromeda/glm/model.h>

#include <andromeda/glm/model_ops.h>
#include <andromeda/glm/model_cli.h>

namespace andromeda
{
  namespace glm
  {
    void write_configs(nlohmann::json& configs, std::filesystem::path dir="./")
    {
      for(auto& config:configs)
        {
          std::string mode = config["mode"].get<std::string>();

          std::string filename = "glm_config_"+mode+".example.json";
          LOG_S(INFO) << "writing " << filename;

          std::ofstream ofs(filename.c_str());

          if(ofs.good()) { ofs << std::setw(4) << config << std::endl; }
          else { LOG_S(WARNING) << "could not open file to write ..."; }

          ofs.close();
        }
    }

    template<typename glm_model_type>
    nlohmann::json get_configurations(std::shared_ptr<glm_model_type> model)
    {
      nlohmann::json configs = nlohmann::json::array({});

      {
        glm::model_cli<glm::CREATE, glm_model_type> creator(model);
        nlohmann::json config = creator.to_config();

        auto& producers = config[base_producer::producers_lbl];

        {
          auto& nlp_models = model->get_parameters().models;
          andromeda::producer<andromeda::TEXT> producer(nlp_models);

          for(auto item:producer.to_json())
            {
              producers.push_back(item);
            }
        }

        {
          auto& nlp_models = model->get_parameters().models;
          andromeda::producer<andromeda::DOCUMENT> producer(nlp_models);

          for(auto item:producer.to_json())
            {
              producers.push_back(item);
            }
        }

        configs.push_back(config);
      }

      {
        glm::model_cli<glm::AUGMENT, glm_model_type> augmenter(model);
        configs.push_back(augmenter.to_config());
      }
      
      {
        glm::model_cli<glm::DISTILL, glm_model_type> distiller(model);
        configs.push_back(distiller.to_config());
      }

      {
        glm::model_cli<glm::QUERY, glm_model_type> querier(model);
        configs.push_back(querier.to_config());
      }

      {
        glm::model_cli<glm::EXPLORE, glm_model_type> explorer(model);
        configs.push_back(explorer.to_config());
      }

      return configs;
    }

    template<typename glm_model_type>
    void create_glm_model(nlohmann::json& config, std::shared_ptr<glm_model_type> model)
    {
      model->configure(config, true);

      auto& nlp_models = model->get_parameters().models;
      LOG_S(INFO) << "#-models: " << nlp_models.size();
      
      std::vector<std::shared_ptr<base_producer> > producers;
      andromeda::to_producers(config, nlp_models, producers);

      glm::model_cli<glm::CREATE, glm_model_type> creator(model, config);
      for(auto& producer:producers)
        {
          creator.create(producer);
        }

      if(glm::io_base::has_save(config))
        {
          glm::model_op<glm::SAVE> io;
	  
          io.from_config(config);
	  io.save(model);
        }
    }

    template<typename glm_model_type>
    void augment_glm_model(nlohmann::json& config, std::shared_ptr<glm_model_type> model)
    {
      if(glm::io_base::has_load(config))
        {
          glm::model_op<glm::LOAD> io;

          io.from_config(config);
	  io.set_incremental(true);

	  if(not io.load(model))
	    {
	      return;
	    }	  
        }
      else
        {
          return;
        }

      {
	glm::model_cli<glm::AUGMENT, glm_model_type> augmenter(model);
	augmenter.augment();	
      }

      if(glm::io_base::has_save(config))
        {
          glm::model_op<glm::SAVE> io;

          io.from_config(config);
	  io.save(model);
        }      
    }
    
    template<typename glm_model_type>
    void distill_glm_model(nlohmann::json& config,
			   std::shared_ptr<glm_model_type>& old_model,
			   std::shared_ptr<glm_model_type>& new_model)
    {
      if(glm::io_base::has_load(config))
        {
          glm::model_op<glm::LOAD> io;

          io.from_config(config);
	  io.set_incremental(false);

	  if(not io.load(old_model))
	    {
	      return;
	    }	  
        }

      if(old_model!=NULL)
	{
	  glm::model_cli<glm::DISTILL, glm_model_type> distillator(old_model);
	  distillator.from_config(config);
	  
	  new_model = distillator.distill();
	}
      
      if(new_model!=NULL and glm::io_base::has_save(config))
        {
          glm::model_op<glm::SAVE> io;

          io.from_config(config);
	  io.save(new_model);
        }
    }

    template<typename glm_model_type>
    void query_glm_model(nlohmann::json& config, nlohmann::json& output,
                         std::shared_ptr<glm_model_type> model)
    {
      LOG_S(INFO) << "config: \n" << config.dump(2);
      
      if(glm::io_base::has_load(config))
        {
          glm::model_op<glm::LOAD> io;

          io.from_config(config);
	  io.set_incremental(false);

	  if(not io.load(model))
	    {
	      return;
	    }
        }
      else
        {
          return;
        }

      glm::model_cli<glm::QUERY, glm_model_type> querier(model);
      querier.execute(config, output, false);
    }

    template<typename glm_model_type>
    void explore_glm_model(nlohmann::json& config, std::shared_ptr<glm_model_type> model)
    {
      if(glm::io_base::has_load(config))
        {
          glm::model_op<glm::LOAD> io;

	  io.from_config(config);
	  io.set_incremental(false);

	  if(not io.load(model))
	    {
	      return;
	    }
        }

      glm::model_cli<glm::EXPLORE, glm_model_type> explorer(model);
      explorer.interactive();
    }

  }

}

#endif
