//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_STATE_H_
#define ANDROMEDA_BASE_CRF_STATE_H_

namespace andromeda_crf
{
  namespace utils
  {
    class crf_state
    {
    public:

      const static inline std::string undef_label = "__undef__";
      
    public:

      crf_state();

      crf_state(const std::string& l);

      void set_label(const std::string& l);
      
      void add_feature(const std::string& f);

    public:

      std::string label;
      std::vector<std::string> features;
    };

    crf_state::crf_state():
      label(undef_label)
    {};

    crf_state::crf_state(const std::string& label):
      label(label)
    {};
    
    void crf_state::set_label(const std::string& l)
    {
      label = l;
    }

    void crf_state::add_feature(const std::string& f)
    {
      if(f.find_first_of('\t') != std::string::npos)
	{
	  LOG_S(FATAL) << "error: illegal characters in a feature string";
	}
      
      features.push_back(f);
    }
    
  }

}

#endif
