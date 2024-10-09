//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_EVALUATE_H_
#define ANDROMEDA_BASE_CRF_EVALUATE_H_

namespace andromeda_crf
{
  class evaluater
  {
  public:

    const static inline std::vector<std::string> header={"label", "true-count", "pred-count",
                                                         "from-model", "recall", "precision", "F1"};

    typedef andromeda_crf::utils::crf_token token_type;

    typedef andromeda_crf::crf_model model_type;

    typedef std::tuple<std::string, std::size_t, std::size_t,
                       bool, double, double, double> row_type;

  public:

    evaluater(std::filesystem::path model_file,
              std::string tokenizer_mode);

    evaluater(std::shared_ptr<model_type> model, bool verbose=false);
    
    ~evaluater();

    bool evaluate(std::filesystem::path validation_file,
		  std::filesystem::path metrics_file);

  private:

    bool initialise();

    std::string compute_metrics();
    void write_metrics(std::string ofile="evaluation.txt");

  private:

    bool verbose;
    
    std::string model_file;
    std::string tokenizer_mode;

    std::shared_ptr<model_type> model;

    std::map<int, std::string> to_label;
    std::map<std::string, int> to_index;

    std::map<std::string, bool> is_from_model;

    std::size_t perfect_preds, number_preds;
    
    std::vector<std::vector<std::size_t> > confusion_matrix;
    std::vector<row_type> metrics_table;
  };
  
  evaluater::evaluater(std::filesystem::path model_file,
                       std::string tokenizer_mode):
    verbose(false),

    // model_file(model_file),
    model_file(model_file.string()),
    tokenizer_mode(tokenizer_mode),

    model(NULL),

    to_label({}),
    to_index({}),

    is_from_model({}),

    perfect_preds(0),
    number_preds(0),

    confusion_matrix(),
    metrics_table()
  {
    model = std::make_shared<model_type>();
    // if(not model->load_from_file(model_file, false))
    if (not model->load_from_file(model_file.string(), false)) 
      {
        model = NULL;
        LOG_S(ERROR) << "could not read model from: " << model_file;
      }
  }

  evaluater::evaluater(std::shared_ptr<model_type> model, bool verbose):
    verbose(verbose),

    model_file("null"),
    tokenizer_mode("null"),
    
    model(model),

    to_label({}),
    to_index({}),

    is_from_model({}),

    perfect_preds(0),
    number_preds(0),
    
    confusion_matrix(),
    metrics_table()    
  {}
  
  evaluater::~evaluater()
  {}

  bool evaluater::initialise()
  {
    if(model==NULL)
      {
        return false;
      }

    to_label={};
    to_index={};

    is_from_model={};

    std::string first_header="true \\ pred";
    std::size_t flen = first_header.size();

    for(std::size_t i=0; i<model->num_classes(); i++)
      {
        to_label[i] = model->get_class_label(i);
        to_index[model->get_class_label(i)] = i;

        is_from_model[model->get_class_label(i)] = true;

        flen = std::max(flen, to_label.at(i).size());
      }

    confusion_matrix.resize(to_label.size());
    for(std::size_t i=0; i<model->num_classes(); i++)
      {
        confusion_matrix[i].resize(to_label.size(), 0);
      }

    return true;
  }

  bool evaluater::evaluate(std::filesystem::path validation_file,
			   std::filesystem::path metrics_file)
  {
    if(not initialise())
      {
        return false;
      }

    std::vector<std::vector<token_type> > samples={};
    
    if(validation_file.extension()==".txt")
      {
	std::ifstream ifs(validation_file.c_str());
	if(ifs.good())
	  {
	    trainer::read_tagged(&ifs, samples);
	  }
	else
	  {
	    LOG_S(ERROR) << "could not open file: " << validation_file;
	    samples.clear();
	  }
      }
    else if(validation_file.extension()==".jsonl")
      {
	std::vector<std::vector<token_type> > train={};

	std::ifstream ifs(validation_file.c_str());	
	if(ifs.good())
	  {
	    trainer::read_samples(ifs, train, samples);
	  }
      }
    else
      {
	LOG_S(WARNING) << "validation-file does not have extension `.txt` or `.jsonl`: "
		       << validation_file;
	return false;
      }

    perfect_preds = 0;
    number_preds = samples.size();
    
    predicter pred(model);
    for(auto& sample:samples)
      {
        for(auto& token:sample)
          {
            if(to_index.count(token.true_label)==0)
              {
		std::size_t i = to_label.size();

                to_label[i] = token.true_label;
                to_index[token.true_label] = i;

                is_from_model[token.true_label] = false;

                LOG_S(WARNING) << "model-labels and evaluation-labels out of sync: "
                               << token.true_label << " is not in model-classes";

                confusion_matrix.resize(to_label.size());
                for(std::size_t j=0; j<to_label.size(); j++)
                  {
                    confusion_matrix[j].resize(to_label.size(), 0);
                  }
              }
          }

        pred.predict(sample);

	if(verbose)
	  {
	    tabulate(sample);
	  }
	
	bool correct=true;
        for(auto& token:sample)
          {
            int i = to_index.at(token.true_label);
            int j = to_index.at(token.pred_label);

            confusion_matrix.at(i).at(j) += 1;

	    if(i!=j)
	      {
		correct = false;
	      }
          }

	perfect_preds += correct? 1:0;
      }

    // write_metrics(metrics_file);
    write_metrics(metrics_file.string());

    return true;
  }

  std::string evaluater::compute_metrics()
  {
    metrics_table.clear();

    std::size_t mlen=0, flen=header.at(0).size();
    for(auto head:header)
      {
        mlen = std::max(mlen, head.size());
      }

    for(auto& item:to_label)
      {
        flen = std::max(flen, item.second.size());

        int i = to_index.at(item.second);

        std::size_t diag_cnt=confusion_matrix.at(i).at(i);
        std::size_t true_cnt=0;
        std::size_t pred_cnt=0;

        for(std::size_t l=0; l<to_index.size(); l++)
          {
            true_cnt += confusion_matrix.at(i).at(l);
            pred_cnt += confusion_matrix.at(l).at(i);
          }

        double recall = double(diag_cnt)/(1.e-12+double(true_cnt));
        double prec   = double(diag_cnt)/(1.e-12+double(pred_cnt));
        double f1 = 2*recall*prec/(1.e-12+recall+prec);

        metrics_table.emplace_back(item.second, true_cnt, pred_cnt,
                                   is_from_model.at(item.second),
                                   recall, prec, f1);
      }

    std::sort(metrics_table.begin(), metrics_table.end(),
	      [](const row_type& lhs, const row_type& rhs)
	      {
		return std::get<1>(lhs)>std::get<1>(rhs);
	      });
    
    std::stringstream ss;

    ss << "%-perfect: " << double(perfect_preds)/double(number_preds)
       << " [" << perfect_preds << "/" << number_preds << "] \n\n";
    
    for(std::size_t l=0; l<header.size(); l++)
      {
        std::size_t len = l==0? flen:mlen;
        ss << std::setw(len) << header.at(l) << " | ";
      }
    ss << "\n";

    for(std::size_t l=0; l<header.size(); l++)
      {
        std::size_t len = l==0? flen:mlen;
        ss << std::setw(len) << std::string(len, '-') << " | ";
      }
    ss << "\n";

    for(auto& row:metrics_table)
      {
        ss << std::setw(flen) << std::get<0>(row) << " | "
           << std::setw(mlen) << std::get<1>(row) << " | "
           << std::setw(mlen) << std::get<2>(row) << " | "
           << std::setw(mlen) << std::get<3>(row) << " | "
           << std::setw(mlen) << std::get<4>(row) << " | "
           << std::setw(mlen) << std::get<5>(row) << " | "
           << std::setw(mlen) << std::get<6>(row) << " | "
           << "\n";
      }
    LOG_S(INFO) << "overview table: \n" << ss.str();

    return ss.str();
  }

  void evaluater::write_metrics(std::string ofile)
  {
    std::stringstream ss;

    std::string corner = "true \\ pred";

    std::size_t flen=corner.size(), mlen=4;
    for(auto& item:to_label)
      {
        flen = std::max(flen, item.second.size());
      }

    ss << std::setw(flen) << corner << " | ";
    for(auto& item:to_label)
      {
        ss << std::setw(mlen) << item.second.substr(0,mlen) << " | ";
      }
    ss << "\n";

    ss << std::setw(flen) << std::string(flen,'-') << " | ";
    for(std::size_t l=0; l<to_label.size(); l++)
      {
        ss << std::setw(mlen) << std::string(mlen,'-') << " | ";
      }
    ss << "\n";

    int rind=0;
    for(auto& row:confusion_matrix)
      {
        ss << std::setw(flen) << to_label.at(rind++) << " | ";
        for(auto& cnt:row)
          {
            std::string val = cnt==0? "-":std::to_string(cnt);
            ss << std::setw(mlen) << val << " | ";
          }
        ss << "\n";
      }

    {
      std::ofstream ofs(ofile.c_str());

      if(ofs.good())
        {
          ofs << "metrics-table: \n\n" << compute_metrics() << "\n\n";
          ofs << "confusion-matrix: \n\n" << ss.str() << "\n";
        }
      ofs.close();

      LOG_S(INFO) << "evaluation written in: " << ofile;
    }

  }

}

#endif
