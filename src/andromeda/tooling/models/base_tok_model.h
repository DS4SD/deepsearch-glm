//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_TOK_MODEL_H_
#define ANDROMEDA_MODELS_BASE_TOK_MODEL_H_

#include <sentencepiece_processor.h>
#include <sentencepiece_trainer.h>

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

    /*   CONTROL   */

    // returns the size of vocabs.
    int get_num_tokens() { assert(model.use_count()>0); return model->GetPieceSize(); }

    // returns the vocab id of "foo"
    int to_ind(std::string tok) { return model->PieceToId(tok); }

    // returns the string representation of id 10.
    std::string to_token(int ind) { return model->IdToPiece(ind);}

    // returns true if the given id is an unknown token. e.g., <unk>
    bool is_unknown(int ind) { return model->IsUnknown(ind); }

    // returns true if the given id is a control token. e.g., <s>, </s>
    bool is_control(int ind) { return model->IsControl(ind); }

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
    nlohmann::json config = nlohmann::json::object({});
    {
      config["min-log-level"] = 2;

      config["model-name"] = "<name>";
      config["model-type"] = "<default:unigram, bpe, word or char>";

      config["vocab-size"] = "<int:32000>";
      config["input-file"] = "<text.txt>";

      config["character-coverage"] = 0.9995;
      config["number-of-threads"] = 1;

      config["max-sentencepiece-length"] = 16;
      config["max-sentence-length"] = 4096;

      config["split-by-number"] = true;
      config["split-digits"] = true;

      config["control-symbols"] = nlohmann::json::array({});
      config["user-symbols"] = nlohmann::json::array({});
    }

    return config;
  }

  /*
   * For a full list with options, look into 
   * `https://github.com/google/sentencepiece/blob/master/doc/options.md`
   */
  bool base_tok_model::train(nlohmann::json config)
  {
    std::string model_name = config["model-name"].get<std::string>();
    std::size_t vocab_size = config["vocab-size"].get<std::size_t>();
    std::string input_file = config["input-file"].get<std::string>();

    std::stringstream ss;
    ss << "--model_prefix=" << model_name
       << "--vocab_size="   << vocab_size
       << "--input="        << input_file;

    if(config.count("model-type"))
      {
        ss << "--model_type=" << config.value("model-type", "unigram");
      }

    if(config.count("min-log-level"))
      {
        ss << "--minloglevel=" << config.value("min-log-level", 2);
      }

    if(config.count("character-coverage"))
      {
        ss << "--character_coverage=" << config.value("character-coverage", 0.9995);
      }


    if(config.count("number-of-threads"))
      {
        ss << "--num_threads=" << config.value("number-of-threads", 1);
      }

    if(config.count("max-sentencepiece-length"))
      {
        ss << "--max_sentencepiece_length=" << config.value("max-sentencepiece-length", 16);
      }

    if(config.count("max-sentence-length"))
      {
        ss << "--max_sentence_length=" << config.value("max-sentence-length", 4096);
      }

    if(config.count("split-by-number"))
      {
        ss << "--split_by_number" << config.value("split-by-number", false);
      }

    if(config.count("split-digits"))
      {
        ss << "--split_digits" << config.value("split-digits", true);
      }

    if(config.count("control-symbols"))
      {
	std::vector<std::string> syms = {};
	syms = config.value("control-symbols", syms);

	if(syms.size()>0)
	  {
	    ss << "--control_symbols=";

	    for(int l=0; l<syms.size(); l++)
	      {
		ss << syms.at(l);

		if(l+1<syms.size())
		  {
		    ss << ",";
		  }
	      }
	  }
      }
    
    if(config.count("user-symbols"))
      {
	std::vector<std::string> syms = {};
	syms = config.value("user-symbols", syms);

	if(syms.size()>0)
	  {
	    ss << "--user_defined_symbols=";
	    
	    for(int l=0; l<syms.size(); l++)
	      {
		ss << syms.at(l);

		if(l+1<syms.size())
		  {
		    ss << ",";
		  }
	      }
	  }
      }

    {
      std::string cmd = ss.str();
      LOG_S(INFO) << "start training with cmd = " << cmd;

      sentencepiece::SentencePieceTrainer::Train(cmd);
    }

    return true;
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

  /*
    std::string base_tok_model::decode(int ind)
    {
    std::string result="";

    if(model.use_count()==0)
    {
    LOG_S(WARNING) << "no model is loaded in base_tok_model";
    return result;
    }

    std::vector<int> inds = {ind};
    model->Decode(inds, &result);

    return result;
    }
  */

}

#endif
