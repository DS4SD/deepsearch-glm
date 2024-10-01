//-*-C++-*-

#include <andromeda/nlp/tok.h> // tokenizer
#include <andromeda/nlp/pos.h> // part-of-speech
#include <andromeda/nlp/cls.h> // classification
#include <andromeda/nlp/ent.h> // entity
#include <andromeda/nlp/rel.h> // relation (binary)
#include <andromeda/nlp/rec.h> // relation (multi)

#include <andromeda/nlp/utils.h>

#ifndef ANDROMEDA_NLP_H
#define ANDROMEDA_NLP_H

namespace andromeda
{

  void write_nlp_configs(nlohmann::json& configs, std::filesystem::path path)
  {
    for(nlohmann::json config:configs)
      {
	if(config.count("mode")==1 and
	   config.count("model")==1)
	  {
	    std::string mode = config["mode"].get<std::string>();
	    std::string model = config["model"].get<std::string>();

	    std::stringstream ss;
	    ss << "nlp_" << mode << "_" << model << ".example.json"; 
	
	    std::filesystem::path filename = path / ss.str();
	    // LOG_S(INFO) << "writing " << filename.c_str();
		LOG_S(INFO) << "writing " << filename.string();

	
	    // std::ofstream ofs(filename.c_str());
		std::ofstream ofs(filename);
	    if(ofs.good())
	      {
		ofs << config.dump(4);
	      }	
	  }
	else if(config.count("mode")==1 and
		config.count("models")==1)
	  {
	    std::string mode = config["mode"].get<std::string>();
	    //std::string models = config["models"].get<std::string>();

	    std::stringstream ss;
	    ss << "nlp_" << mode << ".example.json"; 
	
	    std::filesystem::path filename = path / ss.str();
	    // LOG_S(INFO) << "writing " << filename.c_str();
		LOG_S(INFO) << "writing " << filename.string();
	
	    // std::ofstream ofs(filename.c_str());
		std::ofstream ofs(filename);
	    if(ofs.good())
	      {
		ofs << config.dump(4);
	      }	
	  }
	else
	  {
	    LOG_S(WARNING) << "can not write " << config.dump(2);
	  }
      }
  }
  
  nlohmann::json create_nlp_configs()
  {
    LOG_S(INFO) << __FUNCTION__;
    
    nlohmann::json configs = nlohmann::json::array({});
    
    for(model_name name:MODEL_NAMES)
      {
	LOG_S(INFO) << name << ": " << to_string(name);
	
	auto model = to_trainable_model(name);

	if(model!=NULL)
	  {
	    nlohmann::json config = model->create_train_config();
	    configs.push_back(config);
	  }
      }
    
    // writing config for predict
    {
      nlohmann::json config;
      {
        config["mode"] = "predict";

        config["models"] = "conn;verb;term;reference";
        config["interactive"] = true;

	auto& producers = config[base_producer::producers_lbl];

	{
	  andromeda::producer<andromeda::PROMPT> producer;
	  for(auto item:producer.to_json())
	    {
	      producers.push_back(item);
	    }
	}
	
	{
	  andromeda::producer<andromeda::TEXT> producer;
	  for(auto item:producer.to_json())
	    {
	      producers.push_back(item);
	    }
	}
	
	{
	  andromeda::producer<andromeda::DOCUMENT> producer;
	  for(auto item:producer.to_json())
	    {
	      producers.push_back(item);
	    }
	}
      }

      configs.push_back(config);
    }

    return configs;
  }

  template<typename producer_type>
  void nlp_predict_on_producer(std::shared_ptr<producer_type> producer,
			       nlohmann::json& config, bool verbose)
  {
    typedef typename producer_type::subject_type subject_type;

    bool interactive = true;
    interactive = config.value("interactive", interactive);
    
    subject_type subject;    
    std::size_t count=0;

    auto start = std::chrono::system_clock::now();
    
    producer->reset_pointer();
    while(producer->keep_reading(count))
      {
	subject.clear();
	if(not producer->read(subject, count))
	  {
	    continue;
	  }

	producer->apply(subject);
	producer->write(subject);
	
	if(interactive)
	  {
	    subject.show(true, true,
			 false, true,
			 true, true, true);

	    std::string line="y";
	    std::getline(std::cin, line);
	    
	    if(line=="quit" or line=="q")
	      {
		break;
	      }	    			    
	  }
	else if(verbose)
	  {
	    auto end = std::chrono::system_clock::now();
	    std::chrono::duration<double> diff = end-start;
	    
	    std::cout << "\rtotal #-docs: " << std::setw(8) << std::fixed << count << ", "
		      << std::setprecision(4)
		      << "time (sec): " << std::setw(8) << diff.count() << ", "
		      << "speed (sec/doc): " << std::setw(8) << diff.count()/count
		      << std::flush;
	  }
	else
	  {}
      }
  }
  
  void nlp_predict(nlohmann::json& config)
  {
    std::vector<std::shared_ptr<andromeda::base_nlp_model> > nlp_models={};
    andromeda::to_models(config, nlp_models, true);
    
    std::vector<std::shared_ptr<andromeda::base_producer> > base_producers={};
    andromeda::to_producers(config, nlp_models, base_producers);

    for(auto& base_producer:base_producers)
      {
	switch(base_producer->get_subject_name())
	  {
	  case andromeda::PROMPT:
	    {
	      typedef andromeda::producer<andromeda::PROMPT> producer_type;
	      auto producer = std::dynamic_pointer_cast<producer_type>(base_producer);
	      
	      nlp_predict_on_producer(producer, config, true);
	    }
	    break;
	    
	  case andromeda::TEXT:
	    {
	      typedef andromeda::producer<andromeda::TEXT> producer_type;
	      auto producer = std::dynamic_pointer_cast<producer_type>(base_producer);
	      
	      nlp_predict_on_producer(producer, config, true);
	    }
	    break;

	  case andromeda::DOCUMENT:
	    {
	      typedef andromeda::producer<andromeda::DOCUMENT> producer_type;
	      auto producer = std::dynamic_pointer_cast<producer_type>(base_producer);

	      nlp_predict_on_producer(producer, config, true);
	    }
	    break;	    

	  default:
	    {
	      LOG_S(WARNING) << "nlp-model (" << __FILE__ << ":" << __LINE__
			     << ") does not support producer-type: "
			     << to_string(base_producer->get_subject_name());
	    }

	  }
      }    
  }

  void nlp_train(nlohmann::json& config)
  {
    std::string value = "null";
    value = config.value("model", value);
  
    andromeda::model_name name = andromeda::to_modelname(value);
    auto model = andromeda::to_trainable_model(name);
    
    if(model==NULL or (not (model->is_trainable())))
      {
	LOG_S(ERROR) << "model '" << value << "' can not be trained";
	return;
      }
    
    bool success = model->train(config);
    
    if(success)
      {
	LOG_S(INFO) << "model '" << model->get_key() << "' is trained!";
      }
    else
      {
	LOG_S(WARNING) << "model '" << model->get_key() << "' is not trained!";
      }    
  }
  
}

#endif
