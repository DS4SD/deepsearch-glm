//-*-C++-*-

#ifndef ANDROMEDA_MODELS_BASE_FST_MODEL_SUPERVISED_TRAINER_H_
#define ANDROMEDA_MODELS_BASE_FST_MODEL_SUPERVISED_TRAINER_H_

namespace andromeda
{

  class fasttext_supervised_model:
    public base_nlp_model, confusion_matrix_evaluator
  {
    typedef fasttext::Args ft_args_type;
    typedef fasttext::Autotune ft_autotune_type;
    
    typedef fasttext::FastText ft_model_type;
    
  public:

    fasttext_supervised_model();
    ~fasttext_supervised_model();

    template<typename subject_type>  
    bool get(subject_type& subj, base_property& prop);
    
    /*   IO   */    
    virtual bool load(std::filesystem::path ifile);
    virtual bool save(std::filesystem::path ofile);

    /*  CONFIG   */

    virtual nlohmann::json create_train_config();

    /* TRAIN */
    virtual bool is_trainable() { return true; }

    virtual bool prepare_data_for_train(nlohmann::json args,
					std::vector<std::shared_ptr<base_nlp_model> >& dep_models);

    virtual bool train(nlohmann::json args);

    virtual bool evaluate_model(nlohmann::json args,
				std::vector<std::shared_ptr<base_nlp_model> >& dep_models);
    
    /* PREDICT */
    
    virtual std::string preprocess(const std::string& orig);
    virtual bool classify(const std::string& orig, std::string& label, double& conf);

    virtual bool preprocess(const subject<TEXT>& subj, std::string& text);
    virtual bool preprocess(const subject<TABLE>& subj, std::string& text);

    bool classify(subject<TEXT>& subj);
    bool classify(subject<TABLE>& subj);

  protected:

    bool parse_config(nlohmann::json config);
    
    bool prepare_data(std::vector<std::shared_ptr<base_nlp_model> >& dep_models);
    bool read_samples(std::vector<std::shared_ptr<base_nlp_model> >& dep_models,
		      bool read_train, bool read_eval);

    bool launch_training();

    bool evaluate_training();
    
  protected:

    std::filesystem::path model_path;
    std::shared_ptr<ft_model_type> model;

  private:
    
    double learning_rate;
    int epoch, dim, ws, ngram;

    bool autotune;
    std::string modelsize;
    int duration; // in seconds

    std::set<std::string> explicit_hpo_parameters;
    std::set<std::string> explicit_train_parameters;
    
    std::string model_file, metrics_file;//, config_file;
    
    std::string data_file;//, train_file, validate_file, test_file;
    
    std::string fasttext_train_file,
      fasttext_validation_file;
    
    nlohmann::json train_args;
        
    std::vector<std::pair<std::string, std::string> > train_samples; // <label, text>
    std::vector<std::pair<std::string, std::string> > eval_samples;

    confusion_matrix_evaluator conf_matrix;
  };

  fasttext_supervised_model::fasttext_supervised_model():    
    base_nlp_model(),

    model_path("fst_classifier_model.bin"),
    model(NULL),
    
    learning_rate(0.1),
    epoch(5),
    dim(64),
    ws(3),
    ngram(0),

    autotune(false),
    modelsize("100M"),
    duration(360),

    explicit_hpo_parameters({}),
    explicit_train_parameters({}),
    
    model_file("<undefined>"),
    metrics_file("<undefined>"),
    //config_file("<undefined>"),

    data_file("<undefined>"),
    //train_file("<undefined>"),
    //validate_file("<undefined>"),
    //test_file("<undefined>"),
    
    fasttext_train_file("<fasttext_train_file>"),
    fasttext_validation_file("<fasttext_validation_file>"),

    train_args(nlohmann::json::object({}))    
  {
    //LOG_S(INFO) << __FILE__ << ":" << __LINE__;
  }

  fasttext_supervised_model::~fasttext_supervised_model()
  {}

  template<typename subject_type>  
  bool fasttext_supervised_model::get(subject_type& subj,
				      base_property& property)
  {
    for(auto& prop:subj.properties)
      {
	if(prop.get_type()==get_key())
	  {
	    property = prop; 
	    return true;
	  }
      }
    
    return false;
  }
  
  bool fasttext_supervised_model::load(std::filesystem::path ifile)
  {
    //LOG_S(INFO) << __FILE__ << ":" << __LINE__;
    
    std::string model_path = ifile.string();
    
    if(not std::filesystem::exists(ifile))
      {
	LOG_S(ERROR) << "file does not exists: " << model_path;
	return false;
      }

    if(model==NULL)
      {
	model = std::make_shared<ft_model_type>();
      }    

    model->loadModel(model_path);
    
    return true;
  }

  bool fasttext_supervised_model::save(std::filesystem::path ofile)
  {
    std::string model_name = ofile.string();

    if(model_name.ends_with(".bin"))
      {
	model_name = utils::replace(model_name, ".bin", "");
      }
    
    LOG_S(INFO) << "fasttext model save to " << model_name << ".bin";
    model->saveModel(model_name+".bin");
    	
    LOG_S(INFO) << "fasttext vectors save to " << model_name << ".vec";
    model->saveVectors(model_name+".vec");

    //LOG_S(INFO) << "fasttext vectors save to " << model_out;
    //model->saveOutput(model_name);
    
    return true;
  }
  
  nlohmann::json fasttext_supervised_model::create_train_config()
  {
    nlohmann::json config = nlohmann::json::object({});

    config["mode"] = "train";
    config["model"] = get_key();

    nlohmann::json hpo;
    {
      hpo["autotune"] = autotune;
      hpo["modelsize"] = modelsize;
      hpo["duration"] = duration;
    }
    
    nlohmann::json args;
    {
      args["mode"] = "supervised";

      args["learning-rate"] = learning_rate;
      args["epoch"] = epoch;

      args["dim"] = dim;
      args["ws"] = ws;

      args["n-gram"] = ngram;
    }

    nlohmann::json files;
    {
      files["data-file"] = "<filename>";
      //files["train-file"] = "<filename>";
      //files["validate-file"] = "<filename>";
      //files["test-file"] = "<filename>";

      files["model-file"] = "<filename>";
      files["metrics-file"] = "<filename>";
    }

    config["hpo"] = hpo;
    config["args"] = args;
    config["files"] = files;

    return config;
  }

  bool fasttext_supervised_model::parse_config(nlohmann::json config)
  {
    auto hpo_args = config["hpo"];
    auto train_args = config["args"];

    auto files = config["files"];    

    for(auto itr:hpo_args.items())
      {
	explicit_hpo_parameters.insert(itr.key());
      }

    for(auto itr:train_args.items())
      {
	explicit_train_parameters.insert(itr.key());
      }
    
    // HPO
    {
      autotune = hpo_args.value("autotune", autotune);
      
      modelsize = hpo_args.value("modelsize", modelsize);
      duration = hpo_args.value("duration", duration);
    }

    // parameters
    {
      learning_rate = train_args.value("learning-rate", learning_rate);
      epoch = train_args.value("epoch", epoch);
      dim = train_args.value("dim", dim);
      ws = train_args.value("ws", ws);
      ngram = train_args.value("n-gram", ngram);
    }
    
    // files
    {
      data_file = files.value("data-file", "null");
      
      //train_file = train_files.value("train-file", "null");
      //validate_file = train_files.value("validate-file", "null");
      //test_file = train_files.value("test-file", "null");
      
      model_file = files.value("model-file", "null");
      metrics_file = files.value("metrics-file", "null");    

      if(metrics_file=="null")
	{
	  metrics_file = model_file+".metrics.txt";
	}
      
      //config_file = model_file+".config.json";

      fasttext_train_file = data_file+".fasttext.train.txt";
      fasttext_validation_file = data_file+".fasttext.validate.txt";
    }

    return true;
  }

  bool fasttext_supervised_model::prepare_data_for_train(nlohmann::json config,
							 std::vector<std::shared_ptr<base_nlp_model> >& dep_models)
  {
    LOG_S(INFO) << "preparing data to train FastText classifier ...";

    parse_config(config);

    prepare_data(dep_models);
    
    return true;
  }
  
  bool fasttext_supervised_model::train(nlohmann::json config)
  {
    LOG_S(INFO) << "starting to train FastText classifier ...";
    
    parse_config(config);

    //if(not prepare_data())
    //{
    //LOG_S(WARNING) << "could not prepare the data for supervised Fasttext training ...";
    //}

    launch_training();

    save(model_file.c_str());    

    /*
    if(eval_samples.size()==0)
      {
	read_samples(dep_models);
      }
    
    evaluate_training();
    */
    
    return true;
  }

  bool fasttext_supervised_model::evaluate_model(nlohmann::json config,
						 std::vector<std::shared_ptr<base_nlp_model> >& dep_models)
  {
    parse_config(config);

    load(model_file.c_str());

    read_samples(dep_models, false, true);

    evaluate_training();

    return true;
  }
  
  std::string fasttext_supervised_model::preprocess(const std::string& orig)
  {
    return orig;
  }
  
  bool fasttext_supervised_model::preprocess(const subject<TEXT>& subj, std::string& text)
  {
    std::stringstream ss;

    std::size_t MAX = 256;
    std::size_t LEN = subj.get_num_wtokens();

    for(std::size_t l=0; l<std::min(LEN, MAX); l++)
      {
	const auto& token = subj.get_wtoken(l);	    
	auto tags = token.get_tags();
	
	if(tags.size()>0)
	  {
	    ss << "__" << *(tags.begin()) << "__";		
	  }
	else
	  {
	    std::string text = token.get_word();
	    text = utils::to_lower(text);
	    
	    ss << text;
	  }
	
	ss << " ";
      }
    
    text = ss.str();

    return true;
  }
  
  bool fasttext_supervised_model::preprocess(const subject<TABLE>& subj, std::string& text)
  {
    text = subj.get_text();
    return (text.size()>0);
  }

  bool fasttext_supervised_model::prepare_data(std::vector<std::shared_ptr<base_nlp_model> >& dep_models)
  {
    LOG_S(INFO) << __FUNCTION__;
    
    std::ifstream ifs(data_file.c_str());
    if(not ifs.good())
      {
	LOG_S(ERROR) << "could not read from file: " << data_file;
	return 0;
      }

    std::ofstream ofs_train(fasttext_train_file.c_str());
    if(not ofs_train.good())
      {
	LOG_S(ERROR) << "could not create fasttext train-file: " << fasttext_train_file;
	return 0;
      }

    std::ofstream ofs_eval(fasttext_validation_file.c_str());
    if(not ofs_eval.good())
      {
	LOG_S(ERROR) << "could not create fasttext eval-file: " << fasttext_validation_file;
	return 0;
      }    

    train_samples.clear();
    eval_samples.clear();    

    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    LOG_S(INFO) << "start reading from file: " << data_file;

    auto char_normaliser = text_element::create_char_normaliser(false);
    auto text_normaliser = text_element::create_text_normaliser(false);
    
    std::string line, orig="null", text="null", label="null";
    while(std::getline(ifs, line))
      {
	nlohmann::json item = nlohmann::json::parse(line);

	bool training_sample = bool(dis(gen)<0.9);
	if(item.count("training-sample"))
	  {
	    training_sample = item.at("training-sample").get<bool>();
	  }
	
	bool good = false;	
	if(item.count("label") and item.count("text"))
	  {
	    label = item.at("label").get<std::string>();
	    orig = item.at("text").get<std::string>();
	    
	    subject<TEXT> subj;
	    subj.set(orig, char_normaliser, text_normaliser);

	    for(auto dep_model:dep_models)
	      {
		dep_model->apply(subj);
	      }
	    
	    good = this->preprocess(subj, text);
	  }
	else if(item.count("label") and item.count("data"))
	  {

	  }
	else
	  {
	    LOG_S(WARNING) << "no `label` or `text` detected: aborting ...";
	    return false;
	  }

	if(not good)
	  {
	    continue;
	  }
	
	if(training_sample)
	  {
	    ofs_train << "__label__" << label << " " << text << "\n";
	    train_samples.push_back({label, text});
	  }
	else
	  {
	    ofs_eval << "__label__" << label << " " << text << "\n";
	    eval_samples.push_back({label, text});
	  }
      }

    LOG_S(INFO) << "read successfully: #-train: " << train_samples.size() << ", #-val: " << eval_samples.size();

    LOG_S(INFO) << "fasttext train-file: " << fasttext_train_file;
    LOG_S(INFO) << "fasttext validation-file: " << fasttext_validation_file; 

    return true;    
  }

  bool fasttext_supervised_model::read_samples(std::vector<std::shared_ptr<base_nlp_model> >& dep_models,
					       bool read_train, bool read_eval)
  {
    LOG_S(INFO) << __FUNCTION__;
    
    std::ifstream ifs(data_file.c_str());
    if(not ifs.good())
      {
	LOG_S(ERROR) << "could not read from file: " << data_file;
	return 0;
      }

    train_samples.clear();
    eval_samples.clear();    

    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    LOG_S(INFO) << "start reading from file: " << data_file;

    auto char_normaliser = text_element::create_char_normaliser(false);
    auto text_normaliser = text_element::create_text_normaliser(false);

    std::size_t cnt=0;
    std::string line, orig="null", text="null", label="null";
    while(std::getline(ifs, line))
      {
	std::cout << "\r\t#-lines: " << cnt++ << std::flush;
	
	nlohmann::json item = nlohmann::json::parse(line);

	bool training_sample = bool(dis(gen)<0.9);
	if(item.count("training-sample"))
	  {
	    training_sample = item.at("training-sample").get<bool>();
	  }

	if(training_sample and (not read_train))
	  {
	    continue;
	  }

	if((not training_sample) and (not read_eval))
	  {
	    continue;
	  }
	
	bool good = false;	
	if(item.count("label") and item.count("text"))
	  {
	    label = item.at("label").get<std::string>();
	    orig = item.at("text").get<std::string>();
	    
	    subject<TEXT> subj;
	    subj.set(orig, char_normaliser, text_normaliser);

	    for(auto dep_model:dep_models)
	      {
		dep_model->apply(subj);
	      }
		
	    good = this->preprocess(subj, text);
	  }
	else if(item.count("label") and item.count("data"))
	  {

	  }
	else
	  {
	    LOG_S(WARNING) << "no `label` or `text` detected: aborting ...";
	    return false;
	  }

	if(not good)
	  {
	    continue;
	  }
	
	if(training_sample)
	  {
	    train_samples.push_back({label, text});
	  }
	else
	  {
	    eval_samples.push_back({label, text});
	  }
      }
    
    LOG_S(INFO) << "read successfully: #-train: " << train_samples.size() << ", #-val: " << eval_samples.size();

    return true;        
  }
  
  bool fasttext_supervised_model::launch_training()
  {
    LOG_S(INFO) << __FUNCTION__;

    // -autotune-validation ./tmp/semantic-model/nlp-train-semantic.annot.jsonl.fasttext.validate.txt -autotune-duration 360 -autotune-modelsize 100M -dim 64 -wordNgrams 1
    
    std::vector<std::string> args_vec
      = {
	 "", "supervised",
	 
	 "-input", fasttext_train_file,
	 "-output", model_file//,

	 //"-autotune-validation", fasttext_validation_file,
	 //"-autotune-duration", std::to_string(autotune_duration),
	 //"-autotune-modelsize", autotune_modelsize,
	 
	 //"-lr", std::to_string(learning_rate),
	 //"-dim", std::to_string(dim),
	 //"-ws", std::to_string(ws),
	 //"-epoch", std::to_string(epoch),
	 //"-wordNgrams", std::to_string(ngram)
      };

    if(autotune)
      {
	args_vec.push_back("-autotune-validation");
	args_vec.push_back(fasttext_validation_file);

	if(explicit_hpo_parameters.count("duration"))
	  {
	    args_vec.push_back("-autotune-duration");
	    args_vec.push_back(std::to_string(duration));
	  }

	if(explicit_hpo_parameters.count("modelsize"))
	  {
	    args_vec.push_back("-autotune-modelsize");
	    args_vec.push_back(modelsize);
	  }
      }

    if(explicit_train_parameters.count("dim"))
      {
	args_vec.push_back("-dim");
	args_vec.push_back(std::to_string(dim));	
      }

    if(explicit_train_parameters.count("ws"))
      {
	args_vec.push_back("-ws");
	args_vec.push_back(std::to_string(ws));	
      }

    if(explicit_train_parameters.count("n-gram"))
      {
	args_vec.push_back("-wordNgrams");
	args_vec.push_back(std::to_string(ngram));	
      }    
    
    if(explicit_train_parameters.count("learning-rate"))
      {
	args_vec.push_back("-lr");
	args_vec.push_back(std::to_string(learning_rate));	
      }

    if(explicit_train_parameters.count("epoch"))
      {
	args_vec.push_back("-epoch");
	args_vec.push_back(std::to_string(dim));	
      }    
    
    if(model==NULL)
      {
	model = std::make_shared<ft_model_type>();
      }

    {
      std::stringstream ss;
      ss << "fasttext ";
      for(auto _ : args_vec)
	{
	  ss << _ << " ";
	}

      //LOG_S(INFO) << "training with command:\n" << ss.str(); 
    }
    
    ft_args_type ft_args;
    ft_args.parseArgs(args_vec);
    
    if(ft_args.hasAutotune())
      {
	//LOG_S(INFO) << "start HPO autotuning ... ";
	
	ft_autotune_type autotune(model);
	autotune.train(ft_args);
      }
    else
      {
	model->train(ft_args);	
      }

    return true;
  }

 bool fasttext_supervised_model::evaluate_training()
  {
    LOG_S(INFO) << __FUNCTION__  << "\t #-eval: " << eval_samples.size();
    
    conf_matrix.clear();
    for(auto& sample:eval_samples)
      {
	conf_matrix.update(sample.first);
      }

    std::string true_label="null";
    std::string pred_label="null";

    double conf=-1;
    for(auto& sample:eval_samples)
      {
	true_label = sample.first;
	
	this->classify(sample.second, pred_label, conf);

	conf_matrix.update(true_label, pred_label);

	if(true_label!=pred_label)
	 {
	   if(sample.second.size()<64)
	     {
	       LOG_S(INFO) << "true: " << std::setw(16) << true_label << "; "
			   << "pred [" << int(100*conf) << "]: " << std::setw(16) << pred_label
			   << " => text: " << sample.second;
	     }
	   else
	     {
	       LOG_S(INFO) << "true: " << std::setw(16) << true_label << "; "
			   << "pred [" << int(100*conf) << "]: " << std::setw(16) << pred_label
			   << " => text: " << sample.second.substr(0,64);
	     }
	 }
      }

    conf_matrix.compute();

    LOG_S(INFO) << "metrics: \n" << conf_matrix.tabulate_metrics();
    LOG_S(INFO) << "confusion-matrix: \n" << conf_matrix.tabulate_confusion();

    std::ofstream ofs(metrics_file.c_str());
    if(ofs.good())
      {
	ofs << "metrics: \n" << conf_matrix.tabulate_metrics() << "\n"
	    << "confusion-matrix: \n" << conf_matrix.tabulate_confusion() << "\n";
	ofs.close();

	LOG_S(INFO) << "written metrics to " << metrics_file;
      }
    
    return true;
  }

  bool fasttext_supervised_model::classify(const std::string& orig, std::string& label, double& conf)
  {
    if(model==NULL)
      {
	LOG_S(ERROR) << "no model is loaded ...";
	return false;
      }

    std::string text = preprocess(orig);
    
    std::istringstream iss(text);
    
    std::vector<std::pair<fasttext::real, std::string> > predictions;
    model->predictLine(iss, predictions, 5, 1.e-6);

    std::string name("__label__");
    std::size_t ind = name.size();

    for(auto& pred:predictions)
      {
        conf = pred.first;
	
	std::size_t len = pred.second.size();	  
	label = pred.second.substr(ind, len-ind);

	return true;
      }

    return false;
  }

  bool fasttext_supervised_model::classify(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    std::string text;

    std::string label="null";
    double conf=0.0;

    if(preprocess(subj, text) and classify(text, label, conf))
      {
	//std::string key = get_key();	

	subj.properties.emplace_back(subj.get_hash(), TEXT, "#", get_name(), label, conf);
	subj.applied_models.insert(get_key());
      }
    
    return update_applied_models(subj);    
  }

  bool fasttext_supervised_model::classify(subject<TABLE>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
	return false;
      }

    std::string text;

    std::string label="null";
    double conf=0.0;

    if(preprocess(subj, text) and classify(text, label, conf))
      {
	//std::string key = get_key();	

	subj.properties.emplace_back(subj.get_hash(), TABLE, "#", get_name(), label, conf);
	subj.applied_models.insert(get_key());
      }
    
    return update_applied_models(subj);    
  }      
  
}

#endif
