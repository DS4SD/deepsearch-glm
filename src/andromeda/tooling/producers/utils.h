//-*-C++-*-

#ifndef ANDROMEDA_PRODUCER_UTILS_H_
#define ANDROMEDA_PRODUCER_UTILS_H_

namespace andromeda
{
  bool to_producer(nlohmann::json& config,
		   std::vector<std::shared_ptr<base_nlp_model> >& models,
		   std::vector<std::shared_ptr<base_producer> >& producers)
  {
    //LOG_S(INFO) << "initialising producer: " << config.dump();

    std::string subject_type="undef";
    subject_type = config.value(base_producer::subject_lbl, subject_type);
    
    std::shared_ptr<base_producer> producer=NULL;
    
    switch(to_subject_name(subject_type))
      {
      case PROMPT:
	{
	  producer = std::make_shared<andromeda::producer<andromeda::PROMPT> >(config, models);

	  producers.push_back(producer);
	  return true;	  
	}
	break;    
	
      case TEXT:
	{
	  producer = std::make_shared<andromeda::producer<andromeda::TEXT> >(config, models);

	  producers.push_back(producer);
	  return true;	  
	}
	break;    

      case DOCUMENT:
	{
	  producer = std::make_shared<andromeda::producer<andromeda::DOCUMENT> >(config, models);

	  producers.push_back(producer);
	  return true;	  
	}
	break;    	
	
      default:
	{
	  LOG_S(ERROR) << "no implementation for producer: " << subject_type;
	}
      }

    return false;
  }

  bool to_producers(nlohmann::json& configs,
		    std::vector<std::shared_ptr<base_nlp_model> >& models,
		    std::vector<std::shared_ptr<base_producer> >& producers)
  {
    if(configs.is_array())
      {    
	for(auto& config:configs)
	  {
	    to_producer(config, models, producers);
	  }
      }
    else if(configs.is_object() and configs.count("producers"))
      {
	to_producers(configs["producers"], models, producers);	
      }
    else
      {
	LOG_S(WARNING) << "could not interprete config: \n"
		       << configs.dump(2);
	return false;
      }
    
    return true;
  }
  
}

#endif
