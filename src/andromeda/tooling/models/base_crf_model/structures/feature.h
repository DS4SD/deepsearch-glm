//-*-C++-*-

#ifndef ANDROMEDA_CRF_FEATURE_H
#define ANDROMEDA_CRF_FEATURE_H

namespace andromeda_crf
{
  namespace utils
  {
    class crf_feature
    {
    public:

      typedef unsigned int feature_body_type;

      const static inline int MAX_LABEL_TYPES = 50;
      
    public:

      crf_feature(const int l, const int f);
      
      int label() const;
      int feature() const;

      feature_body_type body() const;

    private:

      feature_body_type _body;
    };

    crf_feature::crf_feature(const int l, const int f):
      _body((f << 8) + l)
    {
      assert(l >= 0 && l <= MAX_LABEL_TYPES);
      assert(f >= 0 && f <= 0xffffff);
    }

    int crf_feature::label()   const
    {
      return _body & 0xff;
    }
    
    int crf_feature::feature() const
    {
      return _body >> 8;
    }
    
    typename crf_feature::feature_body_type crf_feature::body() const
    {
      return _body;
    }    
    
  }

}

#endif
