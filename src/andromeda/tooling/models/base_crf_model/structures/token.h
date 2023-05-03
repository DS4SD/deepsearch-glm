//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_TOKEN_H_
#define ANDROMEDA_BASE_CRF_TOKEN_H_

#include <string>

namespace andromeda_crf
{
  namespace utils
  {
    class crf_token
    {
    public:

      const static inline std::string undef_label = "__undef__";
      const static inline std::size_t undef_index = -1;

    public:

      crf_token(std::string text, std::string true_label);

      crf_token(std::string text, const std::size_t beg, const size_t end);

      crf_token(std::string text, std::string true_label,
                const std::size_t beg, const std::size_t end);

    public:

      std::string text;

      std::string true_label;
      std::string pred_label;

      double pred_conf;

      std::size_t beg;
      std::size_t end;
    };

    crf_token::crf_token(std::string text,
                         std::string true_label):
      text(text),

      true_label(true_label),
      pred_label(undef_label),
      pred_conf(0.0),

      beg(undef_index),
      end(undef_index)
    {}

    crf_token::crf_token(std::string text,
                         const std::size_t beg,
                         const std::size_t end):
      text(text),

      true_label(undef_label),
      pred_label(undef_label),
      pred_conf(0.0),

      beg(beg),
      end(end)
    {}

    crf_token::crf_token(std::string text,
                         std::string true_label,
                         const std::size_t beg,
                         const std::size_t end):
      text(text),

      true_label(true_label),
      pred_label(undef_label),
      pred_conf(0.0),

      beg(beg),
      end(end)
    {}
  }

  //}

}

#endif
