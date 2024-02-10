//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_TOK_MODEL_H_
#define ANDROMEDA_MODELS_BASE_TOK_MODEL_H_

#include <sentencepiece_processor.h>

namespace andromeda
{
  class base_tok_model: public base_nlp_model
  {
  protected:

    typedef sentencepiece::SentencePieceProcessor tok_model_type;
    
  public:

    base_tok_model();
    virtual ~base_tok_model() {};

    /*   IO   */

    virtual bool load(std::filesystem::path ifile, bool verbose);
    virtual bool save(std::filesystem::path ofile);
    
    /*   INFERENCE   */
    
    virtual bool apply(std::string& text, nlohmann::json& annots) { return false; }

    virtual bool apply(subject<TEXT>& subj) { return false; }
    virtual bool apply(subject<TABLE>& subj) { return false; }

    virtual bool apply(subject<DOCUMENT>& subj) { return false; }
    
    /*  CONFIG   */

    virtual nlohmann::json create_train_config();

    /*   TRAIN   */
    virtual bool is_trainable() { return true; }
    virtual bool train(nlohmann::json config);

  protected:

    /*   PREDICT   */

    std::vector<int> encode(const std::string& text);
    std::string decode(const std::vector<int>& inds);

  private:

    bool parse_config(nlohmann::json config);

  protected:

    std::filesystem::path model_path;

    std::shared_ptr<tok_model_type> model;
  };

  base_tok_model::base_tok_model()
  {}

  bool base_tok_model::load(std::filesystem::path path, bool verbose)
  {
    if(not std::filesystem::exists(path))
      {
        LOG_S(ERROR) << "path does not exists: " << path;
        return false;
      }

    if(model.use_count()==0)
      {
        model = std::make_shared<tok_model_type>();
      }

    const auto status = model->Load(path.c_str());

    if(!status.ok())
      {
        LOG_S(ERROR) << status.ToString();
        return false;
      }

    return true;
  }

  bool base_tok_model::save(std::filesystem::path ofile)
  {
    return false;
  }

  nlohmann::json base_tok_model::create_train_config()
  {
    nlohmann::json config;
    return config;
  }

  bool base_tok_model::train(nlohmann::json config)
  {
    return false;
  }
  
  std::vector<int> base_tok_model::encode(const std::string& text)
  {
    std::vector<int> result={};

    if(model.use_count()==0)
      {
	LOG_S(WARNING) << "no model is loaded in base_tok_model";
        return result;
      }

    model->Encode(text, &result);
    //for (const int id : ids) {
    //std::cout << id << std::endl;
    //}

    return result;
  }

  std::string base_tok_model::decode(const std::vector<int>& inds)
  {
    std::string result="";

    if(model.use_count()==0)
      {
	LOG_S(WARNING) << "no model is loaded in base_tok_model";
        return result;
      }

    model->Decode(inds, &result);

    return result;
  }

}

#endif
