//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_TRAIN_H_
#define ANDROMEDA_BASE_CRF_TRAIN_H_

namespace andromeda_crf
{
  class trainer
  {
    typedef andromeda_crf::crf_model model_type;

    typedef andromeda_crf::utils::crf_token token_type;

  public:

    trainer();
    trainer(std::shared_ptr<model_type> model,
	    int epoch,
	    double gaussion);
      
    ~trainer();

    //    void train(std::string ifilename);

    void train(std::string train_file,
               std::string validation_file);

    static void read_tagged(std::istream* ifile, std::vector<std::vector<token_type> >& vs);

    static void read_samples(std::ifstream& ifile,
                             std::vector<std::vector<token_type> >& train_samples,
                             std::vector<std::vector<token_type> >& val_samples);

  private:

    bool read_samples(std::string train_filename,
		      std::string validate_filename);
    
  private:

    std::shared_ptr<model_type> model;

    int epoch, cutoff;
    double gaussian_sigma, widthfactor;

    std::vector<std::vector<token_type> > train_samples, val_samples;
  };

  trainer::trainer():
    model(NULL)
  {}

  trainer::trainer(std::shared_ptr<model_type> model,
                   int epoch,
                   double gaussian_sigma):
    model(model),

    epoch(epoch),
    cutoff(0),

    gaussian_sigma(gaussian_sigma),
    widthfactor(0.0),
    
    train_samples({}),
    val_samples({})
  {}

  trainer::~trainer()
  {}

  void trainer::train(std::string train_filename, std::string validate_filename)
  {
    if(not read_samples(train_filename, validate_filename))
      {
	return;
      }
    
    LOG_S(INFO) << "#-training samples: " << train_samples.size();
    LOG_S(INFO) << "#-validation samples: " << val_samples.size();
    
    //andromeda_crf::crf_train(model_type::PERCEPTRON, *model, train_samples, val_samples, 0.0);
    //andromeda_crf::crf_train(model_type::PERCEPTRON, *model, train_samples, val_samples, gaussian);

    for(auto itr = train_samples.begin(); itr!=train_samples.end(); itr++)
      {
        const std::vector<utils::crf_token>& s = *itr;

        utils::crf_state_sequence cs;
        for(std::size_t j=0; j<s.size(); j++)
          {
            cs.add_state(create_crfstate(s, j));
          }

        model->add_training_sample(cs);
      }

    for(auto itr = val_samples.begin(); itr!=val_samples.end(); itr++)
      {
        const std::vector<utils::crf_token>& s = *itr;

        utils::crf_state_sequence cs;
        for(std::size_t j=0; j<s.size(); j++)
          {
            cs.add_state(create_crfstate(s, j));
          }

        model->add_validation_sample(cs);
      }

    {
      model->train(model_type::PERCEPTRON, epoch, cutoff, gaussian_sigma, widthfactor);
    }
  }

  bool trainer::read_samples(std::string train_filename,
			     std::string validate_filename)
  {
    train_samples={};
    val_samples={};

    if(train_filename.ends_with(".txt") and
       validate_filename.ends_with(".txt") )
      {
        {
          std::ifstream ifs(train_filename.c_str());
          if(ifs.good())
            {
              read_tagged(&ifs, train_samples);
            }
          else
            {
              return false;
            }
        }

        {
          std::ifstream ifs(validate_filename.c_str());
          if(ifs.good())
            {
              read_tagged(&ifs, val_samples);
            }
          else
            {
              return false;
            }
        }
      }
    else if(train_filename.ends_with(".txt"))
      {
        std::ifstream ifs(train_filename.c_str());
        if(ifs.good())
          {
            read_tagged(&ifs, train_samples);
          }
        else
          {
            return false;
          }

        val_samples={};
      }
    else if(train_filename.ends_with(".jsonl"))
      {
        std::ifstream ifs(train_filename.c_str());
        if(ifs.good())
          {
            read_samples(ifs, train_samples, val_samples);
          }
        else
          {
            LOG_S(ERROR) << "could not open file: " << train_filename;
            return false;
          }
      }
    else
      {
        LOG_S(ERROR) << "train-filename " << train_filename << " does not end with `.txt` or `.jsonl`";
        return false;
      }

    return true;
  }

  /*
    void trainer::train(std::string train_filename,
    std::string validation_filename)
    {
    std::vector<std::vector<token_type> > train_samples={}, val_samples={};

    {
    std::ifstream ifs(train_filename.c_str());
    if(ifs.good())
    {
    read_tagged(&ifs, train_samples);
    }
    else
    {
    LOG_S(ERROR) << "could not open file: " << train_filename;
    train_samples.clear();
    }
    }

    if(std::filesystem::exists(validation_filename))
    {
    std::ifstream ifs(validation_filename.c_str());
    if(ifs.good())
    {
    read_tagged(&ifs, val_samples);
    }
    else
    {
    LOG_S(ERROR) << "could not open file: " << validation_filename;
    val_samples.clear();
    }
    }
    else
    {
    LOG_S(WARNING) << "no validation filename: " << validation_filename;
    val_samples.clear();
    }

    LOG_S(INFO) << "#-training samples: " << train_samples.size();
    LOG_S(INFO) << "#-validation samples: " << val_samples.size();

    andromeda_crf::crf_train(model_type::PERCEPTRON, *model, train_samples, val_samples, 0.0);
    }
  */

  void trainer::read_tagged(std::istream* ifile, std::vector<std::vector<token_type> >& vs)
  {
    andromeda_crf::utils::parenthesis_converter paren_converter;

    std::string line;
    while(std::getline(*ifile,line))
      {
        std::istringstream is(line);
        std::string s;

        std::vector<token_type> sentence;

        while (is >> s) {
          std::string::size_type i = s.find_last_of('/');

          std::string str = s.substr(0, i);
          std::string pos = s.substr(i+1);

          str = paren_converter.Ptb2Pos(str);

          token_type t(str, pos);
          sentence.push_back(t);
        }
        vs.push_back(sentence);
      }
  }

  void trainer::read_samples(std::ifstream& ifile,
                             std::vector<std::vector<token_type> >& train_samples,
                             std::vector<std::vector<token_type> >& val_samples)
  {
    std::string line;
    while(std::getline(ifile, line))
      {
        nlohmann::json sample = nlohmann::json::parse(line);

        //assert(sample.count("word-tokens")>0);
	//assert(sample.count(text_element::word_tokens_lbl)==1);
        assert(sample.count("training-sample")>0);
        //LOG_S(INFO) << sample.dump(2);

        auto& wtokens = sample.at(andromeda::text_element::word_tokens_lbl);

        std::vector<std::string> headers = {};
        headers = wtokens.value("headers", headers);

        /*
        //assert(headers.find("char_i")!=headers.end());
        //assert(headers.find("char_j")!=headers.end());
        //assert(headers.find("word")!=headers.end());
        //assert(headers.find("true-label")!=headers.end());

        std::size_t i_ind = std::distance(headers.begin(), headers.find("char_i"));
        std::size_t j_ind = std::distance(headers.begin(), headers.find("char_i"));
        std::size_t t_ind = std::distance(headers.begin(), headers.find("word"));
        std::size_t l_ind = std::distance(headers.begin(), headers.find("true-label"));
        */

        std::size_t i_ind=-1, j_ind=-1, t_ind=-1, l_ind=-1;
        for(std::size_t l=0; l<headers.size(); l++)
          {
            if(headers.at(l)=="char_i") { i_ind = l; }
            if(headers.at(l)=="char_j") { j_ind = l; }
            if(headers.at(l)=="word") { t_ind = l; }
            if(headers.at(l)=="true-label") { l_ind = l; }
          }

        assert(0<=i_ind and i_ind<headers.size());
        assert(0<=j_ind and j_ind<headers.size());
        assert(0<=t_ind and t_ind<headers.size());
        assert(0<=l_ind and l_ind<headers.size());

        std::vector<token_type> tokens={};
        for(std::size_t l=0; l<wtokens["data"].size(); l++)
          {
            assert(headers.size()==wtokens["data"][l].size());

            std::size_t char_i=-1, char_j=-1;
            std::string word, label;

            char_i = wtokens["data"][l][i_ind].get<std::size_t>();
            char_j = wtokens["data"][l][j_ind].get<std::size_t>();

            word = wtokens["data"][l][t_ind].get<std::string>();
            label = wtokens["data"][l][l_ind].get<std::string>();

            tokens.emplace_back(word, label, char_i, char_j);
          }

        bool training_sample = sample.value("training-sample", true);

        if(training_sample)
          {
            train_samples.push_back(tokens);
          }
        else
          {
            val_samples.push_back(tokens);
          }
      }

  }

}

#endif
