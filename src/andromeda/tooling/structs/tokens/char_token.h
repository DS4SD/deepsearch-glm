//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_TOKENS_CHAR_TOKEN_H_
#define ANDROMEDA_STRUCTS_TOKENS_CHAR_TOKEN_H_

namespace andromeda
{
  class char_token: public base_types
  {
  public:

    const static inline std::vector<std::string> HEADERS = {"char_i", "char_j",
							    "len", "char index",
							    "unicode (dec)", "orig", "norm"};
    
    const static inline index_type DEFAULT_INDEX=-1;

  public:

    char_token(uint32_t char_ind,
               std::string orig_str,
               std::string norm_str);

    std::string str() { return norm_str; }
    std::size_t len() { return norm_str.size(); }
    char_ind_type ind() { return char_ind; }

    std::string get_orig() { return orig_str; }
    std::string get_norm() { return norm_str; }

    char_ind_type get_ind() { return char_ind; }

    range_type get_rng() { return rng; };
    index_type get_rng(index_type l) { return rng.at(l); };

    void set_rng(index_type i, index_type j) { rng.at(0)=i; rng.at(1)=j; }

  private:

    uint32_t char_ind;
    std::string orig_str, norm_str;

    range_type rng;
  };

  char_token::char_token(uint32_t char_ind,
                         std::string orig_str,
                         std::string norm_str):
    char_ind(char_ind),
    orig_str(orig_str),
    norm_str(norm_str)
  {
    rng = {DEFAULT_INDEX, DEFAULT_INDEX};
  }

}

#endif
