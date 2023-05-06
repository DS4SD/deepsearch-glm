//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_MODEL_H_
#define ANDROMEDA_BASE_CRF_MODEL_H_

#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdio>

namespace andromeda_crf
{
  class crf_model
  {
  public:

    typedef typename utils::crf_feature::feature_body_type feature_body_type;

    enum OptimizationMethod { BFGS, PERCEPTRON, SGD };

    const static inline int MAX_LABEL_TYPES = utils::crf_feature::MAX_LABEL_TYPES;
    const static inline int MAX_LEN = 1000;

    const static inline std::string BOS_LABEL = "!BOS!";
    const static inline std::string EOS_LABEL = "!EOS!";

    const static inline bool USE_BOS_EOS = false;
    const static inline bool OUTPUT_MARGINAL_PROB = true;
    const static inline bool USE_EDGE_TRIGRAMS = false;

    const static inline int PERCEPTRON_NITER = 20;
    const static inline int LOOKAHEAD_DEPTH = 2;

    const static inline double PERCEPTRON_MARGIN = 40;
    const static inline int HV_OFFSET = 3;

  public:

    crf_model();
    ~crf_model();

    void incr_line_counter() { _line_counter++; }

    std::size_t num_classes() const { return _num_classes; }

    std::string get_class_label(int i) const { return _label_bag.Str(i); }

    int get_class_id(const std::string & s) const { return _label_bag.Id(s); }

    void add_training_sample(const utils::crf_state_sequence& s);

    void add_validation_sample(const utils::crf_state_sequence& s);

    int train(OptimizationMethod method, const int epochs, const int cutoff = 0,
              const double sigma = 0, const double widthfactor = 0);

    //  std::vector<double> classify(utils::crf_state & s) const;

    void decode_forward_backward(utils::crf_state_sequence & s,
                                 std::vector<std::map<std::string, double> > & tagp);

    void decode_viterbi(utils::crf_state_sequence & s);

    void decode_nbest(utils::crf_state_sequence & s0,
                      std::vector<std::pair<double, std::vector<std::string> > > & nbest,
                      const int num, const double min_prob);

    void decode_lookahead(utils::crf_state_sequence & s0);

    bool load_from_file(const std::string& filename, bool verbose);

    bool save_to_file(const std::string& filename, const double t = 0) const;

    void get_features(std::list< std::pair< std::pair<std::string, std::string>, double> > & fl);

  private:

    /*********************
     ***  functions  ***
     *********************/

    void convert(const utils::crf_state_sequence& seq,
                 utils::crf_sample_sequence& sample);

    double heldout_likelihood();

    double heldout_lookahead_error();

    double forward_backward(const utils::crf_sample_sequence& s);

    double viterbi(const utils::crf_sample_sequence& seq,
                   std::vector<int>& best_seq);

    void initialize_edge_weights();

    void initialize_state_weights(const utils::crf_sample_sequence& seq);

    //  void lookahead_initialize_edge_weights();

    void lookahead_initialize_state_weights(const utils::crf_sample_sequence& seq);

    int make_feature_bag(const int cutoff);

    //  int classify(const Sample & nbs, std::vector<double> & membp) const;

    double update_model_expectation();

    double add_sample_model_expectation(const utils::crf_sample_sequence& seq,
                                        std::vector<double>& vme, int& ncorrect);

    void add_sample_empirical_expectation(const utils::crf_sample_sequence& seq,
                                          std::vector<double>& vee);

    int perform_BFGS();

    int perform_AveragedPerceptron();

    int perform_StochasticGradientDescent();

    int perform_LookaheadTraining(int epochs);

    double lookahead_search(const utils::crf_sample_sequence& seq,
                            std::vector<int>& history,
                            const int start,
                            const int max_depth,  const int depth,
                            double current_score,
                            std::vector<int>& best_seq,
                            const bool follow_gold = false,
                            const std::vector<int> *forbidden_seq = NULL);

    void calc_diff(const double val,
                   const utils::crf_sample_sequence& seq,
                   const int start,
                   const std::vector<int>& history,
                   const int depth, const int max_depth,
                   std::map<int, double>& diff);

    int update_weights_sub(const utils::crf_sample_sequence& seq,
                           std::vector<int>& history,
                           const int x,
                           std::map<int, double>& diff);

    int update_weights_sub2(const utils::crf_sample_sequence& seq,
                            std::vector<int>& history,
                            const int x,
                            std::map<int, double>& diff);

    int update_weights_sub3(const utils::crf_sample_sequence& seq,
                            std::vector<int>& history,
                            const int x,
                            std::map<int, double>& diff);

    int lookaheadtrain_sentence(const utils::crf_sample_sequence& seq,
                                int& t, std::vector<double>& wa);

    int decode_lookahead_sentence(const utils::crf_sample_sequence& seq,
                                  std::vector<int>& vs);

    void init_feature2mef();

    double calc_loglikelihood(const utils::crf_sample_sequence& seq);

    //  std::vector<double> calc_state_weight(const utils::crf_sample_sequence & seq, const int i) const;

    std::vector<double> calc_state_weight(const int i);// const;

    void nbest_search(const double lb, const int len,
                      const int x, const int y, const double rhs_score,
                      std::vector<utils::crf_path> & vp);

    double nbest(const utils::crf_sample_sequence & seq,
                 std::vector<utils::crf_path> & sequences,
                 const int max_num, const double min_prob);

    double FunctionGradient(const std::vector<double> & x,
                            std::vector<double> & grad);

    /*
      static double FunctionGradientWrapper(const std::vector<double> & x,
      std::vector<double> & grad);
    */

    double forward_prob(const int len);
    double backward_prob(const int len);

    int& edge_feature_id3(const int w, const int x, const int y, const int z);// const;

    int& edge_feature_id2(const int x, const int y, const int z);// const;

    int& edge_feature_id(const int l, const int r);// const;

    double& state_weight(const int x, const int l);// const;

    double& edge_weight2(const int x, const int y, const int z);// const;

    double& edge_weight3(const int w, const int x, const int y, const int z);// const;

    double& edge_weight(const int l, const int r);// const;

    double& forward_cache(const int x, const int l);// const;

    double& backward_cache(const int x, const int l);// const;

    int& backward_pointer(const int x, const int l);// const;

  private:

    /*********************
     ***  members  ***
     *********************/

    std::vector<utils::crf_sample_sequence> _vs; // vector of training_samples
    std::vector<utils::crf_sample_sequence> validation_samples; // vector of training_samples

    //StringBag _label_bag;
    utils::string_bag _label_bag;
    //  MiniStringBag _featurename_bag;

    utils::string_dict _featurename_bag;

    double _sigma; // Gaussian prior
    double _inequality_width;

    std::vector<double> _vl;  // vector of lambda
    std::vector<bool> is_edge;

    utils::crf_feature_bag _fb;
    int _num_classes;

    std::vector<double> _vee;  // empirical expectation
    std::vector<double> _vme;  // model expectation
    std::vector< std::vector< int > > _feature2mef;

    double _train_error;   // current error rate on the training data
    double _heldout_error; // current error rate on the heldout data

    int _early_stopping_n;

    std::vector<double> _vhlogl;

    int _line_counter; // for error message. Incremented at forward_backward

    //int nbest_search_path[crf_model::MAX_LEN];
    //std::vector<int> nbest_search_path;

    /*
      static int edge_feature_id[crf_model::MAX_LABEL_TYPES][crf_model::MAX_LABEL_TYPES];
      static double state_weight[MAX_LEN][crf_model::MAX_LABEL_TYPES];
      static double edge_weight[crf_model::MAX_LABEL_TYPES][crf_model::MAX_LABEL_TYPES];
      static double forward_cache[MAX_LEN][crf_model::MAX_LABEL_TYPES];
      static double backward_cache[MAX_LEN][crf_model::MAX_LABEL_TYPES];
      static int backward_pointer[MAX_LEN][crf_model::MAX_LABEL_TYPES];
    */

    /*
      int* p_edge_feature_id;
      int* p_edge_feature_id2;
      int* p_edge_feature_id3;
      int* p_backward_pointer;
    */

    std::vector<int> nbest_search_path,
      p_edge_feature_id,
      p_edge_feature_id2,
      p_edge_feature_id3,
      p_backward_pointer;

    /*
      double* p_state_weight;
      double* p_edge_weight;
      double* p_edge_weight2;
      double* p_edge_weight3;
      double* p_forward_cache;
      double* p_backward_cache;
    */

    std::vector<double> p_state_weight,
      p_edge_weight,
      p_edge_weight2,
      p_edge_weight3,
      p_forward_cache,
      p_backward_cache;
  };

}

#endif
