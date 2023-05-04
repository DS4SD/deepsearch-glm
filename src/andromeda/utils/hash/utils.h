//-*-C++-*-

#ifndef ANDROMEDA_UTILS_HASH_UTILS_H_
#define ANDROMEDA_UTILS_HASH_UTILS_H_

namespace andromeda
{
  namespace utils
  {

    // https://stackoverflow.com/questions/5085915/what-is-the-best-hash-function-for-uint64-t-keys-ranging-from-0-to-its-max-value
    static inline uint64_t murmerhash3(uint64_t val)
    {
      uint64_t hash = val;

      hash ^= hash >> 33;
      hash *= 0xff51afd7ed558ccd;
      hash ^= hash >> 33;
      hash *= 0xc4ceb9fe1a85ec53;
      hash ^= hash >> 33;

      return hash;
    }

    static inline uint64_t combine_hash(uint64_t hash, uint64_t val)
    {
      hash ^= val + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      return hash;
    }

    static uint64_t to_hash(const std::string& text)
    {
      const static std::string seed = "QWERTYUIOP!@#$%^&*()_+qwertyuiop";
      std::string name = seed + text;
      
      uint64_t hash = std::hash<std::string_view>{}(name.c_str());
      return hash;
    }

    static uint64_t to_hash(const std::vector<uint64_t>& path)
    {
      switch(path.size())
        {
        case 0:
	  {
	    LOG_S(FATAL) << "hashing path of length 0";
	    return -1;
	  }
	  
        case 1:
	  {
	    return path.at(0);
	  }

        default:
          {
            uint64_t hash = path.size();
            hash = utils::murmerhash3(hash);

            for(uint64_t chash:path)
              {
                hash = utils::combine_hash(hash, chash);
              }

            return hash;
          }
        }
    }
    
  }

}

#endif
