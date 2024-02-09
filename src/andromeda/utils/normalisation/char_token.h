//-*-C++-*-

/*
#ifndef ANDROMEDA_UTILS_CHAR_TOKEN_H_
#define ANDROMEDA_UTILS_CHAR_TOKEN_H_

namespace andromeda
{
  namespace utils
  {
    struct char_token
    {
    public:

      uint32_t    orig_int;
      std::string orig_str;

      std::vector<uint32_t> norm_ints;
      std::string norm_str;

      std::string label;

    public:

      char_token():
	orig_int(-1),
        orig_str("__null__"),

        norm_ints(std::vector<uint32_t>({})),
        norm_str("__null__"),

	label("__null__")
      {}
      
      char_token(uint32_t orig_int, std::string orig_str,
                 std::vector<uint32_t> norm_ints, std::string norm_str,
                 std::string label):
        orig_int(orig_int),
        orig_str(orig_str),

        norm_ints(norm_ints),
        norm_str(norm_str),

	label(label)
      {}
    };

  }

}

#endif
*/
