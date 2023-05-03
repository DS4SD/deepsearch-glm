//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_NLP_INTERFACE_H
#define PYBIND_ANDROMEDA_NLP_INTERFACE_H

#include "andromeda.h"

namespace andromeda_py
{  
  class nlp_model
  {
  public:
    
    nlp_model();
    ~nlp_model();

    void initialise(const std::string model_expr="term");

    nlohmann::json get_apply_configs();
    nlohmann::json get_train_configs();
    
    nlohmann::json apply(nlohmann::json& config);
    nlohmann::json train(nlohmann::json& config);
    
    nlohmann::json apply_on_text(std::string& text);
    
    nlohmann::json apply_on_pdfdoc(nlohmann::json& doc);

  private:

    void apply_paragraphs(std::shared_ptr<andromeda::producer<andromeda::PARAGRAPH> > producer,
			  nlohmann::json& results);

    void apply_pdfdocs(std::shared_ptr<andromeda::producer<andromeda::DOCUMENT> > producer,
		       nlohmann::json& results);
    
  private:

    std::vector<std::shared_ptr<andromeda::base_nlp_model> > models;

    std::shared_ptr<andromeda::utils::char_normaliser> char_normaliser;
    std::shared_ptr<andromeda::utils::text_normaliser> text_normaliser;    
  };

  nlp_model::nlp_model():
    models({}),

    char_normaliser(andromeda::text_element::create_char_normaliser(false)),
    text_normaliser(andromeda::text_element::create_text_normaliser(false))
  {}
  
  nlp_model::~nlp_model()
  {}
  
  void nlp_model::initialise(const std::string model_expr)
  {
    andromeda::to_models(model_expr, this->models, false);
  }

  nlohmann::json nlp_model::get_apply_configs()
  {
    nlohmann::json configs = nlohmann::json::array({});

    {
      nlohmann::json config;
      {
	config["mode"] = "predict";
	
	config["models"] = "reference;conn;term;verb";
	config["interactive"] = true;
	
	config["subject"] = "<prompt;paragraph;paragraph:jsonl:text;pdfdoc>";
	
	config["input"] = "";
	config["output"] = "";
      }

      configs.push_back(config);
    }

    return configs;
  }

  nlohmann::json nlp_model::get_train_configs()
  {
    nlohmann::json configs = nlohmann::json::array({});
    
    std::shared_ptr<andromeda::base_nlp_model> model;
    for(auto name:andromeda::MODEL_NAMES)
      {
	model = andromeda::to_trainable_model(name);
	LOG_S(INFO) << "model: " << model;
	
	if(model!=NULL and model->is_trainable())
	  {
	    LOG_S(INFO) << andromeda::to_string(name) << ": trainable";
	    
	    nlohmann::json config = model->create_train_config();
	    configs.push_back(config);
	  }
	else
	  {
	    LOG_S(WARNING) << andromeda::to_string(name) << ": not trainable";
	  }
      }

    return configs;    
  }
  
  nlohmann::json nlp_model::apply(nlohmann::json& config)
  {
    nlohmann::json result = nlohmann::json::array({});

    std::vector<std::shared_ptr<andromeda::base_nlp_model> > nlp_models={};
    andromeda::to_models(config, nlp_models, false);
    
    std::vector<std::shared_ptr<andromeda::base_producer> > base_producers={};
    andromeda::to_producers(config, nlp_models, base_producers);

    nlohmann::json results = nlohmann::json::array({});

    for(auto& base_producer:base_producers)
      {
	switch(base_producer->get_subject_name())
	  {
	  case andromeda::PARAGRAPH:
	    {
	      typedef andromeda::producer<andromeda::PARAGRAPH> producer_type;
	      auto producer = std::dynamic_pointer_cast<producer_type>(base_producer);

	      apply_paragraphs(producer, /*exporter,*/ results);
	    }
	    break;

	  case andromeda::DOCUMENT:
	    {
	      typedef andromeda::producer<andromeda::DOCUMENT> producer_type;
	      auto producer = std::dynamic_pointer_cast<producer_type>(base_producer);

	      apply_pdfdocs(producer, /*exporter,*/ results);
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
    
    return results;
  }

  void nlp_model::apply_paragraphs(std::shared_ptr<andromeda::producer<andromeda::PARAGRAPH> > producer,
				   nlohmann::json& results)
  {
    andromeda::subject<andromeda::PARAGRAPH> subj;

    std::size_t count=0;
    
    bool read=true;
    while(read)      
      {
	{
	  //std::scoped_lock lock(read_mtx);
	  read = producer->read(subj, count);
	}

	if(not read)
	  {
	    continue;
	  }
	
	producer->apply(subj);
	
	//nlohmann::json out = subject.to_json();
	subj.show(true, true,
		  false, true,
		  true, true, true);
	
	std::string line="y";
	std::getline(std::cin, line);
	
	if(line=="quit" or line=="q")
	  {
	    break;
	  }	    	
      }
  }
  
  void nlp_model::apply_pdfdocs(std::shared_ptr<andromeda::producer<andromeda::DOCUMENT> > producer,
				nlohmann::json& results)
  {
    andromeda::subject<andromeda::DOCUMENT> subj;

    std::size_t count=0;
    
    bool read=true;
    while(read)
      {
	{
	  //std::scoped_lock lock(read_mtx);
	  read = producer->read(subj, count);
	}

	if(not read)
	  {
	    continue;
	  }
	
	producer->apply(subj);
      }
  }  
  
  nlohmann::json nlp_model::train(nlohmann::json& config)
  {
    std::string model_name = "null";
    model_name = config.value("model", model_name);
    
    andromeda::model_name name = andromeda::to_modelname(model_name);
    std::shared_ptr<andromeda::base_nlp_model> model = andromeda::to_trainable_model(name);

    bool success=false;
    std::stringstream ss;    

    if(model==NULL or (not (model->is_trainable())))
      {
	ss << "model '" << model_name << "' can not be trained";
      }
    else
      {
	success = model->train(config);
	
	if(success)
	  {
	    ss << "model '" << model->get_key() << "' is trained!";
	  }
	else
	  {
	    ss << "model '" << model->get_key() << "' is not trained!";
	  }	
      }

    nlohmann::json result = config;
    {
      nlohmann::json& training = result["model-training"];
      {
	training["success"] = success;
	training["message"] = ss.str();
      }
    }
    
    return result;
  }
  
  nlohmann::json nlp_model::apply_on_text(std::string& text)
  {
    andromeda::subject<andromeda::PARAGRAPH> paragraph;
    bool valid = paragraph.set(text, char_normaliser, text_normaliser);

    bool success=false;
    std::stringstream ss;    
    
    if(valid)
      {
	for(auto& model:models)
	  {
	    model->apply(paragraph);
	  }

	success=true;
	ss << "success";
      }
    else
      {
	success=false;
	ss << "text is not UTF8 compliant";
      }
    
    nlohmann::json result = paragraph.to_json();
    {
      nlohmann::json& application = result["model-application"];
      {
	application["success"] = success;
	application["message"] = ss.str();
      }
    }
    
    return result;
  }

  nlohmann::json nlp_model::apply_on_pdfdoc(nlohmann::json& data)
  {
    andromeda::subject<andromeda::DOCUMENT> pdfdoc;    
    bool valid = pdfdoc.set(data, char_normaliser, text_normaliser);

    bool success=false;
    std::stringstream ss;    
    
    if(valid)
      {
	for(auto& model:models)
	  {
	    model->apply(pdfdoc);
	  }

	success=true;
	ss << "success";	
      }
    else
      {
	success=true;
        ss << "pdfdoc is not compliant ... aborting";
      }

    nlohmann::json result = pdfdoc.to_json();
    {
      nlohmann::json& application = result["model-application"];
      {
	application["success"] = success;
	application["message"] = ss.str();
      }
    }
    
    return result;
  }
  
}

#endif

