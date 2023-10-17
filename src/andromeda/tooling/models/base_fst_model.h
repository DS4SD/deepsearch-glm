//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_FST_MODEL_H_
#define ANDROMEDA_MODELS_BASE_FST_MODEL_H_

#include <fasttext/args.h>
#include <fasttext/autotune.h>
#include <fasttext/fasttext.h>

namespace andromeda
{
  class base_fst_model: public base_nlp_model
  {
  public:
    
    typedef fasttext::Args ft_args_type;
    typedef fasttext::Autotune ft_autotune_type;
    
    typedef fasttext::FastText ft_model_type;
    
  public:

    base_fst_model();
    virtual ~base_fst_model() {};

    /*   IO   */    
    virtual bool load(std::filesystem::path ifile, bool verbose);
    virtual bool save(std::filesystem::path ofile);
    
    /*   TRAIN   */

  protected:
    
    std::shared_ptr<ft_model_type> model;
  };

  base_fst_model::base_fst_model():
    base_nlp_model(),    
    model(NULL)
  {}

  bool base_fst_model::load(std::filesystem::path ifile, bool verbose)
  {
    LOG_S(INFO) << __FILE__ << ":" << __LINE__;
    
    if(not std::filesystem::exists(ifile))
      {
	return false;
      }

    std::string model_name = ifile.string();

    if(verbose)
      {
	LOG_S(INFO) << "fasttext model load from " << model_name;
      }
    
    LOG_S(INFO) << __FILE__ << ":" << __LINE__ << " -> start loading ...";    
    model->loadModel(model_name);    
    
    return true;
  }

  bool base_fst_model::save(std::filesystem::path ofile)
  {
    LOG_S(INFO);
    
    std::string model_name = ofile.string();

    LOG_S(INFO) << "fasttext model save to " << model_name << ".bin";
    model->saveModel(model_name + ".bin");

    LOG_S(INFO) << "fasttext vectors save to " << model_name << ".vec";
    model->saveVectors(model_name + ".vec");

    LOG_S(INFO) << "fasttext output save to " << model_name << ".output";
    model->saveOutput(model_name + ".output");

    return true;
  }

}

#endif
