//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_AUGMENT_CONFIG_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_AUGMENT_CONFIG_H_

namespace andromeda
{
  namespace glm
  {
    class augment_config
    {
    public:

      augment_config();
      augment_config(nlohmann::json config);

      void from_json(nlohmann::json& config);

      nlohmann::json to_json();

    private:

      nlohmann::json configuration;


    };

    augment_config::augment_config():
      configuration()
    {}

    augment_config::augment_config(nlohmann::json config):
      configuration(config)
    {}

      void augment_config::from_json(nlohmann::json& config)
    {
    }

    nlohmann::json augment_config::to_json()
    {
      nlohmann::json result;

      result["mode"] = to_string(AUGMENT);

      return result;
    }

  }

}

#endif
