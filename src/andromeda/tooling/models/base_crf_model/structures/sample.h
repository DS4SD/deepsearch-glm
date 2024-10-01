// //-*-C++-*-

// #ifndef ANDROMEDA_CRF_SAMPLE_H
// #define ANDROMEDA_CRF_SAMPLE_H

// namespace andromeda_crf
// {
//   namespace utils
//   {
//     struct crf_sample
//     {
//       int label;
//       std::vector<int> positive_features;
//     };

//   }

// }

// #endif

//-*-C++-*-

#ifndef ANDROMEDA_CRF_SAMPLE_H
#define ANDROMEDA_CRF_SAMPLE_H

#include <vector>

namespace andromeda_crf
{
  namespace utils
  {
    struct crf_sample
    {
      int label = 0;
      std::vector<int> positive_features;

      crf_sample(int lbl = 0) : label(lbl) {}
    };
  }
}

#endif
