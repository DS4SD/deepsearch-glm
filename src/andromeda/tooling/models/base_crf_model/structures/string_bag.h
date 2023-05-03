//-*-C++-*-

#ifndef ANDROMEDA_CRF_STRING_BAG_H
#define ANDROMEDA_CRF_STRING_BAG_H

#include <vector>
#include <string>
#include <iostream>

namespace andromeda_crf
{
  namespace utils
  {
    struct mini_string_bag
    {
      //#ifdef USE_HASH_MAP
      //typedef __gnu_cxx::hash_map<std::string, int, hashfun_str> map_type;
      //typedef std::tr1::unordered_map<std::string, int, hashfun_str> map_type;
      //#else
      typedef std::map<std::string, int> map_type;
      //#endif
      
      int _size;
      map_type str2id;

      mini_string_bag():
	_size(0)
      {}
      
      int Put(const std::string& i)
      {
        map_type::const_iterator j = str2id.find(i);
        if (j == str2id.end()) {
          int id = _size;
          _size++;
          str2id[i] = id;
          return id;
        }
        return j->second;
      }

      int Id(const std::string & i) const {
        map_type::const_iterator j = str2id.find(i);
        if (j == str2id.end())  return -1;
        return j->second;
      }

      int Size() const { return _size; }

      void Clear() { str2id.clear(); _size = 0; }

      map_type::const_iterator begin() const { return str2id.begin(); }
      map_type::const_iterator end()   const { return str2id.end(); }
    };

    struct string_bag:
      public mini_string_bag
    {
      std::vector<std::string> id2str;

      int Put(const std::string & i) {
        map_type::const_iterator j = str2id.find(i);
        if (j == str2id.end()) {
          int id = id2str.size();
          id2str.push_back(i);
          str2id[i] = id;
          return id;
        }
        return j->second;
      }

      std::string Str(const int id) const {
        assert(id >= 0 && id < (int)id2str.size());
        return id2str[id];
      }

      int Size() const { return id2str.size(); }

      void Clear() {
        str2id.clear();
        id2str.clear();
      }
      
    };

  }

}

#endif
