//-*-C++-*-

#ifndef ANDROMEDA_MODELS_UTILS_CONFUSION_MATRIX_EVALUATOR_H_
#define ANDROMEDA_MODELS_UTILS_CONFUSION_MATRIX_EVALUATOR_H_

namespace andromeda
{
  class confusion_matrix_evaluator
  {
  public:

    const static inline std::vector<std::string> header={"label", "true-count", "pred-count",
                                                         "from-model", "recall", "precision", "F1"};

    typedef std::tuple<std::string, std::size_t, std::size_t,
                       bool, double, double, double> row_type;

  public:

    confusion_matrix_evaluator();
    ~confusion_matrix_evaluator();

    void clear();

    bool update(std::string label);
    
    bool update(std::string true_label,
		std::string pred_label);

    void update(std::vector<std::string>& true_labels,
		std::vector<std::string>& pred_labels);

    void compute();

    std::string tabulate_metrics();
    std::string tabulate_confusion();

  private:

    std::map<std::size_t, std::string> to_label;
    std::map<std::string, std::size_t> to_index;

    std::map<std::string, bool> is_from_model;

    std::size_t perfect_preds, number_preds;

    std::vector<std::vector<std::size_t> > confusion_matrix;
    std::vector<row_type> metrics_table;
  };

  confusion_matrix_evaluator::confusion_matrix_evaluator():
    to_label({}),
    to_index({}),

    is_from_model({}),
    perfect_preds(0),
    number_preds(0),

    confusion_matrix(),
    metrics_table()
  {}

  confusion_matrix_evaluator::~confusion_matrix_evaluator()
  {}

  void confusion_matrix_evaluator::clear()
  {
    to_label.clear();
    to_index.clear();

    is_from_model.clear();
    perfect_preds = 0;
    number_preds = 0;

    confusion_matrix.clear();
    metrics_table.clear();
  }

  bool confusion_matrix_evaluator::update(std::string label)
  {
    if(to_index.count(label)==0)
      {
	std::size_t i = to_label.size();

	to_label[i] = label;
	to_index[label] = i;

	is_from_model[label] = true;
	
	confusion_matrix.resize(to_label.size());
	for(std::size_t i=0; i<to_label.size(); i++)
	  {
	    confusion_matrix[i].resize(to_label.size(), 0);
	  }

	return true;
      }

    return false;
  }
  
  bool confusion_matrix_evaluator::update(std::string true_label,
					  std::string pred_label)
  {
    if(to_index.count(pred_label)==0)
      {
	LOG_S(WARNING) << "evaluation-labels out of sync: "
		       << pred_label << " is not in model-classes";
	
	return false;
      }
    
    if(to_index.count(true_label)==0)
      {
	std::size_t i = to_label.size();

	to_label[i] = true_label;
	to_index[true_label] = i;

	is_from_model[true_label] = false;

	LOG_S(WARNING) << "model-labels and evaluation-labels out of sync: "
		       << true_label << " is not in model-classes";

	confusion_matrix.resize(to_label.size());
	for(std::size_t i=0; i<to_label.size(); i++)
	  {
	    confusion_matrix[i].resize(to_label.size(), 0);
	  }
      }

    std::size_t i = to_index.at(true_label);
    std::size_t j = to_index.at(pred_label);

    confusion_matrix.at(i).at(j) += 1;

    perfect_preds += (i==j)? 1:0;
    number_preds += 1;

    return true;
  }

  void confusion_matrix_evaluator::compute()
  {
    //LOG_S(INFO) << __FUNCTION__ << "\t" << confusion_matrix.size();

    //for(auto row:confusion_matrix)
    //{
    //LOG_S(INFO) << row.size();
    //}
    
    metrics_table.clear();

    //std::size_t flen=0;
    for(auto& item:to_label)
      {
        //flen = std::max(flen, item.second.size());
	
        std::size_t i = to_index.at(item.second);
	
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
  }

  std::string confusion_matrix_evaluator::tabulate_metrics()
  {
    std::size_t mlen=0, flen=header.at(0).size();
    for(auto head:header)
      {
        mlen = std::max(mlen, head.size());
      }

    for(auto& item:to_label)
      {
        flen = std::max(flen, item.second.size());
      }

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
    //LOG_S(INFO) << "overview table: \n" << ss.str();

    return ss.str();
  }

  std::string confusion_matrix_evaluator::tabulate_confusion()
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

    std::size_t rind=0;
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

    return ss.str();
  }
  
}

#endif
