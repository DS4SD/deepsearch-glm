//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_MODEL_CPP_
#define ANDROMEDA_BASE_CRF_MODEL_CPP_

#include <cmath>
#include <cstdio>
#include <cfloat>
#include <set>
#include <random>
#include <algorithm>

namespace andromeda_crf
{
  //static crf_model* pointer_to_working_object = NULL; // this is not a good solution...

  crf_model::crf_model()
  {
    _early_stopping_n = 0;
    _line_counter = 0;

    nbest_search_path.resize(crf_model::MAX_LEN, 0);

    //p_edge_feature_id2  = (int*)malloc(sizeof(int) * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES);
    //p_edge_feature_id  = (int*)malloc(sizeof(int) * MAX_LABEL_TYPES * MAX_LABEL_TYPES);
    //p_backward_pointer = (int*)malloc(sizeof(int) * MAX_LEN * MAX_LABEL_TYPES);

    p_edge_feature_id .resize(std::pow(MAX_LABEL_TYPES, 2), 0);
    p_edge_feature_id2.resize(std::pow(MAX_LABEL_TYPES, 3), 0);
    p_backward_pointer.resize(MAX_LEN*MAX_LABEL_TYPES, 0);

    //p_edge_weight      = (double*)malloc(sizeof(double) * MAX_LABEL_TYPES * MAX_LABEL_TYPES);
    //p_edge_weight2     = (double*)malloc(sizeof(double) * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES);
    //p_state_weight     = (double*)malloc(sizeof(double) * MAX_LEN * MAX_LABEL_TYPES);
    //p_forward_cache    = (double*)malloc(sizeof(double) * MAX_LEN * MAX_LABEL_TYPES);
    //p_backward_cache   = (double*)malloc(sizeof(double) * MAX_LEN * MAX_LABEL_TYPES);

    p_edge_weight.resize(std::pow(MAX_LABEL_TYPES, 2));
    p_edge_weight2.resize(std::pow(MAX_LABEL_TYPES, 3));
    p_state_weight.resize(MAX_LEN * MAX_LABEL_TYPES);
    p_forward_cache.resize(MAX_LEN * MAX_LABEL_TYPES);
    p_backward_cache.resize(MAX_LEN * MAX_LABEL_TYPES);

    if(USE_EDGE_TRIGRAMS)
      {
        //p_edge_feature_id3  = (int*)malloc(sizeof(int) * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES);
        p_edge_feature_id3.resize(std::pow(MAX_LABEL_TYPES, 4));

        //p_edge_weight3     = (double*)malloc(sizeof(double) * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES);

        p_edge_weight3.resize(std::pow(MAX_LABEL_TYPES, 4));
      }
  }

  crf_model::~crf_model()
  {
    //if (USE_EDGE_TRIGRAMS) {
    //free(p_edge_feature_id3);
    //free(p_edge_weight3);
    //}

    //free(p_edge_feature_id2);
    //free(p_edge_feature_id);
    //free(p_backward_pointer);

    //free(p_state_weight);
    //free(p_edge_weight2);
    //free(p_edge_weight);
    //free(p_forward_cache);
    //free(p_backward_cache);
  }

  int& crf_model::edge_feature_id3(const int w, const int x, const int y, const int z) //const
  {
    assert(w >= 0 && w < MAX_LABEL_TYPES);
    assert(x >= 0 && x < MAX_LABEL_TYPES);
    assert(y >= 0 && y < MAX_LABEL_TYPES);
    assert(z >= 0 && z < MAX_LABEL_TYPES);

    return p_edge_feature_id3.at(w * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES +
                                 x * MAX_LABEL_TYPES * MAX_LABEL_TYPES +
                                 y * MAX_LABEL_TYPES +
                                 z);
  }

  int& crf_model::edge_feature_id2(const int x, const int y, const int z) //const
  {
    assert(x >= 0 && x < MAX_LABEL_TYPES);
    assert(y >= 0 && y < MAX_LABEL_TYPES);
    assert(z >= 0 && z < MAX_LABEL_TYPES);

    return p_edge_feature_id2.at(x * MAX_LABEL_TYPES * MAX_LABEL_TYPES +
                                 y * MAX_LABEL_TYPES +
                                 z);
  }

  int& crf_model::edge_feature_id(const int l, const int r) //const
  {
    assert(l >= 0 && l < MAX_LABEL_TYPES);
    assert(r >= 0 && r < MAX_LABEL_TYPES);

    return p_edge_feature_id.at(l*MAX_LABEL_TYPES +
                                r);
  }

  double& crf_model::state_weight(const int x, const int l) //const
  {
    return p_state_weight.at(x * MAX_LABEL_TYPES +
                             l);
  }

  double& crf_model::edge_weight2(const int x, const int y, const int z) //const
  {
    return p_edge_weight2.at(x * MAX_LABEL_TYPES * MAX_LABEL_TYPES +
                             y * MAX_LABEL_TYPES +
                             z);
  }

  double& crf_model::edge_weight3(const int w, const int x, const int y, const int z) //const
  {
    return p_edge_weight3.at(w * MAX_LABEL_TYPES * MAX_LABEL_TYPES * MAX_LABEL_TYPES +
                             x * MAX_LABEL_TYPES * MAX_LABEL_TYPES +
                             y * MAX_LABEL_TYPES +
                             z);
  }

  double& crf_model::edge_weight(const int l, const int r) //const
  {
    return p_edge_weight.at(l * MAX_LABEL_TYPES + r);
  }

  double& crf_model::forward_cache(const int x, const int l) //const
  {
    return p_forward_cache.at(x * MAX_LABEL_TYPES + l);
  }

  double& crf_model::backward_cache(const int x, const int l) //const
  {
    return p_backward_cache.at(x * MAX_LABEL_TYPES + l);
  }

  int& crf_model::backward_pointer(const int x, const int l) //const
  {
    return p_backward_pointer.at(x * MAX_LABEL_TYPES + l);
  }

  double crf_model::FunctionGradient(const std::vector<double>& x,
                                     std::vector<double>& grad)
  {
    assert(_fb.Size() == x.size());
    for(std::size_t i=0; i<x.size(); i++)
      {
        _vl[i] = x[i];

        if(_vl[i] < -50)
          {
            _vl[i] = -50;
          }

        if(_vl[i] >  50)
          {
            _vl[i] =  50;
          }
      }

    double score = update_model_expectation();

    if(_sigma==0)
      {
        for(std::size_t i = 0; i < x.size(); i++)
          {
            grad[i] = -(_vee[i] - _vme[i]);
          }
      }
    else
      {
        const double c = 1.0 / (_sigma * _sigma);

        for(std::size_t i = 0; i < x.size(); i++)
          {
            grad[i] = -(_vee[i] - _vme[i] - c * _vl[i]);
          }
      }

    return -score;
  }

  /*
    double crf_model::FunctionGradientWrapper(const std::vector<double> & x,
    std::vector<double> & grad)
    {
    return pointer_to_working_object->FunctionGradient(x, grad);
    }
  */

  /*
    int
    crf_model::perform_BFGS()
    {

    pointer_to_working_object = this;

    const int dim = _fb.Size();
    std::vector<double> x0(dim);

    for (int i = 0; i < dim; i++) { x0[i] = _vl[i]; }

    std::vector<double> x;
    if (_inequality_width > 0) {
    cerr << "performing OWL-QN" << std::endl;
    x = perform_OWLQN(crf_model::FunctionGradientWrapper, x0, _inequality_width);
    } else {
    cerr << "performing L-BFGS" << std::endl;
    x = perform_LBFGS(crf_model::FunctionGradientWrapper, x0);
    }

    for (int i = 0; i < dim; i++) { _vl[i] = x[i]; }

    return 0;

    }
  */

  double crf_model::forward_prob(const int len)
  {
    for (int x = 0; x < len; x++) {
      //    double maxv = 0;
      double total = 0;
      for (int i = 0; i < _num_classes; i++) {
        double sum;
        if (x == 0) {
          //  sum = edge_weight[_num_classes][i]; // BOS
          sum = edge_weight(_num_classes, i); // BOS
        } else {
          sum = 0;
          for (int j = 0; j < _num_classes; j++) {
            sum += edge_weight(j, i) * forward_cache(x-1, j);
          }
          //  for (std::vector<int>::const_iterator j = _tagrl[i].begin(); j != _tagrl[i].end(); j++){
          //    sum += edge_weight[*j][i] * forward_cache[x-1][*j];
          //  }
        }
        sum *= state_weight(x, i);
        forward_cache(x, i) = sum;
        //      maxv = max(sum, maxv);
        total += sum;
      }
      //    maxv *= 0.0000000000001;
      for (int i = 0; i < _num_classes; i++) {
        forward_cache(x, i) /= total;
        state_weight(x, i) /= total;
      }
    }

    double total = 0;
    for (int i = 0; i < _num_classes; i++) {
      total += forward_cache(len-1, i) * edge_weight(i, _num_classes+1); // EOS
    }

    return total;
  }

  double crf_model::backward_prob(const int len)
  {
    for (int x = len - 1; x >= 0; x--) {
      for (int i = 0; i < _num_classes; i++) {
        double sum;
        if (x == len - 1) {
          sum = edge_weight(i, _num_classes+1); // EOS
        } else {
          sum = 0;
          for (int j = 0; j < _num_classes; j++) {
            sum += edge_weight(i, j) * backward_cache(x+1, j);
          }
          //  for (std::vector<int>::const_iterator j = _taglr[i].begin(); j != _taglr[i].end(); j++){
          //    sum += edge_weight[i][*j] * backward_cache[x+1][*j];
          //  }

        }
        sum *= state_weight(x, i);
        backward_cache(x, i) = sum;
      }
    }

    double total = 0;
    for (int i = 0; i < _num_classes; i++) {
      total += backward_cache(0, i) * edge_weight(_num_classes, i); // BOS
    }

    return total;
  }

  void crf_model::initialize_edge_weights()
  {
    for (int i = 0; i < _label_bag.Size(); i++) {
      for (int j = 0; j < _label_bag.Size(); j++) {
        const int id = edge_feature_id(i, j);
        //      if (id < 0) { edge_weight[i][j] = 1; continue; }
        assert(id >= 0);
        const double ew = _vl[id];
        edge_weight(i, j) = exp(ew);
      }
    }
  }

  void crf_model::initialize_state_weights(const utils::crf_sample_sequence & seq)
  {
    std::vector<double> powv(_num_classes);

    for(std::size_t i=0; i<seq.vs.size(); i++)
      {
        //    std::vector<double> powv(_num_classes, 0.0);
        powv.assign(_num_classes, 0.0);
        const utils::crf_sample & s = seq.vs[i];

        for(auto j = s.positive_features.begin(); j != s.positive_features.end(); j++)
          {
            for(auto k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++)
              {
                const double w = _vl[*k];
                powv[_fb.Feature(*k).label()] += w;
              }
          }

        for(int j=0; j<_num_classes; j++)
          {
            state_weight(i, j) = exp(powv[j]);
          }
      }
  }

  double crf_model::forward_backward(const utils::crf_sample_sequence& seq)
  {
    initialize_state_weights(seq);
    //  num_tags = _num_classes;

    const double fp = forward_prob(seq.vs.size());
    const double bp = backward_prob(seq.vs.size());

    assert(abs(fp - 1) < 0.01);
    assert(abs(bp - 1) < 0.01);

    /*
      if (seq.vs.size() > 60) {
      cerr << "len = " << seq.vs.size() << " ";
      cerr << "fp = " << fp << " ";
      cerr << "bp = " << bp << std::endl;
      for (int i = 0; i < seq.vs.size(); i++) {
      double m = 0;
      for (int j = 0; j < _num_classes; j++) {
      m = max(m, backward_cache[i][j]);
      }
      cerr << m << " ";
      }
      cerr << std::endl;
      }
    */

    /*
      if (!(fp > 0 && fp < DBL_MAX)) {
      cout << std::endl;
      cout << "fp = " << fp << std::endl;
      cout << "scaling factor " << std::endl;
      for (int i = 0; i < seq.vs.size(); i++) {
      cout << i << " " << scaling_factor[i] << std::endl;
      }
      cout << std::endl;

      for (int i = 0; i < seq.vs.size(); i++) {
      for (int j = 0; j < _num_classes; j++) {
      cout << forward_cache[i][j] << "\t";
      }
      cout << std::endl;
      }
      //    forward_prob_recur(seq.vs.size(), seq.vs.size() - 2, 0);

      save_to_file("model");
      exit(1);
      }
    */

    if (!(fp > 0 && fp < DBL_MAX && bp > 0 && bp < DBL_MAX))
      {
        //std::cerr << std::endl << "error: line:" << _line_counter << " floating overflow. a different value of Gaussian prior might work." << std::endl;
        ////std::cerr << std::endl << "error: floating overflow.  " << std::endl;

        LOG_S(ERROR) << "line:" << _line_counter << " floating overflow. "
                     << " --> a different value of Gaussian prior might work!";

        return 1.f; // may cause assert failure because forward_cache/backward_cache are also overflow
      }

    //  assert(abs(fp - bp) < 0.1);
    assert(fp > 0 && fp < DBL_MAX);
    assert(bp > 0 && bp < DBL_MAX);

    return fp;
  }

  double crf_model::viterbi(const utils::crf_sample_sequence & seq,
                            std::vector<int> & best_seq)
  {
    initialize_state_weights(seq);
    //  num_tags = _num_classes;

    const int len = seq.vs.size();

    for (int x = 0; x < len; x++) {
      double total = 0;
      for (int i = 0; i < _num_classes; i++) {
        double m = -DBL_MAX;
        if (x == 0) {
          m = edge_weight(_num_classes, i); // BOS
        } else {
          for (int j = 0; j < _num_classes; j++) {
            double score = edge_weight(j, i) * forward_cache(x-1, j);
            if (score > m) {
              m = score;
              backward_pointer(x, i) = j;
            }
          }
        }
        m *= state_weight(x, i);
        forward_cache(x, i) = m;
        total += m;
      }
      for (int i = 0; i < _num_classes; i++) {
        forward_cache(x, i) /= total;
      }
    }

    double m = -DBL_MAX;
    for (int i = 0; i < _num_classes; i++) {
      double score = forward_cache(len-1, i) * edge_weight(i, _num_classes+1); // EOS
      if (score > m) {
        m = score;
        best_seq[len-1] = i;
      }
    }
    for (int x = len - 2; x >= 0; x--) {
      best_seq[x] = backward_pointer(x+1, best_seq[x+1]);
    }

    return 0;
  }

  void crf_model::nbest_search(const double lb, const int len, const int x, const int y,
                               const double rhs_score, std::vector<utils::crf_path> & vp)
  {
    if (x < len && forward_cache(x, y) * rhs_score < lb) return;

    nbest_search_path[x] = y;

    // root node
    if (x == len) {
      for (int i = 0; i < _num_classes; i++) {
        nbest_search(lb, len, x - 1, i, rhs_score, vp);
      }
      return;
    }

    const double sw = state_weight(x, y);

    // leaf nodes
    if (x == 0) {
      const double path_score = rhs_score * sw;
      utils::crf_path p(path_score, std::vector<int>(&(nbest_search_path[0]), &(nbest_search_path[len])));
      vp.push_back(p);
      return;
    }

    for (int i = 0; i < _num_classes; i++) {
      const double ew = edge_weight(i, y);
      nbest_search(lb, len, x - 1, i, rhs_score * sw * ew, vp);
    }
  }

  double crf_model::nbest(const utils::crf_sample_sequence & seq, std::vector<utils::crf_path> & vp,
                          const int max_num, const double min_prob)
  {
    //  num_tags = _num_classes;

    initialize_state_weights(seq);
    forward_prob(seq.vs.size());

    //const double fp = forward_prob(seq.vs.size());
    //assert(abs(fp - 1) < 0.01);

    const int len = seq.vs.size();

    for (int x = 0; x < len; x++) {
      for (int i = 0; i < _num_classes; i++) {
        double m = -DBL_MAX;
        if (x == 0) {
          m = edge_weight(_num_classes, i); // BOS
        } else {
          for (int j = 0; j < _num_classes; j++) {
            double score = edge_weight(j, i) * forward_cache(x-1, j);
            if (score > m) {
              m = score;
              backward_pointer(x, i) = j;
            }
          }
        }
        m *= state_weight(x, i);
        forward_cache(x, i) = m;
      }
    }

    double m = -DBL_MAX;
    for (int i = 0; i < _num_classes; i++) {
      double score = forward_cache(len-1, i) * edge_weight(i, _num_classes+1); // EOS
      if (score > m) {
        m = score;
      }
    }

    // n-best
    int iter = 0;
    //  for (double lb = 0.5 * m; lb >= min_prob; lb *= 0.5) {
    for (double lb = 0.3 * m;; lb *= 0.3) {
      vp.clear();
      nbest_search(std::max(lb, min_prob), len, len, 0, 1.0, vp);
      //    double sum = 0;
      //    for (std::vector<utils::crf_path>::const_iterator i = vp.begin(); i != vp.end(); i++)
      //      sum += i->score;
      //    if (abs(sum - 1.0) < 0.000000001) break;
      if (iter++ > 1000) break;
      if ((int)vp.size() >= max_num) break;
      if (lb <= min_prob) break;
    }
    //  std::cerr << iter << std::endl;
    sort(vp.begin(), vp.end());

    return vp.size();
  }


  /**
     double crf_model::calc_likelihood(const utils::crf_sample_sequence & seq, double fp)
     {
     assert(abs(fp - 1.0) < 0.01);
     const int len = seq.vs.size();

     double p = 1.0;
     for (int i = 0; i < len; i++) {
     p *= state_weight[i][seq.vs[i].label];
     assert(p > 0);
     }
     for (int i = 0; i < len-1; i++) {
     p *= edge_weight[seq.vs[i].label][seq.vs[i+1].label];
     }
     p *= edge_weight[_num_classes][seq.vs[0].label];       // BOS
     p *= edge_weight[seq.vs[len-1].label][_num_classes+1]; // EOS

     //  p /= fp;
     //    cout << "p = " << p << std::endl;
     assert(p > 0);

     return p;
     }
  **/

  double crf_model::calc_loglikelihood(const utils::crf_sample_sequence & seq)
  {
    const int len = seq.vs.size();

    double logp = 0;
    for (int i = 0; i < len; i++) {
      logp += log(state_weight(i, seq.vs[i].label));
    }
    for (int i = 0; i < len-1; i++) {
      logp += log(edge_weight(seq.vs[i].label, seq.vs[i+1].label));
    }
    logp += log(edge_weight(_num_classes, seq.vs[0].label));       // BOS
    logp += log(edge_weight(seq.vs[len-1].label,_num_classes+1)); // EOS

    return logp;
  }

  int crf_model::make_feature_bag(const int cutoff)
  {
    //#ifdef USE_HASH_MAP
    //  typedef __gnu_cxx::hash_map<mefeature_type, int> map_type;
    //typedef std::tr1::unordered_map<mefeature_type, int> map_type;
    //#else
    //typedef std::map<mefeature_type, int> map_type;
    //#endif

    //map_type count;

    std::map<feature_body_type, int> count;

    if(cutoff > 0)
      {
        for(auto k = _vs.begin(); k != _vs.end(); k++)
          {
            for(auto i = k->vs.begin(); i != k->vs.end(); i++)
              {
                for(auto j = i->positive_features.begin(); j != i->positive_features.end(); j++)
                  {
                    //utils::ME_Feature(i->label, *j).body()
                    utils::crf_feature feature(i->label, *j);
                    count[feature.body()]++;

                    //count[utils::crf_feature(i->label, *j).body()]++;
                  }
              }
          }
      }

    for(auto k = _vs.begin(); k != _vs.end(); k++)
      {
        for(auto i = k->vs.begin(); i != k->vs.end(); i++)
          {
            for(auto j = i->positive_features.begin(); j != i->positive_features.end(); j++)
              {
                const utils::crf_feature feature(i->label, *j);

                if(cutoff>0 && count[feature.body()] <= cutoff)
                  {
                    continue;
                  }

                _fb.Put(feature);
              }
          }
      }

    init_feature2mef();

    return 0;
  }

  double crf_model::heldout_likelihood()
  {
    double logl = 0;
    int ncorrect = 0, total_len = 0;

    initialize_edge_weights();
    //for (std::vector<utils::crf_sample_sequence>::const_iterator i = _heldout.begin(); i != _heldout.end(); i++) {
    for(auto itr=validation_samples.begin(); itr!=validation_samples.end(); itr++)
      {
        total_len += itr->vs.size();

        // double fp = forward_backward(*i);
        // double p = calc_likelihood(*i, fp);
        // logl += log(p);

        logl += calc_loglikelihood(*itr);

        for(std::size_t j=0; j<itr->vs.size(); j++)
          {
            const utils::crf_sample & s = itr->vs[j];
            //      std::vector<double> wsum = calc_state_weight(*i, j);

            std::vector<double> wsum = calc_state_weight(j);
            if(s.label==max_element(wsum.begin(), wsum.end())-wsum.begin())
              {
                ncorrect++;
              }
          }
      }
    _heldout_error = 1 - (double)ncorrect / total_len;

    //return logl /= _heldout.size();
    return logl /= validation_samples.size();
  }

  //std::vector<double> crf_model::calc_state_weight(const utils::crf_sample_sequence & seq, const int i) const
  std::vector<double> crf_model::calc_state_weight(const int i) //const
  {
    std::vector<double> wsum(_num_classes);

    for(int j = 0; j < _num_classes; j++)
      {
        wsum[j] = forward_cache(i, j) / state_weight(i, j) * backward_cache(i, j);
      }

    return wsum;
  }

  void crf_model::add_sample_empirical_expectation(const utils::crf_sample_sequence & seq,
                                                   std::vector<double> & vee)
  {
    for(std::size_t i = 0; i < seq.vs.size(); i++) {
      for(auto j = seq.vs[i].positive_features.begin(); j != seq.vs[i].positive_features.end(); j++){
        for(auto k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++) {
          if (_fb.Feature(*k).label() == seq.vs[i].label) {
            assert(*k >= 0 && *k < _vee.size());
            _vee[*k] += 1.0;
          }
        }
      }
    }

    for(int i = 0; i < (int)seq.vs.size() - 1; i++) {
      const int c0 = seq.vs[i].label;
      const int c1 = seq.vs[i+1].label;
      _vee[edge_feature_id(c0, c1)] += 1.0;
    }
    if (USE_BOS_EOS) {
      _vee[edge_feature_id(_num_classes, seq.vs[0].label)] += 1.0;
      _vee[edge_feature_id(seq.vs[seq.vs.size()-1].label, _num_classes+1)] += 1.0;
    }
  }

  double crf_model::add_sample_model_expectation(const utils::crf_sample_sequence & seq,
                                                 std::vector<double>& vme,
                                                 int & ncorrect)
  {
    forward_backward(seq);
    //const double fp = forward_backward(seq);
    //assert(abs(fp - 1.0) < 0.01);

    //    double p = calc_likelihood(seq, fp);
    //    logl += log(p);
    const double logl = calc_loglikelihood(seq);

    for(std::size_t i = 0; i < seq.vs.size(); i++)
      {
        const utils::crf_sample & s = seq.vs[i];

        // model expectation (state)
        std::vector<double> wsum = calc_state_weight(i);
        for(auto j = s.positive_features.begin(); j != s.positive_features.end(); j++){
          for(auto k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++) {
            //vme[*k] += wsum[_fb.Feature(*k).label()] / fp;
            vme[*k] += wsum[_fb.Feature(*k).label()];
          }
        }

        if(s.label == max_element(wsum.begin(), wsum.end()) - wsum.begin())
          ncorrect++;

        // model expectation (edge)
        if (i == seq.vs.size() - 1) continue;

        for (int j = 0; j < _num_classes; j++) {
          const double lhs = forward_cache(i, j);
          for (int k = 0; k < _num_classes; k++) {
            const double rhs = backward_cache(i+1, k);
            assert(lhs != DBL_MAX && rhs != DBL_MAX);
            //const double w = lhs * edge_weight[j][k] * rhs / fp;
            const double w = lhs * edge_weight(j, k) * rhs;
            vme[edge_feature_id(j, k)] += w;
          }
        }
      }

    if (USE_BOS_EOS) {
      // model expectation (BOS -> *)
      for (int j = 0; j < _num_classes; j++) {
        const double rhs = backward_cache(0, j);
        //const double w = edge_weight[_num_classes][j] * rhs / fp;
        const double w = edge_weight(_num_classes, j) * rhs;
        vme[edge_feature_id(_num_classes, j)] += w;
      }
      // model expectation (* -> EOS)
      const int len = seq.vs.size();
      for (int j = 0; j < _num_classes; j++) {
        const double lhs = forward_cache(len-1, j);
        //const double w = edge_weight[j][_num_classes+1] * lhs / fp;
        const double w = edge_weight(j, _num_classes+1) * lhs;
        vme[edge_feature_id(j, _num_classes+1)] += w;
      }
    }

    return logl;
  }

  double crf_model::update_model_expectation()
  {
    double logl = 0;
    int ncorrect = 0, total_len = 0;

    _vme.resize(_fb.Size());
    for (int i = 0; i < _fb.Size(); i++) _vme[i] = 0;

    initialize_edge_weights();

    for(auto n = _vs.begin(); n != _vs.end(); n++) {
      const utils::crf_sample_sequence & seq = *n;
      total_len += seq.vs.size();
      logl += add_sample_model_expectation(seq, _vme, ncorrect);
    }

    for (int i = 0; i < _fb.Size(); i++) {
      _vme[i] /= _vs.size();
    }

    _train_error = 1 - (double)ncorrect / total_len;

    logl /= _vs.size();

    if (_sigma > 0) {
      const double c = 1/(2*_sigma*_sigma);
      for (int i = 0; i < _fb.Size(); i++) {
        logl -= _vl[i] * _vl[i] * c;
      }
    }

    //logl /= _vs.size();

    //  fprintf(stderr, "iter =%3d  logl = %10.7f  train_acc = %7.5f\n", iter, logl, (double)ncorrect/train.size());
    //  fprintf(stderr, "logl = %10.7f  train_acc = %7.5f\n", logl, (double)ncorrect/_train.size());

    return logl;
  }

  inline bool contain_space(const std::string & s)
  {
    //for (int i = 0; i < s.size(); i++) {
    for(std::size_t i=0; i<s.size(); i++)
      {
        if(isspace(s[i]))
          {
            return true;
          }
      }

    return false;
  }

  /*
    void crf_model::add_training_sample(const CRF_Sequence & seq)
    {
    if (seq.vs.size() >= MAX_LEN) {
    std::cerr << "error: sequence is too long.";
    exit(1);
    }
    if (seq.vs.size() == 0) {
    std::cerr << "warning: empty sentence" << std::endl;
    return;
    }
    assert(seq.vs.size() > 0);

    utils::crf_sample_sequence s1;
    for (std::vector<CRF_State>::const_iterator i = seq.vs.begin(); i != seq.vs.end(); i++) {

    if (i->label == BOS_LABEL || i->label == EOS_LABEL) {
    std::cerr << "error: the label name \"" << i->label << "\" is reserved. Use a different name.";
    exit(1);
    }

    if (contain_space(i->label)) {
    std::cerr << "error: the name of a label must not contain any space." << std::endl; exit(1);
    }

    utils::crf_sample s;
    s.label = _label_bag.Put(i->label);
    if (s.label >= MAX_LABEL_TYPES - 2) {
    std::cerr << "error: too many types of labels." << std::endl;
    exit(1);
    }

    assert(s.label >= 0 && s.label < MAX_LABEL_TYPES);

    for (std::vector<std::string>::const_iterator j = i->features.begin(); j != i->features.end(); j++) {
    if (contain_space(*j)) {
    std::cerr << "error: the name of a feature must not contain any space." << std::endl; exit(1);
    }
    s.positive_features.push_back(_featurename_bag.Put(*j));
    }

    s1.vs.push_back(s);
    }

    _vs.push_back(s1);
    }
  */

  void crf_model::convert(const utils::crf_state_sequence& seq, utils::crf_sample_sequence& s1)
  {
    if (seq.vs.size() >= MAX_LEN) {
      LOG_S(ERROR) << "error: sequence is too long.";
      return;

    }

    if (seq.vs.size() == 0) {
      LOG_S(ERROR) << "warning: empty sentence" << std::endl;
      return;
    }
    assert(seq.vs.size() > 0);

    for (std::vector<utils::crf_state>::const_iterator i = seq.vs.begin(); i != seq.vs.end(); i++) {

      if (i->label == BOS_LABEL || i->label == EOS_LABEL) {
        LOG_S(ERROR) << "error: the label name \"" << i->label << "\" is reserved. Use a different name.";
        exit(1);
      }

      if (contain_space(i->label)) {
        LOG_S(ERROR) << "error: the name of a label must not contain any space.";
        exit(1);
      }

      utils::crf_sample s;
      s.label = _label_bag.Put(i->label);
      if (s.label >= MAX_LABEL_TYPES - 2) {
        LOG_S(ERROR) << "error: too many types of labels." << std::endl;
        exit(1);
      }

      assert(s.label >= 0 && s.label < MAX_LABEL_TYPES);

      for (std::vector<std::string>::const_iterator j = i->features.begin(); j != i->features.end(); j++)
	{
	  if(contain_space(*j))
	    {
	      LOG_S(ERROR) << "error: the name of a feature (" << (*j) << ") must not contain any space.";

	      std::string feat = *j;
	      feat = andromeda::utils::replace(feat, " ", "_");

	      s.positive_features.push_back(_featurename_bag.Put(feat));
	    }
	  else	    
	    {
	      s.positive_features.push_back(_featurename_bag.Put(*j));
	    }
      }
      s1.vs.push_back(s);
    }

    _vs.push_back(s1);
  }

  void crf_model::add_training_sample(const utils::crf_state_sequence& crf_seq)
  {
    utils::crf_sample_sequence sample_seq;
    convert(crf_seq, sample_seq);

    _vs.push_back(sample_seq);
  }

  void crf_model::add_validation_sample(const utils::crf_state_sequence& crf_seq)
  {
    utils::crf_sample_sequence sample_seq;
    convert(crf_seq, sample_seq);

    validation_samples.push_back(sample_seq);
  }

  int crf_model::train(const OptimizationMethod method, const int epochs, const int cutoff,
                       const double sigma, const double widthfactor)
  //           const double Nsigma2, const double widthfactor)
  {
    if (sigma > 0 && widthfactor > 0) {
      //  if (Nsigma2 > 0 && widthfactor > 0) {
      LOG_S(ERROR) << "error: Gausian prior and inequality modeling cannot be used together." << std::endl;
      return 0;
    }

    if (_vs.size() == 0) {
      LOG_S(ERROR) << "error: no training data." << std::endl;
      return 0;
    }

    //if (_nheldout >= (int)_vs.size()) {
    //LOG_S(ERROR) << "error: too much heldout data. no training data is available." << std::endl;
    //return 0;
    //}
    //if (_nheldout > 0) random_shuffle(_vs.begin(), _vs.end());

    _label_bag.Put(BOS_LABEL);
    _label_bag.Put(EOS_LABEL);
    _num_classes = _label_bag.Size() - 2;

    //_nheldout = int(0.05*(_vs.size()));
    //LOG_S(WARNING) << "#-validation: " << _nheldout;

    //for (int i = 0; i < _nheldout; i++)
    //{
    //_heldout.push_back(_vs.back());
    //_vs.pop_back();
    //}
    //LOG_S(WARNING) << "#-training: " << _vs.size();

    //int total_len = 0;
    //for(size_t i=0; i<_vs.size(); i++)
    //{
    //total_len += _vs[i].vs.size();
    //}

    //  _sigma = sqrt((double)total_len / Nsigma2);
    //  if (Nsigma2 == 0) _sigma = 0;
    _sigma = sigma;
    _inequality_width = widthfactor / _vs.size();

    //if (cutoff > 0)
    {
      //LOG_S(ERROR) << "cutoff threshold = " << cutoff << std::endl;
      LOG_S(INFO) << "gaussion sigma = " << _sigma;
      LOG_S(INFO) << "cutoff threshold = " << cutoff;
    }
    //  if (_sigma > 0) LOG_S(ERROR) << "Gaussian prior sigma = " << _sigma << std::endl;
    //    LOG_S(ERROR) << "N*sigma^2 = " << Nsigma2 << " sigma = " << _sigma << std::endl;

    //if (widthfactor > 0)
    {
      //LOG_S(ERROR) << "widthfactor = " << widthfactor << std::endl;
      LOG_S(INFO) << "widthfactor = " << widthfactor;
    }

    //LOG_S(ERROR) << "preparing for estimation...";
    LOG_S(INFO) << "preparing for estimation ...";

    make_feature_bag(cutoff);
    //  _vs.clear();

    //LOG_S(ERROR) << "done" << std::endl;
    //LOG_S(ERROR) << "number of state types = " << _num_classes << std::endl;
    //LOG_S(ERROR) << "number of samples = " << _vs.size() << std::endl;
    //LOG_S(ERROR) << "number of features = " << _fb.Size() << std::endl;

    LOG_S(INFO) << "#-labels: " << _num_classes << ", "
                << "#-samples: " << _vs.size() << ", "
                << "#-features: " << _fb.Size();

    //LOG_S(ERROR) << "calculating empirical expectation...";
    LOG_S(INFO) << "calculating empirical expectation...";

    _vee.resize(_fb.Size());
    _vee.assign(_vee.size(), 0.0);

    int count = 0;
    for (std::vector<utils::crf_sample_sequence>::const_iterator n = _vs.begin(); n != _vs.end(); n++, count++) {
      add_sample_empirical_expectation(*n, _vee);
    }

    for(std::size_t i=0; i<_vee.size(); i++)
      {
        _vee[i] /= _vs.size();
      }

    //LOG_S(ERROR) << "done" << std::endl;
    LOG_S(INFO) << "done";

    _vl.resize(_fb.Size());
    _vl.assign(_vl.size(), 0.0);

    /*
      switch (method) {
      case BFGS:
      perform_BFGS(); break;
      case PERCEPTRON:
      perform_AveragedPerceptron(); break;
      case SGD:
      perform_StochasticGradientDescent(); break;
      }*/
    perform_LookaheadTraining(epochs);

    if (_inequality_width > 0)
      {
        int sum = 0;
        for (int i = 0; i < _fb.Size(); i++)
          {
            if (_vl[i] != 0) sum++;
          }
        //LOG_S(ERROR) << "number of active features = " << sum << std::endl;
        LOG_S(INFO) << "number of active features = " << sum;
      }

    return 0;
  }

  void crf_model::get_features(std::list< std::pair< std::pair<std::string, std::string>, double> > & fl)
  {
    fl.clear();

    for (utils::string_dict::const_Iterator i = _featurename_bag.begin(); i != _featurename_bag.end(); i++)
      {
        for (int j = 0; j < _label_bag.Size(); j++)
          {
            const std::string label = _label_bag.Str(j);
            const std::string history = i.getStr();

            int id = _fb.Id(utils::crf_feature(j, i.getId()));

            if (id < 0) continue;

            fl.push_back( make_pair(make_pair(label, history), _vl[id]) );
          }
      }
  }

  bool crf_model::load_from_file(const std::string & filename, bool verbose)
  {
    if(verbose)
      {
        LOG_S(INFO) << "loading from " << filename;
      }

    FILE * fp = fopen(filename.c_str(), "r");

    if (!fp)
      {
        LOG_S(ERROR) << "cannot open " << filename;
        return false;
      }

    _vl.clear();
    _label_bag.Clear();
    _featurename_bag.Clear();
    _fb.Clear();

    char buf[1024];
    while(fgets(buf, 1024, fp)) {
      std::string line(buf);
      std::string::size_type t1 = line.find_first_of('\t');
      std::string::size_type t2 = line.find_last_of('\t');
      std::string classname = line.substr(0, t1);
      std::string featurename = line.substr(t1 + 1, t2 - (t1 + 1) );
      float lambda;
      std::string w = line.substr(t2+1);
      sscanf(w.c_str(), "%f", &lambda);

      const int label = _label_bag.Put(classname);
      const int feature = _featurename_bag.Put(featurename);
      _fb.Put(utils::crf_feature(label, feature));
      _vl.push_back(lambda);
    }

    // for zero-wight edges
    _label_bag.Put(BOS_LABEL);
    _label_bag.Put(EOS_LABEL);
    for (int i = 0; i < _label_bag.Size(); i++) {
      for (int j = 0; j < _label_bag.Size(); j++) {
        const std::string & label1 = _label_bag.Str(j);
        const int l1 = _featurename_bag.Put("->\t" + label1);
        const int id = _fb.Id(utils::crf_feature(i, l1));
        if (id < 0) {
          _fb.Put(utils::crf_feature(i, l1));
          _vl.push_back(0);
        }
      }
    }

    for (int i = 0; i < _label_bag.Size(); i++) {
      for (int j = 0; j < _label_bag.Size(); j++) {
        for (int k = 0; k < _label_bag.Size(); k++) {
          const std::string & label1 = _label_bag.Str(j);
          const std::string & label2 = _label_bag.Str(k);
          const int l1 = _featurename_bag.Put("->\t" + label1 + "\t->\t" + label2);
          const int id = _fb.Id(utils::crf_feature(i, l1));
          if (id < 0) {
            _fb.Put(utils::crf_feature(i, l1));
            _vl.push_back(0);
          }
        }
      }
    }

    if (USE_EDGE_TRIGRAMS) {
      for (int i = 0; i < _label_bag.Size(); i++) {
        for (int j = 0; j < _label_bag.Size(); j++) {
          for (int k = 0; k < _label_bag.Size(); k++) {
            for (int l = 0; l < _label_bag.Size(); l++) {

              const std::string & label1 = _label_bag.Str(j);
              const std::string & label2 = _label_bag.Str(k);
              const std::string & label3 = _label_bag.Str(l);

              const int l1 = _featurename_bag.Put("->\t" + label1 + "\t->\t" + label2 + "\t->\t" + label3);
              const int id = _fb.Id(utils::crf_feature(i, l1));

              if (id < 0) {
                _fb.Put(utils::crf_feature(i, l1));
                _vl.push_back(0);
              }
            }
          }
        }
      }
    }

    _num_classes = _label_bag.Size() - 2;

    init_feature2mef();
    initialize_edge_weights();

    fclose(fp);

    if(verbose)
      {
        LOG_S(INFO) << " -> loading CRF-model done!";
      }

    return true;
  }

  void crf_model::init_feature2mef()
  {
    _feature2mef.clear();

    for(std::size_t i = 0; i < _featurename_bag.Size(); i++) {
      std::vector<int> vi;
      for (int k = 0; k < _num_classes; k++) {
        int id = _fb.Id(utils::crf_feature(k, i));
        if (id >= 0) vi.push_back(id);
      }
      _feature2mef.push_back(vi);
    }

    for(int i = 0; i < _label_bag.Size(); i++) {
      for(int j = 0; j < _label_bag.Size(); j++) {
        const std::string & label1 = _label_bag.Str(j);
        const int l1 = _featurename_bag.Put("->\t" + label1);
        const int id = _fb.Put(utils::crf_feature(i, l1));
        edge_feature_id(i, j) = id;
      }
    }

    for (int i = 0; i < _label_bag.Size(); i++) {
      for (int j = 0; j < _label_bag.Size(); j++) {
        for (int k = 0; k < _label_bag.Size(); k++) {
          const std::string & label1 = _label_bag.Str(j);
          const std::string & label2 = _label_bag.Str(k);
          const int l1 = _featurename_bag.Put("->\t" + label1 + "\t->\t" + label2);
          const int id = _fb.Put(utils::crf_feature(i, l1));
          edge_feature_id2(i, j, k) = id;
        }
      }
    }

    if (USE_EDGE_TRIGRAMS) {
      for (int i = 0; i < _label_bag.Size(); i++) {
        for (int j = 0; j < _label_bag.Size(); j++) {
          for (int k = 0; k < _label_bag.Size(); k++) {
            for (int l = 0; l < _label_bag.Size(); l++) {
              const std::string & label1 = _label_bag.Str(j);
              const std::string & label2 = _label_bag.Str(k);
              const std::string & label3 = _label_bag.Str(l);
              const int l1 = _featurename_bag.Put("->\t" + label1 + "\t->\t" + label2 + "\t->\t" + label3);
              const int id = _fb.Put(utils::crf_feature(i, l1));
              edge_feature_id3(i, j, k, l) = id;
            }
          }
        }
      }
    }
  }

  bool crf_model::save_to_file(const std::string & filename, const double th) const
  {
    LOG_S(INFO) << "start saving model to " << filename;

    FILE * fp = fopen(filename.c_str(), "w");
    if (!fp) {
      LOG_S(ERROR) << "cannot open " << filename;
      return false;
    }

    std::size_t tot=0, cnt=0;

    //  for (MiniStringBag::map_type::const_iterator i = _featurename_bag.begin();
    for (utils::string_dict::const_Iterator i = _featurename_bag.begin(); i != _featurename_bag.end(); i++)
      {
        for(int j = 0; j < _label_bag.Size(); j++)
          {
            tot += 1;

            std::string label = _label_bag.Str(j);
            std::string history = i.getStr();

            int id = _fb.Id(utils::crf_feature(j, i.getId()));

            if(id < 0)
              {
                //LOG_S(WARNING) << id << " -> " << label << ": " << history << " => negative id";
                //continue;
              }
            else if(_vl[id] == 0) // ignore zero-weight features
              {
                //LOG_S(WARNING) << id << " -> " << label << ": " << history << " => zero weight";
                //continue;
              }
            else if (abs(_vl[id]) < th) // cut off low-weight features
              {
                //LOG_S(WARNING) << id << " -> " << label << ": " << history << " => low-weight";
                //continue;
              }
            else
              {
                //LOG_S(INFO) << id << " -> " << label << ": " << history << " => writing";
                cnt += 1;
                fprintf(fp, "%s\t%s\t%f\n", label.c_str(), history.c_str(), _vl[id]);
              }
          }
      }

    fclose(fp);

    LOG_S(INFO) << "saved/total features: " << cnt << "/" << tot;
    LOG_S(INFO) << " -> model saved to " << filename;

    return true;
  }

  void crf_model::decode_forward_backward(utils::crf_state_sequence & s0,
                                          std::vector< std::map<std::string, double> > & tagp)
  {
    if (s0.vs.size() >= MAX_LEN) {
      LOG_S(ERROR) << "error: sequence is too long." << std::endl;
      return;
    }

    utils::crf_sample_sequence seq;

    for (std::vector<utils::crf_state>::const_iterator i = s0.vs.begin(); i != s0.vs.end(); i++) {
      utils::crf_sample s;
      for (std::vector<std::string>::const_iterator j = i->features.begin(); j != i->features.end(); j++) {
        const int id = _featurename_bag.Id(*j);
        if (id >= 0) s.positive_features.push_back(id);
      }
      seq.vs.push_back(s);
    }

    tagp.clear();
    forward_backward(seq);

    for(std::size_t i = 0; i < seq.vs.size(); i++)
      {
        std::vector<double> wsum = calc_state_weight(i);
        std::map<std::string, double> tp;

        if (OUTPUT_MARGINAL_PROB)
          {
            double sum = 0;

            for (std::vector<double>::const_iterator j = wsum.begin(); j != wsum.end(); j++) sum += *j;

            s0.vs[i].label = "";
            assert(abs(sum -1) < 0.01);
            double maxp = -1;
            std::string maxtag;

            for (std::size_t j = 0; j < wsum.size(); j++)
              {
                double p = wsum[j]/sum;
                if (p <= 0.001) continue;
                tp[_label_bag.Str(j).c_str()] = p;
                if (p > maxp) { maxp = p; maxtag = _label_bag.Str(j).c_str();}
              }
            tagp.push_back(tp);
            s0.vs[i].label = maxtag;
          } else {
          const int l = max_element(wsum.begin(), wsum.end()) - wsum.begin();
          s0.vs[i].label = _label_bag.Str(l);
        }
      }

  }

  void crf_model::decode_viterbi(utils::crf_state_sequence & s0)
  {
    //  num_tags = _num_classes;

    if (s0.vs.size() >= MAX_LEN) {
      LOG_S(ERROR) << "error: sequence is too long." << std::endl;
      return;
    }

    utils::crf_sample_sequence seq;

    for (std::vector<utils::crf_state>::const_iterator i = s0.vs.begin(); i != s0.vs.end(); i++) {
      utils::crf_sample s;
      for (std::vector<std::string>::const_iterator j = i->features.begin(); j != i->features.end(); j++) {
        const int id = _featurename_bag.Id(*j);
        if (id >= 0) s.positive_features.push_back(id);
      }
      seq.vs.push_back(s);
    }

    std::vector<int> vs(seq.vs.size());
    viterbi(seq, vs);
    for (size_t i = 0; i < seq.vs.size(); i++) {
      s0.vs[i].label = _label_bag.Str(vs[i]);
    }
  }

  void crf_model::decode_nbest(utils::crf_state_sequence & s0,
                               std::vector<std::pair<double, std::vector<std::string> > > & sequences,
                               const int num, const double min_prob)
  {
    //  num_tags = _num_classes;

    if (s0.vs.size() >= MAX_LEN) {
      LOG_S(ERROR) << "error: sequence is too long." << std::endl;
      return;
    }

    utils::crf_sample_sequence seq;

    for (std::vector<utils::crf_state>::const_iterator i = s0.vs.begin(); i != s0.vs.end(); i++) {
      utils::crf_sample s;
      for (std::vector<std::string>::const_iterator j = i->features.begin(); j != i->features.end(); j++) {
        const int id = _featurename_bag.Id(*j);
        if (id >= 0) s.positive_features.push_back(id);
      }
      seq.vs.push_back(s);
    }

    std::vector<int> vs(seq.vs.size());
    std::vector<utils::crf_path> nb;
    nbest(seq, nb, num, min_prob);

    sequences.clear();
    if (nb.size() == 0) return;

    for (size_t i = 0; i < seq.vs.size(); i++) {
      s0.vs[i].label = _label_bag.Str(nb[0].vs[i]);
    }
    for (std::vector<utils::crf_path>::const_iterator i = nb.begin(); i != nb.end(); i++) {
      if (i->score < min_prob) break;
      std::vector<std::string> vstr(i->vs.size());
      for (size_t j = 0; j < vstr.size(); j++) {
        vstr[j] = _label_bag.Str((i->vs)[j]);
      }
      sequences.push_back(std::pair<double, std::vector<std::string> >(i->score, vstr));
      if ((int)sequences.size() >= num) break;
    }
  }

  int crf_model::perform_AveragedPerceptron()
  {
    const int dim = _fb.Size();

    std::vector<double> wsum(dim, 0);

    initialize_edge_weights();

    //  const double a = 1.0;

    int iter = 0;
    while(iter < 5) {

      for (int i = 0; i < dim; i++) wsum[i] += _vl[i] * _vs.size();
      iter++;

      int error_num = 0;
      int rest = _vs.size();
      for (std::vector<utils::crf_sample_sequence>::const_iterator n = _vs.begin(); n != _vs.end(); n++, rest--) {

        const utils::crf_sample_sequence & seq = *n;
        std::vector<int> vs(seq.vs.size());
        viterbi(seq, vs);

        double Loss = 0;
        for (size_t i = 0; i < vs.size(); i++) {
          if (vs[i] != seq.vs[i].label) Loss += 1.0;
        }
        if (Loss == 0) continue;

        if (Loss > 0) error_num++;

        // single best MIRA (just to compute the coefficient)
        std::map<int, double> X;
        // state
        for (size_t i = 0; i < seq.vs.size(); i++) {
          const utils::crf_sample & s = seq.vs[i];
          if (s.label == vs[i]) continue;
          for (std::vector<int>::const_iterator j = s.positive_features.begin(); j != s.positive_features.end(); j++){
            for (std::vector<int>::const_iterator k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++) {
              const int label = _fb.Feature(*k).label();
              if (label == s.label) X[*k] += 1.0;
              if (label == vs[i])   X[*k] -= 1.0;
            }
          }
        }

        // edge
        if(seq.vs.size()>0)
          {
            for(std::size_t i=0; i<seq.vs.size()-1; i++)
              {
                const int eid0 = edge_feature_id(seq.vs[i].label, seq.vs[i+1].label);
                const int eid1 = edge_feature_id(vs[i], vs[i+1]);

                if(eid0 == eid1)
                  {
                    continue;
                  }

                X[eid0] += 1.0;
                X[eid1] -= 1.0;
              }
          }

        double wX = 0, X2 = 0;
        for (std::map<int, double>::const_iterator i = X.begin(); i != X.end(); i++) {
          wX += _vl[i->first] * i->second;
          X2 += i->second * i->second;
        }
        const double a = std::max(0.0, (Loss - wX) / X2);

        // state
        for (size_t i = 0; i < seq.vs.size(); i++) {
          const utils::crf_sample & s = seq.vs[i];
          if (s.label == vs[i]) continue;

          for (std::vector<int>::const_iterator j = s.positive_features.begin(); j != s.positive_features.end(); j++){
            for (std::vector<int>::const_iterator k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++) {
              const int label = _fb.Feature(*k).label();
              if (label == s.label) { _vl[*k] += a; wsum[*k] += a * rest; }
              if (label == vs[i])   { _vl[*k] -= a; wsum[*k] -= a * rest; }
            }
          }
        }

        // edge
        for (int i = 0; i < int(seq.vs.size()) - 1; i++) {
          const int eid0 = edge_feature_id(seq.vs[i].label, seq.vs[i+1].label);
          double & w0 = _vl[eid0];
          w0 += a;
          wsum[eid0] += a * rest;
          edge_weight(seq.vs[i].label, seq.vs[i+1].label) = exp(w0);

          const int eid1 = edge_feature_id(vs[i], vs[i+1]);
          double & w1 = _vl[eid1];
          w1 -= a;
          wsum[eid1] -= a * rest;
          edge_weight(vs[i], vs[i+1]) = exp(w1);
        }

      }
      LOG_S(ERROR) << "iter = " << iter << " error_num = " << error_num << std::endl;
      if (error_num == 0) break;
    }

    for (int i = 0; i < dim; i++) {
      _vl[i] = wsum[i] / (iter * _vs.size());
    }


    return 0;
  }

  inline int sign(double x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
  }

  static std::vector<double> pseudo_gradient(const std::vector<double> & x,
                                             const std::vector<double> & grad0,
                                             const double C)
  {
    std::vector<double> grad = grad0;
    for (size_t i = 0; i < x.size(); i++) {
      if (x[i] != 0) {
        grad[i] += C * sign(x[i]);
        continue;
      }
      const double gm = grad0[i] - C;
      if (gm > 0) {
        grad[i] = gm;
        continue;
      }
      const double gp = grad0[i] + C;
      if (gp < 0) {
        grad[i] = gp;
        continue;
      }
      grad[i] = 0;
    }

    return grad;
  }

  /*
    static void l1ball_projection(std::vector<double> & v, const double z)
    {
    std::vector<double> v1 = v;
    for (int i = 0; i < v1.size(); i++) v1[i] = abs(v1[i]);

    sort(v1.begin(), v1.end());

    int ro = 1;
    double sum = 0, rosum = 0;
    for (int j = 1; j <= v1.size(); j++) {
    const double u = v1[v1.size() - j];
    sum += u;
    if (u - (sum - z)/j > 0) { ro = j; rosum = sum; }
    }

    const double theta = (rosum - z) / ro;
    //  LOG_S(ERROR) << theta << " ";

    for (int i = 0; i < v.size(); i++) {
    if (v[i] > 0) {
    v[i] = max(0.0, v[i] - theta);
    } else if (v[i] < 0) {
    v[i] = min(0.0, v[i] + theta);
    }
    }
    }
  */

  int crf_model::perform_StochasticGradientDescent()
  {
    const double L1_PSEUDO_GRADIENT = 0;
    //  const double L1_PSEUDO_GRADIENT = 0.00001;
    const std::size_t d = _fb.Size();

    std::vector<int> ri(_vs.size());
    for(std::size_t i=0; i<ri.size(); i++)
      {
        ri[i] = i;
      }

    const int batch_size = 20;

    _vee.assign(_vee.size(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());

    //int iter = 0, k = 0;//, m;
    int k=0;

    const double eta0 = 1.0;
    const double tau = 5.0 * _vs.size() / batch_size;

    for (int iter = 0; iter < 20; iter++) {
      //  for (int iter = 0; iter < 40; iter++) {
      std::vector<double> vme(d, 0);
      initialize_edge_weights();
      //random_shuffle(ri.begin(), ri.end());
      shuffle(ri.begin(), ri.end(), gen);

      int n = 0, ncorrect = 0, ntotal = 0;
      double logl = 0;

      for(int i = 0;; i++)
        {
          const utils::crf_sample_sequence & seq = _vs[ri[i]];
          ntotal += seq.vs.size();
          logl += add_sample_model_expectation(seq, vme, ncorrect);
          add_sample_empirical_expectation(seq, _vee);

          int ri_len = ri.size();

          n++;
          if(n==batch_size || i+1==ri_len)
            {
              // update the weights of the features
              for(std::size_t j = 0; j < d; j++) {
                _vee[j] /= n;
                vme[j] /= n;
              }

              std::vector<double> grad(d);
              for(std::size_t j = 0; j < d; j++) {
                grad[j] = vme[j] - _vee[j];
              }

              const double eta = eta0 * tau / (tau + k);
              if (L1_PSEUDO_GRADIENT == 0) {
                for(std::size_t j = 0; j < d; j++) {
                  _vl[j] -= eta * grad[j];
                }
              } else {
                grad = pseudo_gradient(_vl, grad, L1_PSEUDO_GRADIENT);
                for(std::size_t j = 0; j < d; j++) {
                  const double prev = _vl[j];
                  _vl[j] -= eta * grad[j];
                  if (prev * _vl[j] < 0) _vl[j] = 0;
                }
              }

              //  l1ball_projection(_vl, 500000);

              // reset
              k++;
              n = 0;
              vme.assign(d, 0);

              _vee.assign(_vee.size(), 0);

              initialize_edge_weights();

              if(i+1==ri_len)
                {
                  break;
                }
            }
        }
      logl /= _vs.size();

      LOG_S(ERROR) << "iter = " << iter << " logl = " << logl
                   << " acc = " << (double)ncorrect / ntotal << std::endl;
    }

    return 0;
  }

  void crf_model::lookahead_initialize_state_weights(const utils::crf_sample_sequence & seq)
  {
    std::vector<double> powv(_num_classes);

    for(std::size_t i=0; i<seq.vs.size(); i++)
      {
        powv.assign(_num_classes, 0.0);
        const utils::crf_sample & s = seq.vs[i];

        for(std::vector<int>::const_iterator j = s.positive_features.begin(); j != s.positive_features.end(); j++)
          {
            for (std::vector<int>::const_iterator k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++)
              {
                const double w = _vl[*k];
                powv[_fb.Feature(*k).label()] += w;
              }
          }

        for(int j=0; j<_num_classes; j++)
          {
            state_weight(i, j) = powv[j];
          }

      }
  }

  double crf_model::lookahead_search(const utils::crf_sample_sequence & seq,
                                     std::vector<int> & history,
                                     const int start,
                                     const int max_depth,  const int depth,
                                     double current_score,
                                     std::vector<int> & best_seq,
                                     const bool follow_gold,
                                     const std::vector<int> *forbidden_seq)
  {
    assert(history[HV_OFFSET + start - 1 + depth] >= 0);
    assert(history[HV_OFFSET + start - 1] >= 0);

    if (current_score > 0.001 * DBL_MAX || current_score < -0.001 * DBL_MAX) {
      LOG_S(ERROR) << "error: overflow in lookahead_search()" << std::endl; exit(1);
    }

    //  if (forbidden_seq && depth > 0) {
    if (forbidden_seq && depth == 1) {
      if ( (*forbidden_seq)[depth - 1] != history[HV_OFFSET + start + depth - 1])
        forbidden_seq = NULL;
    }

    // terminal (leaf) node
    if (depth >= max_depth || start + depth >= (int)seq.vs.size()) {
      best_seq.clear();
      if (forbidden_seq) return current_score;
      else               return current_score + PERCEPTRON_MARGIN;
    }

    double m = -DBL_MAX;
    for (int i = 0; i < _num_classes; i++) {
      if (follow_gold && i != seq.vs[start + depth].label) continue;

      double new_score = current_score;
      // edge unigram features (state bigrams)
      new_score += _vl[edge_feature_id(history[HV_OFFSET + start + depth -  1], i)];

      // edge bigram features (state trigrams)
      if (depth + start > 0)
        new_score += _vl[edge_feature_id2(history[HV_OFFSET + start + depth - 2], history[HV_OFFSET + start + depth - 1], i)];

      // edge trigram features (state 4-grams)
      if (USE_EDGE_TRIGRAMS) {
        if (depth + start > 1)
          new_score += _vl[edge_feature_id3(history[HV_OFFSET + start + depth - 3], history[HV_OFFSET + start + depth - 2], history[HV_OFFSET + start + depth - 1], i)];
      }

      // state + observation features
      new_score += state_weight(start + depth, i);

      history[HV_OFFSET + start + depth] = i;

      std::vector<int> tmp_seq;
      const double score = lookahead_search(seq, history, start, max_depth, depth + 1, new_score, tmp_seq, false, forbidden_seq);
      //    const double score = lookahead_search(seq, history, start, max_depth, depth + 1, new_score, tmp_seq, follow_gold, forbidden_seq);
      if (score > m) {
        m = score;
        best_seq.clear();
        best_seq.push_back(i);
        copy(tmp_seq.begin(), tmp_seq.end(), back_inserter(best_seq));
      }
    }

    return m;
  }

  void crf_model::calc_diff(const double val,
                            const utils::crf_sample_sequence & seq,
                            const int start,
                            const std::vector<int> & history,
                            const int depth, const int max_depth,
                            std::map<int, double> & diff)
  {
    if (start + depth == (int)seq.vs.size()) return;
    //  if (depth >= LOOKAHEAD_DEPTH) return;
    if (depth >= max_depth) return;

    const int label = history[HV_OFFSET + start + depth];

    int eid = -1;
    eid = edge_feature_id(history[HV_OFFSET + start + depth -  1], label);
    assert(eid >= 0);
    diff[eid] += val;

    int eid2 = -1;
    eid2 = edge_feature_id2(history[HV_OFFSET + start + depth - 2], history[HV_OFFSET + start + depth - 1], label);
    //  assert(eid2 >= 0);
    if (eid2 >= 0) diff[eid2] += val;

    if (USE_EDGE_TRIGRAMS) {
      const int eid3 = edge_feature_id3(history[HV_OFFSET + start + depth - 3], history[HV_OFFSET + start + depth - 2], history[HV_OFFSET + start + depth - 1], label);
      if (eid3 >= 0) diff[eid3] += val;
    }

    assert(start + depth < (int)seq.vs.size());
    const utils::crf_sample & s = seq.vs[start + depth];
    for (std::vector<int>::const_iterator j = s.positive_features.begin(); j != s.positive_features.end(); j++){
      for (std::vector<int>::const_iterator k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++) {
        if (_fb.Feature(*k).label() == label)
          diff[*k] += val;
      }
    }

    calc_diff(val, seq, start, history, depth + 1, max_depth, diff);
  }

  //*
  static void print_bestsq(const std::vector<int> & bestsq)
  {
    int ind=0;
    for(std::vector<int>::const_iterator i = bestsq.begin(); i != bestsq.end(); i++)
      {
        LOG_S(INFO) << "seq[" << ind++ << "]  => " << *i;
      }
  }
  //*/

  int crf_model::update_weights_sub2(const utils::crf_sample_sequence & seq,
                                     std::vector<int> & history,
                                     const int x,
                                     std::map<int, double> & diff)
  {
    // gold-standard sequence
    std::vector<int> gold_seq;

    const double gold_score = lookahead_search(seq, history, x, LOOKAHEAD_DEPTH, 0, 0, gold_seq, true);
    if(false)
      {
        LOG_S(INFO) << "gold = " << gold_seq.size() << " score = " << gold_score;
        print_bestsq(gold_seq);
      }

    std::vector<int> best_seq;

    const double score = lookahead_search(seq, history, x, LOOKAHEAD_DEPTH, 0, 0, best_seq, false, &gold_seq);
    if(false)
      {
        LOG_S(INFO) << "pred = " << best_seq.size() << " score = " << score;
        print_bestsq(best_seq);
      }

    if(gold_seq.size()==best_seq.size())
      {
        int diff=0;
        for(std::size_t l=0; l<best_seq.size(); l++)
          {
            //LOG_S(INFO) << l << ": " << gold_seq.at(l) << "; " << best_seq.at(l);
            if(gold_seq.at(l)!=best_seq.at(l))
              {
                diff += 1;
              }
          }

        if(diff==0)
          {
            return 0;
          }
      }

    /*
    //  if (best_seq == gold_seq) return 0;
    if (best_seq.front() == gold_seq.front())
    {
    return 0;
    }
    */

    std::map<int, double> vdiff;

    copy(gold_seq.begin(), gold_seq.end(), history.begin() + HV_OFFSET + x);
    calc_diff( 1, seq, x, history, 0, LOOKAHEAD_DEPTH, vdiff);

    copy(best_seq.begin(), best_seq.end(), history.begin() + HV_OFFSET + x);
    calc_diff(-1, seq, x, history, 0, LOOKAHEAD_DEPTH, vdiff);

    for(std::map<int, double>::const_iterator j = vdiff.begin(); j != vdiff.end(); j++)
      {
        diff[j->first] += j->second;
      }

    return 1;
  }


  int crf_model::lookaheadtrain_sentence(const utils::crf_sample_sequence & seq, int & t, std::vector<double> & wa)
  {
    // lookahead_initialize_edge_weights();  // to be removed
    lookahead_initialize_state_weights(seq);

    const int len = seq.vs.size();
    //LOG_S(INFO) << "sequence-length: " << len;

    std::vector<int> history(len + HV_OFFSET, -1);
    fill(history.begin(), history.begin() + HV_OFFSET, _num_classes); // BOS

    int error_num = 0;
    for (int x = 0; x < len; x++)
      {
        std::map<int, double> diff;

        error_num += update_weights_sub2(seq, history, x, diff);
        history[HV_OFFSET + x] = seq.vs[x].label;

        for(auto i = diff.begin(); i != diff.end(); i++)
          {
            //LOG_S(INFO) << "(" << i->first << ", " << i->second << ") ";

            const double v = 1.0 * i->second;
            _vl[i->first] += v;
            wa[i->first] += t * v;
          }

        t++;
      }

    return error_num;
  }

  double crf_model::heldout_lookahead_error()
  {
    int nerrors = 0, total_len = 0;

    for(auto itr=validation_samples.begin(); itr!=validation_samples.end(); itr++)
      {
        total_len += itr->vs.size();

        std::vector<int> vs(itr->vs.size());
        decode_lookahead_sentence(*itr, vs);

        for(std::size_t j=0; j<vs.size(); j++)
          {
            if (vs[j] != itr->vs[j].label)
              {
                nerrors++;
              }
          }
      }
    _heldout_error = (double)nerrors / total_len;

    return 0;
  }

  int crf_model::perform_LookaheadTraining(int epochs)
  {
    LOG_S(INFO) << __FILE__ << ":" << __LINE__;
    LOG_S(INFO) << "lookahead depth: " << LOOKAHEAD_DEPTH << ", "
                << "perceptron margin: " << PERCEPTRON_MARGIN << ", "
                << "perceptron niter: " << PERCEPTRON_NITER;

    //LOG_S(ERROR) << "lookahead depth = " << LOOKAHEAD_DEPTH << std::endl;
    //LOG_S(ERROR) << "perceptron margin = " << PERCEPTRON_MARGIN << std::endl;
    //LOG_S(ERROR) << "perceptron niter = " << PERCEPTRON_NITER << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());

    const int dim = _fb.Size();

    std::vector<double> wa(dim, 0);

    int t = 1;

    int iter = 0;
    //while (iter < PERCEPTRON_NITER)
    while (iter < epochs)
      {
        iter++;

        std::vector<int> r(_vs.size());
        for (int i = 0; i < (int)_vs.size(); i++)
          {
            r[i] = i;
          }

        shuffle(r.begin(), r.end(), gen);

        int error_num=0;
        double train_perfect_cnt=0.0;

        for(int i=0; i<(int)_vs.size(); i++)
          {
            const utils::crf_sample_sequence& seq = _vs[r[i]];

            int error_cnt = lookaheadtrain_sentence(seq, t, wa);
            error_num += error_cnt;

            train_perfect_cnt += error_cnt==0? 1.0:0.0;
          }

        train_perfect_cnt/= double(_vs.size());

        if(validation_samples.size() > 0)
          {
            double val_perfect_cnt = 0;

            for(std::size_t l=0; l<validation_samples.size(); l++)
              {
                const utils::crf_sample_sequence& seq = validation_samples.at(l);

                std::vector<int> pred={};
                pred.resize(seq.vs.size());

                decode_lookahead_sentence(seq, pred);

                int error_cnt=0;
                for(std::size_t k=0; k<pred.size(); k++)
                  {
                    if(pred.at(k)!=seq.vs.at(k).label)
                      {
                        error_cnt += 1;
                      }
                  }

                val_perfect_cnt += error_cnt==0? 1.0:0.0;
              }

            val_perfect_cnt/= double(validation_samples.size());

            LOG_S(INFO) << "iter: " << std::setw(3) << iter << ", " << std::setprecision(4)
                        << "train-%-perfect: " << std::setw(6) << 100.0*train_perfect_cnt<< ", "
                        << "valid-%-perfect: " << std::setw(6) << 100.0*val_perfect_cnt;
          }
        else
          {
            LOG_S(INFO) << "iter: " << std::setw(3) << iter << ", " << std::setprecision(4)
                        << "train-%-perfect: " << std::setw(6) << 100.0*train_perfect_cnt;
          }

        if(error_num == 0)
          {
            break;
          }
      }

    for(int i=0; i<dim; i++)
      {
        _vl[i] -= wa[i] / t;
      }

    return 0;
  }

  int crf_model::decode_lookahead_sentence(const utils::crf_sample_sequence & seq, std::vector<int> & vs)
  {
    //    lookahead_initialize_edge_weights();  // to be removed
    lookahead_initialize_state_weights(seq);

    const int len = seq.vs.size();

    std::vector<int> history(len + HV_OFFSET, -1);
    fill(history.begin(), history.begin() + HV_OFFSET, _num_classes); // BOS
    int error_num = 0;
    for (int x = 0; x < len; x++) {

      std::vector<int> bestsq;

      /*const double score = */
      lookahead_search(seq, history, x, LOOKAHEAD_DEPTH, 0, 0, bestsq);

      vs[x] = bestsq.front();
      history[HV_OFFSET + x] = vs[x];
    }

    return error_num;
  }

  void crf_model::decode_lookahead(utils::crf_state_sequence& s0)
  {
    if(s0.vs.size() >= MAX_LEN)
      {
        LOG_S(ERROR) << "sequence is too long: " << MAX_LEN << " > " << s0.vs.size();
        return;
      }

    utils::crf_sample_sequence seq;

    for(auto i=s0.vs.begin(); i!=s0.vs.end(); i++)
      {
        utils::crf_sample s;

        for(auto j = i->features.begin(); j != i->features.end(); j++)
          {
            const int id = _featurename_bag.Id(*j);

            if(id >= 0)
              {
                s.positive_features.push_back(id);
              }
          }

        seq.vs.push_back(s);
      }

    std::vector<int> vs(seq.vs.size());
    decode_lookahead_sentence(seq, vs);

    for(std::size_t i=0; i<seq.vs.size(); i++)
      {
        s0.vs[i].label = _label_bag.Str(vs[i]);
      }
  }

}

#endif
