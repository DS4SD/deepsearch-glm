//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_UTILS_H_
#define ANDROMEDA_BASE_CRF_UTILS_H_

#include <sys/time.h>
#include <stdio.h>
#include <fstream>
#include <map>
#include <list>
#include <iostream>
#include <sstream>
#include <cmath>
#include <set>

namespace andromeda_crf
{
  // FIXME ...
  std::multimap<std::string, std::string> WNdic;

  static std::string normalize(const std::string& s)
  {
    std::string tmp(s);

    for(std::size_t i=0; i<tmp.size(); i++)
      {
        tmp[i] = tolower(tmp[i]);
        if (isdigit(tmp[i]))
          {
            tmp[i] = '#';
          }
      }

    //if (tmp[tmp.size()-1] == 's') tmp = tmp.substr(0, tmp.size()-1);

    return tmp;
  }

  //--------------------------------------------------------------------
  // If you want to use stepp as a chunker, use this function instead
  // of the original crfstate().
  // Also, make sure that you use -f option both in training and testing
  //--------------------------------------------------------------------
  /*
    static CRF_State
    crfstate(const std::vector<Token>&vt, int i)
    {
    CRF_State sample(vt[i].pos);

    std::string posm1 = "!BOS!", strm1 = "!BOS!"; // -1: previous position
    std::string pos0,  str0;                      //  0: current position
    std::string posp1 = "!EOS!", strp1 = "!EOS!"; // +1: next position

    std::string::size_type p = vt[i].str.find_last_of('/');
    str0 = vt[i].str.substr(0, p);
    pos0 = vt[i].str.substr(p+1);

    if (i >= 1) {
    std::string::size_type p = vt[i-1].str.find_last_of('/');
    strm1 = vt[i-1].str.substr(0, p);
    posm1 = vt[i-1].str.substr(p+1);
    }

    if (i < (int)vt.size() - 1) {
    std::string::size_type p = vt[i+1].str.find_last_of('/');
    strp1 = vt[i+1].str.substr(0, p);
    posp1 = vt[i+1].str.substr(p+1);
    }

    sample.add_feature("W0_" + str0);
    sample.add_feature("P0_" + pos0);

    sample.add_feature("W-1_" + strm1);
    sample.add_feature("P-1_" + posm1);

    sample.add_feature("W+1_" + strp1);
    sample.add_feature("P+1_" + posp1);

    //  cout << str0 << pos0 << endl;
    //  exit(0);

    return sample;
    }
  */

  static utils::crf_state create_crfstate(const std::vector<utils::crf_token>& vt, int i)
  {
    utils::crf_state sample;

    std::string str = vt[i].text;
    //  std::string str = normalize(vt[i].str);

    sample.label = vt[i].true_label;

    sample.add_feature("W0_" + vt[i].text);

    sample.add_feature("NW0_" + normalize(str));

    std::string prestr = "BOS";
    if (i > 0) prestr = vt[i-1].text;
    //  if (i > 0) prestr = normalize(vt[i-1].text);

    std::string prestr2 = "BOS";
    if (i > 1) prestr2 = vt[i-2].text;
    //  if (i > 1) prestr2 = normalize(vt[i-2].text);

    std::string poststr = "EOS";
    if (i < (int)vt.size()-1) poststr = vt[i+1].text;
    //  if (i < (int)vt.size()-1) poststr = normalize(vt[i+1].text);

    std::string poststr2 = "EOS";
    if (i < (int)vt.size()-2) poststr2 = vt[i+2].text;
    //  if (i < (int)vt.size()-2) poststr2 = normalize(vt[i+2].text);


    sample.add_feature("W-1_" + prestr);
    sample.add_feature("W+1_" + poststr);

    sample.add_feature("W-2_" + prestr2);
    sample.add_feature("W+2_" + poststr2);

    sample.add_feature("W-10_" + prestr + "_" + str);
    sample.add_feature("W0+1_" + str  + "_" + poststr);
    sample.add_feature("W-1+1_" + prestr  + "_" + poststr);

    //sample.add_feature("W-10+1_" + prestr  + "_" + str + "_" + poststr);

    //  sample.add_feature("W-2-1_" + prestr2  + "_" + prestr);
    //  sample.add_feature("W+1+2_" + poststr  + "_" + poststr2);

    // train = 10000 no effect
    //  if (i > 0 && prestr.size() >= 3)
    //    sample.add_feature("W-1S_" + prestr.substr(prestr.size()-3));
    //  if (i < (int)vt.size()-1 && poststr.size() >= 3)
    //    sample.add_feature("W+1S_" + poststr.substr(poststr.size()-3));

    // sentence type
    //  sample.add_feature("ST_" + vt[vt.size()-1].text);

    for (size_t j = 1; j <= 10; j++) {
      //char buf[1000];
      //    if (str.size() > j+1) {

      if (str.size() >= j) {

        std::string tmp = "SUF";
        tmp += std::to_string(j);
        tmp += "_";
        tmp += str.substr(str.size() - j);

        sample.add_feature(tmp);

        /*
          sprintf(buf, "SUF%d_%s", (int)j, str.substr(str.size() - j).c_str());
          std::string _ = buf;

          if(_!=tmp)
          {
          LOG_S(ERROR) << "features are different: \n"
          << "old: " << _ << "\n"
          << "new: " << tmp;
          }
          else
          {
          LOG_S(INFO) << "features are same: \n"
          << "old: " << _ << "\n"
          << "new: " << tmp;
          }

          sample.add_feature(buf);
        */
      }
      //    if (str.size() > j+1) {
      if (str.size() >= j) {

        std::string tmp = "PRE";
        tmp += std::to_string(j);
        tmp += "_";
        tmp += str.substr(0, j);

        sample.add_feature(tmp);

        // original code ... (kept for legacy purposes for now)
        /*
          sprintf(buf, "PRE%d_%s", (int)j, str.substr(0, j).c_str());
          std::string _ = buf;

          if(_!=tmp)
          {
          LOG_S(ERROR) << "features are different: \n"
          << "old: " << _ << "\n"
          << "new: " << tmp;
          }
          else
          {
          LOG_S(INFO) << "features are same: \n"
          << "old: " << _ << "\n"
          << "new: " << tmp;
          }

          sample.add_feature(buf);
        */
      }
    }

    for (size_t j = 0; j < str.size(); j++) {
      if (isdigit(str[j])) {
        sample.add_feature("CTN_NUM");
        break;
      }
    }
    for (size_t j = 0; j < str.size(); j++) {
      if (isupper(str[j])) {
        sample.add_feature("CTN_UPP");
        break;
      }
    }
    for (size_t j = 0; j < str.size(); j++) {
      if (str[j] == '-') {
        sample.add_feature("CTN_HPN");
        break;
      }
    }
    bool allupper = true;
    for (size_t j = 0; j < str.size(); j++) {
      if (!isupper(str[j])) {
        allupper = false;
        break;
      }
    }

    if(allupper)
      {
        sample.add_feature("ALL_UPP");
      }

    if(WNdic.size() > 0)
      {
        const std::string n = normalize(str);

        for(auto i = WNdic.lower_bound(n); i != WNdic.upper_bound(n); i++)
          {
            sample.add_feature("WN_" + i->second);
          }
      }
    //  for (int j = 0; j < vt.size(); j++)
    //    cout << vt[j].text << " ";
    //  cout << endl;
    //  cout << i << endl;

    //  cout << sample.label << "\t";
    //  for (std::vector<std::string>::const_iterator j = sample.features.begin(); j != sample.features.end(); j++) {
    //      cout << *j << " ";
    //  }
    //  cout << endl;

    return sample;
  }

  //typedef std::vector<utils::crf_token> Sentence;

  int crf_train(const crf_model::OptimizationMethod method,
                crf_model& m, const std::vector<std::vector<utils::crf_token> >& vs,
                double gaussian, const bool use_l1)
  {
    if(method != crf_model::BFGS && use_l1)
      {
        LOG_S(FATAL) << "L1 regularization is not supported in this mode. Please use other method.";
      }

    for(auto i = vs.begin(); i != vs.end(); i++)
      {
        const std::vector<utils::crf_token>& s = *i;

        utils::crf_state_sequence cs;

        for(std::size_t j = 0; j < s.size(); j++)
          {
            cs.add_state(create_crfstate(s, j));
          }

        m.add_training_sample(cs);
      }

    if(use_l1)
      {
        m.train(method, 0, 0, 1.0);
      }
    else
      {
        m.train(method, 0, gaussian);
      }

    return 0;
  }

  /*
  int crf_train(const crf_model::OptimizationMethod method,
                crf_model& model,
                const std::vector<std::vector<utils::crf_token> >& train_samples,
                const std::vector<std::vector<utils::crf_token> >& val_samples,
                double gaussian)
  {
    for(auto itr = train_samples.begin(); itr!=train_samples.end(); itr++)
      {
        const std::vector<utils::crf_token>& s = *itr;

        utils::crf_state_sequence cs;
        for(std::size_t j=0; j<s.size(); j++)
          {
            cs.add_state(create_crfstate(s, j));
          }

        model.add_training_sample(cs);
      }

    for(auto itr = val_samples.begin(); itr!=val_samples.end(); itr++)
      {
        const std::vector<utils::crf_token>& s = *itr;

        utils::crf_state_sequence cs;
        for(std::size_t j=0; j<s.size(); j++)
          {
            cs.add_state(create_crfstate(s, j));
          }

        model.add_validation_sample(cs);
      }

    model.train(method, 0, gaussian);

    return 0;
  }
  */
  
  void crf_decode_lookahead(std::vector<utils::crf_token>& s, crf_model& m,
                            std::vector< std::map<std::string, double> >& tagp)
  {
    utils::crf_state_sequence cs;
    for (size_t j = 0; j < s.size(); j++)
      {
        cs.add_state(create_crfstate(s, j));
      }

    m.decode_lookahead(cs);

    tagp.clear();
    for(std::size_t k=0; k<s.size(); k++)
      {
        s[k].pred_label = cs.vs[k].label;

        std::map<std::string, double> vp;
        vp[s[k].pred_label] = 1.0;

        tagp.push_back(vp);
      }
  }

  void crf_decode_forward_backward(std::vector<utils::crf_token> & s, crf_model& m,
                                   std::vector< std::map<std::string, double> >& tagp)
  {
    utils::crf_state_sequence cs;
    for(std::size_t j=0; j<s.size(); j++)
      {
        cs.add_state(create_crfstate(s, j));
      }

    m.decode_forward_backward(cs, tagp);
    //  m.decode_viterbi(cs);

    for(std::size_t k=0; k<s.size(); k++)
      {
        s[k].pred_label = cs.vs[k].label;
      }
  }

  void crf_decode_nbest(std::vector<utils::crf_token>& s, crf_model& m,
                        std::vector<std::pair<double, std::vector<std::string> > >& nbest_seqs, int n)
  {
    utils::crf_state_sequence cs;
    for(std::size_t j=0; j<s.size(); j++)
      {
        cs.add_state(create_crfstate(s, j));
      }

    m.decode_nbest(cs, nbest_seqs, n, 0);

    for(std::size_t k=0; k<s.size(); k++)
      {
        s[k].pred_label = cs.vs[k].label;
      }
  }

}

#endif
