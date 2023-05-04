//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_SEQUENCE_H_
#define ANDROMEDA_BASE_CRF_SEQUENCE_H_

#include <vector>

namespace andromeda_crf
{
  namespace utils
  {
    class crf_state_sequence
    {
    public:

      void add_state(const crf_state& s); 

    public:

      std::vector<utils::crf_state> vs;
    };

    void crf_state_sequence::add_state(const crf_state& s)
    {
      vs.push_back(s);
    }
    
  }

}

#endif
