//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_PREDICT_H_
#define ANDROMEDA_BASE_CRF_PREDICT_H_

namespace andromeda_crf
{
  class predicter
  {
  public:

    const static inline std::size_t MAX_SEQUENCE_LEN = 990;
    const static inline double PROB_OUTPUT_THRESHOLD = 0.001;

    typedef andromeda_crf::utils::crf_token token_type;

    typedef andromeda_crf::crf_model model_type;

  public:

    predicter();
    
    predicter(std::filesystem::path model_file,
              std::string tokenizer_mode, bool verbose);

    predicter(std::shared_ptr<model_type> model);

    ~predicter();

    void set_model(std::shared_ptr<model_type> model);
    
    void interactive();

    void predict(std::string& line, std::vector<token_type>& vt);

    void predict(std::vector<token_type>& vt);

  private:

    std::string model_file;
    std::string tokenizer_mode;

    std::shared_ptr<model_type> model;
  };

  predicter::predicter():
    model_file("undefined"),
    tokenizer_mode("undefined"),

    model(NULL)
  {}
  
  predicter::predicter(std::filesystem::path model_file,
                       std::string tokenizer_mode, bool verbose):
    // model_file(model_file),
    model_file(model_file.string()),
    tokenizer_mode(tokenizer_mode),

    model(NULL)
  {
    model = std::make_shared<model_type>();
    if(not (model->load_from_file(model_file.string(), verbose)))
      {
        model = NULL;
      }
  }

  predicter::predicter(std::shared_ptr<model_type> model):

    model_file("undefined"),
    tokenizer_mode("default"),

    model(model)
  {}

  predicter::~predicter()
  {}

  void predicter::set_model(std::shared_ptr<model_type> model)
  {
    this->model = model;
  }

  void predicter::interactive()
  {
    while(true)
      {
        std::cout << "input: ";

        std::string line;
        std::getline(std::cin, line);

        if(line=="quit")
          {
            break;
          }

        std::vector<token_type> vt={};
        predict(line, vt);

        tabulate(vt);
      }
  }

  void predicter::predict(std::string& line, std::vector<token_type>& vt)
  {
    vt.clear();

    if(tokenizer_mode=="default")
      {
        LOG_S(WARNING) << "default tokenization";
        andromeda_crf::tokenize(line, vt, true);
      }
    else if(tokenizer_mode=="space")
      {
        LOG_S(WARNING) << "space tokenization";
        andromeda_crf::tokenize(line, vt, false);
      }
    else
      {
        LOG_S(WARNING) << "no valid tokenizer-mode specified: " << tokenizer_mode;
      }

    if(vt.size() == 0)
      {
        LOG_S(WARNING) << "no tokens ...";
        return;
      }
    else if(vt.size() > MAX_SEQUENCE_LEN)
      {
        LOG_S(WARNING) << "too many tokens [" << vt.size() << "]: "
                       << "truncating to " << MAX_SEQUENCE_LEN;

        while(vt.size()>MAX_SEQUENCE_LEN)
          {
            vt.pop_back();
          }
      }
    else
      {}

    //tabulate(vt);

    predict(vt);
  }

  void predicter::predict(std::vector<token_type>& vt)
  {
    std::vector<std::string> org_strs;
    {
      andromeda_crf::utils::parenthesis_converter paren_converter;

      for(auto i = vt.begin(); i != vt.end(); i++)
        {
          org_strs.push_back(i->text);

          i->text = paren_converter.Ptb2Pos(i->text);
          i->pred_label = "?";
        }
    }

    // tag the words
    std::vector<std::map<std::string, double> > tagp0, tagp1;
    {
      //    crf_decode_forward_backward(vt, crfm, tagp0);
      crf_decode_lookahead(vt, *model, tagp0);
    }
    
    for(auto i = vt.begin(); i != vt.end(); i++)
      {
	std::map<std::string, double> dummy;
	tagp1.push_back(dummy);
      }

    // merge the outputs (simple interpolation of probabilities)
    std::vector< std::map<std::string, double> > tagp; // merged

    for(std::size_t i = 0; i < vt.size(); i++)
      {
	const std::map<std::string, double> & crf = tagp0[i];
	const std::map<std::string, double> & ef  = tagp1[i];

	std::map<std::string, double> m, m2; // merged

	double sum = 0;
	for(auto j = crf.begin(); j != crf.end(); j++)
	  {
	    m.insert(std::pair<std::string, double>(j->first, j->second));
	    sum += j->second;
	  }

      for(auto j = ef.begin(); j != ef.end(); j++)
	{
	  sum += j->second;
	  
	  if (m.find(j->first) == m.end())
	    {
	      m.insert(std::pair<std::string, double>(j->first, j->second));
	    }
	  else
	    {
	      m[j->first] += j->second;
	    }
	}

      const double th = PROB_OUTPUT_THRESHOLD * sum;
      
      for(auto j = m.begin(); j != m.end(); j++)
	{
	  if(j->second >= th)
	    {
	      m2.insert(*j);
	    }
	}

      double maxp = -1;
      std::string maxtag;
      for(auto j = m2.begin(); j != m2.end(); j++)
	{
	  const double p = j->second;
	  
	  if(p > maxp)
	    {
	      maxp = p;
	      maxtag = j->first;
	    }
	}

      tagp.push_back(m2);

      vt[i].pred_conf = maxp;
      vt[i].pred_label = maxtag;
    }

  }

  //}

}

#endif
