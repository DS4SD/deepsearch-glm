//-*-C++-*-

#ifndef ANDROMEDA_CRF_PATH_H
#define ANDROMEDA_CRF_PATH_H

#include <string>

namespace andromeda_crf
{

  namespace utils
  {
    
    struct crf_path
    {
      double score;
      double new_score;
      std::vector<int> vs;

      crf_path(const double s, const std::vector<int> & v):
        score(s),
        vs(v)
      {};

      bool operator<(const crf_path & p) const
      {
        return score > p.score;
      }

      std::string str() const
      {
        //char buf[100];
        //sprintf(buf, "%f\t", score);
        //std::string s(buf);

        std::stringstream ss;
        ss << std::setw(16) << score;

        for (std::vector<int>::const_iterator i = vs.begin(); i != vs.end(); i++) {
          //char buf[100];
          //sprintf(buf, "%d ", *i);
          //s += std::string(buf);

          ss << std::setw(8) << *i;
        }

        return ss.str();
      }

    };

  }

}

#endif
