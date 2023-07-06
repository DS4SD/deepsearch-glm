//-*-C++-*-

#include <filesystem>

#include "libraries.h"
#include "andromeda.h"

bool read_config(std::string filename, nlohmann::json& config)
{
  std::ifstream ifs(filename.c_str());

  if(ifs.good())
    {
      ifs >> config;
      return true;
    }
  else
    {
      LOG_S(WARNING) << "configuration file does not exist: " << filename;
    }

  return true;
}

bool parse_arguments(int argc, char *argv[], nlohmann::json& config)
{
  cxxopts::Options options("glm", "GLM toolkit");

  options.add_options()
    ("m,mode", "mode [create-configs,create,augment,distill,train,predict,explore]",
     cxxopts::value<std::string>()->default_value("create-configs"))
    ("c,config", "config-file for the model",
     cxxopts::value<std::string>()->default_value("./config.json"))
    ("h,help", "print usage");

  auto result = options.parse(argc, argv);

  if(result.count("help")==1 or
     (result.count("help")==0 and result.count("mode")==0))
    {
      LOG_S(INFO) << options.help();
      return false;
    }  

  if(result.count("mode")==0)
    {
      LOG_S(WARNING) << "`mode` is a required and needs to be one of "
		     << "`create-configs`, `create`, `distill`, `query` or `explore`";
      return false;
    }
  
  std::string mode = result["mode"].as<std::string>();
  config["mode"] = mode;
  
  std::set<std::string> modes = {"create-configs","create",
				 "augment","distill",
				 "query","explore"};
  
  if(modes.count(mode)==0)
    {
      LOG_S(WARNING) << "mode `" << mode << "` needs to be one of "
		     << "`create-configs`, `create` , `distill`, `query` or `explore`";
      return false;
    }
  
  if(mode=="create-configs")
    {
      return true;
    }

  if(result.count("config")==0)
    {
      LOG_S(WARNING) << "`config` is required for `mode` of type "
		     << "`create`, `distill`, `query` or `explore`.";
      return false;
    }  

  std::string filename = result["config"].as<std::string>();
  if(not read_config(filename, config))
    {
      return false;
    }

  // overwrite the mode from the command-line ...
  config["mode"] = mode;

  if(mode==to_string(andromeda::glm::CREATE))
    {
    }
  else if(mode==to_string(andromeda::glm::DISTILL))
    {
    }
  else if(mode==to_string(andromeda::glm::AUGMENT))
    {
    }  
  else if(mode==to_string(andromeda::glm::QUERY))
    {
    }
  else if(mode==to_string(andromeda::glm::EXPLORE))
    {
    }
  else
    {
      LOG_S(WARNING) << "run with `-h` or -m <mode> -c <config-file>";
      return false;
    }
  
  return true;
}

int main(int argc, char *argv[])
{
  typedef andromeda::glm::model glm_model_type;

  loguru::init(argc, argv);

  nlohmann::json args;
  if(not parse_arguments(argc, argv, args))
    {
      return -1;
    }
  
  std::shared_ptr<glm_model_type> model = std::make_shared<glm_model_type>();

  if(args.count("mode")==0)
    {
      LOG_S(INFO) << "no `mode` detected: " << args.dump(2);
      return -1;
    }
  
  std::string mode = args["mode"].get<std::string>();
  LOG_S(INFO) << "mode: " << mode;
  
  if(mode==to_string(andromeda::glm::CREATE_CONFIGS))
    {
      nlohmann::json configs = andromeda::glm::get_configurations(model);      
      andromeda::glm::write_configs(configs, "./");
    }
  else if(mode==to_string(andromeda::glm::CREATE))
    {
      andromeda::glm::create_glm_model(args, model);
    }
  else if(mode==to_string(andromeda::glm::AUGMENT))
    {
      andromeda::glm::augment_glm_model(args, model);      
    }  
  else if(mode==to_string(andromeda::glm::DISTILL))
    {
      std::shared_ptr<glm_model_type> distilled_model=NULL;
      andromeda::glm::distill_glm_model(args, model, distilled_model);      
    }
  else if(mode==to_string(andromeda::glm::QUERY))
    {
      nlohmann::json result;
      andromeda::glm::query_glm_model(args, result, model);      
    }  
  else if(mode==to_string(andromeda::glm::EXPLORE))
    {
      andromeda::glm::explore_glm_model(args, model);      
    }    
  else
    {
      LOG_S(ERROR) << "no key of value: `create`, `distill`, `query` or `explore`";
    }
  
  return 0;
}
