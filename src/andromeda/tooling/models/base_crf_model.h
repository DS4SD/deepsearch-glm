//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_CRF_MODEL_H_
#define ANDROMEDA_MODELS_BASE_CRF_MODEL_H_

#include <andromeda/tooling/models/base_crf_model/structures.h>
#include <andromeda/tooling/models/base_crf_model/algorithms.h>

namespace andromeda
{
  class base_crf_model: public base_nlp_model
  {
  public:

    typedef andromeda_crf::utils::crf_token crf_token_type;
    typedef andromeda_crf::crf_model crf_model_type;

  public:

    base_crf_model();
    virtual ~base_crf_model() {};

    /*   IO   */

    virtual bool load(std::filesystem::path ifile, bool verbose);
    virtual bool save(std::filesystem::path ofile);

    /*  CONFIG   */

    virtual nlohmann::json create_train_config();

    /*   TRAIN   */
    virtual bool is_trainable() { return true; }
    virtual bool train(nlohmann::json config);

    /*   EVAL   */

    virtual bool evaluate(nlohmann::json args);

  protected:

    /*   PREDICT   */

    bool predict(std::vector<crf_token_type>& tokens);

  private:

    bool parse_config(nlohmann::json config);

  protected:

    std::filesystem::path model_path;
    std::shared_ptr<crf_model_type> model;

  private:

    int epoch;
    double gaussian_sigma;

    std::string model_file, metrics_file, config_file;

    std::string train_file, validate_file, test_file;

    andromeda_crf::predicter predicter;
  };

  base_crf_model::base_crf_model():
    model(NULL),

    epoch(20),
    gaussian_sigma(2.0),

    model_file("<undefined>"),
    metrics_file("<undefined>"),
    config_file("<undefined>"),

    train_file("<undefined>"),
    validate_file("<undefined>"),
    test_file("<undefined>"),

    predicter()
  {}

  bool base_crf_model::load(std::filesystem::path ifile, bool verbose)
  {
    if(model==NULL)
      {
        model = std::make_shared<crf_model_type>();
      }

    bool success = model->load_from_file(ifile, verbose);

    if(success)
      {
        predicter.set_model(model);
      }

    return success;
  }

  bool base_crf_model::save(std::filesystem::path ofile)
  {
    if(model!=NULL)
      {
        return model->save_to_file(ofile);
      }

    return false;
  }

  bool base_crf_model::predict(std::vector<crf_token_type>& tokens)
  {
    predicter.predict(tokens);
    return true;
  }

  nlohmann::json base_crf_model::create_train_config()
  {
    nlohmann::json config;

    config["mode"] = "train";
    config["model"] = get_key();

    nlohmann::json args;
    {
      args["epoch"] = epoch;
      args["gaussian-sigma"] = gaussian_sigma;
    }

    nlohmann::json files;
    {
      files["train-file"] = "<filename>";
      files["validate-file"] = "<optional:filename>";
      files["test-file"] = "<optional:filename>";

      files["model-file"] = "<filename>";
      files["metrics-file"] = "<optional:filename>";
    }

    config["args"] = args;
    config["files"] = files;

    return config;
  }

  bool base_crf_model::parse_config(nlohmann::json config)
  {
    LOG_S(INFO) << __FUNCTION__;

    nlohmann::json args = config["args"];
    {
      epoch = args.value("epoch", epoch);
      gaussian_sigma = args.value("gaussian-sigma", gaussian_sigma);
    }

    nlohmann::json files = config["files"];
    {
      train_file = files.value("train-file", "null");
      validate_file = files.value("validate-file", "null");
      test_file = files.value("test-file", "null");

      model_file = files.value("model-file", "null");
      metrics_file = files.value("metrics-file", "null");
    }

    if(validate_file=="null" and train_file.ends_with(".jsonl"))
      {
        validate_file = train_file;
      }

    if(test_file=="null" and train_file.ends_with(".jsonl"))
      {
        test_file = train_file;
      }

    if(not model_file.ends_with(".bin"))
      {
        model_file += ".bin";
      }

    if(metrics_file=="null")
      {
        metrics_file = model_file.substr(0, model_file.size()-4) + ".metrics.txt";
      }
    else if(not metrics_file.ends_with(".txt"))
      {
        metrics_file += ".txt";
      }

    return true;
  }

  bool base_crf_model::train(nlohmann::json config)
  {
    LOG_S(INFO) << "starting to train CRF ...";

    parse_config(config);

    model = std::make_shared<andromeda_crf::crf_model>();

    andromeda_crf::trainer trainer(model, epoch, gaussian_sigma);
    trainer.train(train_file, validate_file);

    model->save_to_file(model_file);

    if(std::filesystem::exists(test_file))
      {
        andromeda_crf::evaluater evaluater(model);
        evaluater.evaluate(test_file, metrics_file);
      }
    else if(train_file.ends_with(".jsonl"))
      {
        andromeda_crf::evaluater evaluater(model);
        evaluater.evaluate(train_file, metrics_file);
      }

    return true;
  }

  bool base_crf_model::evaluate(nlohmann::json config)
  {
    LOG_S(INFO) << "starting to evaluate CRF ...";

    parse_config(config);

    model = std::make_shared<andromeda_crf::crf_model>();

    model->load_from_file(model_file, false);

    if(std::filesystem::exists(test_file))
      {
        andromeda_crf::evaluater evaluater(model);
        evaluater.evaluate(test_file, metrics_file);
      }
    else if(train_file.ends_with(".jsonl"))
      {
        andromeda_crf::evaluater evaluater(model);
        evaluater.evaluate(train_file, metrics_file);
      }

    return true;
  }

}

#endif
