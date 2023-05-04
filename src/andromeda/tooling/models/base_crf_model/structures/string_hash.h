//-*-C++-*-

#ifndef ANDROMEDA_CRF_STRING_HASH_H
#define ANDROMEDA_CRF_STRING_HASH_H

//#include <vector>
#include <string>
//#include <iostream>

namespace andromeda_crf
{
  namespace utils
  {
    struct hashfun_str
    {
      std::size_t operator()(const std::string& s) const
      {
        assert(sizeof(int) == 4 && sizeof(char) == 1);
        const int* p = reinterpret_cast<const int*>(s.c_str());

	std::size_t v = 0;
        int n = s.size() / 4;

	for (int i = 0; i < n; i++, p++)
	  {
	    //      v ^= *p;
	    v ^= *p << (4 * (i % 2)); // note) 0 <= char < 128  // bug??
	    //        v ^= *p << (i % 2);
	  }
	
        int m = s.size() % 4;
        for (int i = 0; i < m; i++)
	  {
	    v ^= s[4 * n + i] << (i * 8);
	  }
	
        return v;
      }

    };

  }

}

#endif
