//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_TOKENS_WORD_TOKEN_H_
#define ANDROMEDA_STRUCTS_TOKENS_WORD_TOKEN_H_

namespace andromeda
{

  class word_token: public base_types
  {
  public:

    /* The DEFAULT_POS != UNDEF_POS for a specific reason:
     * 
     * DEFAULT_POS is used to define the concept of a `word`-token (eg `report` can be a `noun` or `verb`)
     * UNDEF_POS is used to signal that the POS-tagger could not determine the POS of a word-token
     *
     * --> in order to distinguish in the GLM the node versus the word (i.e. hash is different between
     *     node-token and word-token), we need to distinguish between a default POS and an undefined POS.
     */

    const static inline std::vector<std::string> HEADERS = {"hash", "char_i", "char_j",
							    "word-index",
							    "pos", "tag", "known",
							    "word", "original",
							    "inds", "subws"};

    const static inline std::string DEFAULT="__default__";
    
    const static inline std::string DEFAULT_WORD=DEFAULT;
    const static inline std::string DEFAULT_POS=DEFAULT;

    const static inline std::string UNDEF_POS="__undef__";
    const static inline std::string UNDEF_TAG="";

    const static inline std::string SPACE="▁";
    
  public:

    word_token(std::string word);

    word_token(index_type i,
               std::string word);

    word_token(index_type i,
	       index_type j,
               std::string word);
    
    word_token(std::string word,
               std::string pos);

    word_token(index_type i,
               std::string word,
               std::string pos);

    word_token(index_type i, index_type j,
               std::string word, std::string pos,
	       std::set<std::string> tags);
    
    word_token(hash_type hash, index_type i, index_type j,
	       std::string pos, std::set<std::string> tags, bool known,
	       std::string word,
	       std::vector<int> inds, std::vector<std::string> subw);

    const static std::string get_space() { return SPACE; }
    
    bool has_default_pos() { return DEFAULT_POS==pos; }
    bool has_default_word() { return DEFAULT_WORD==word; }

    bool get_start_with_space() { return start_with_space; }
    
    hash_type get_hash() const { return hash; }
    
    range_type get_rng() const { return rng; };
    index_type get_rng(index_type l) const { return rng[l]; };

    std::string get_word(bool include_space=false, std::string space=SPACE) const;// { return word; }
    std::string get_orig(const std::string& text) const { return text.substr(rng[0], rng[1]-rng[0]); }

    std::string get_pos() const { return pos; } // part-of-speech
    std::set<std::string> get_tags() const { return tags; } // tags

    std::vector<int> get_inds() const { return inds; }
    std::vector<std::string> get_subws() const { return subws; }
    
    void set_word(std::string word);    
    void set_pos(std::string pos);
    void set_tag(std::string tag);
    void remove_tag(std::string tag);
    void set_known(bool known);

    void set_start_with_space(bool val) { start_with_space = val;}
    
    void set_inds(std::vector<int> inds) { this->inds = inds; }
    void set_subws(std::vector<std::string> subws) { this->subws = subws; }
    
    bool has_tag(std::string tag) const;
    bool is_known();
      
    word_token get_word_token();
    word_token get_pos_token();

  private:
    
    void verify();

  private:

    hash_type hash;
    range_type rng;

    std::string word;
    std::string pos; // part-of-speech

    bool known; // not out-of-vocabulary
    bool start_with_space;
    
    std::set<std::string> tags;

    std::vector<int> inds;
    std::vector<std::string> subws;
  };

  word_token::word_token(std::string word):
    hash(utils::to_reproducible_hash(word)),
    rng({0,word.size()}),

    word(word),
    pos(UNDEF_POS),

    known(false),    
    start_with_space(false),
    
    tags({}),

    inds({}),
    subws({})
  {
    verify();
  }

  word_token::word_token(index_type i,
                         std::string word):
    hash(utils::to_reproducible_hash(word)),
    rng({i,i+word.size()}),

    word(word),
    pos(UNDEF_POS),
    
    known(false),
    start_with_space(false),

    tags({}),

    inds({}),
    subws({})
  {
    verify();
  }

  word_token::word_token(index_type i,
			 index_type j,
                         std::string word):
    hash(utils::to_reproducible_hash(word)),
    rng({i,j}),

    word(word),
    pos(UNDEF_POS),

    known(false),    
    start_with_space(false),
    
    tags({}),

    inds({}),
    subws({})
  {
    verify();
  }  

  word_token::word_token(std::string word,
                         std::string pos):
    hash(utils::to_reproducible_hash(word)),
    rng({0,word.size()}),

    word(word),
    pos(pos),
    
    known(false),    
    start_with_space(false),

    tags({}),

    inds({}),
    subws({})
  {
    verify();
  }

  word_token::word_token(index_type i,
                         std::string word,
                         std::string pos):
    hash(utils::to_reproducible_hash(word)),
    rng({i,i+word.size()}),

    word(word),
    pos(pos),

    known(false),    
    start_with_space(false),
    
    tags({}),

    inds({}),
    subws({})
  {
    verify();
  }
  
  word_token::word_token(index_type i, index_type j,
                         std::string word, std::string pos,
			 std::set<std::string> tags):
    hash(utils::to_reproducible_hash(word)),
    rng({i,j}),

    word(word),
    pos(pos),

    known(false),    
    start_with_space(false),
    
    tags(tags),

    inds({}),
    subws({})
  {
    verify();
  }

  word_token::word_token(hash_type hash, index_type i, index_type j,
			 std::string pos, std::set<std::string> tags, bool known,
			 std::string word,
			 std::vector<int> inds, std::vector<std::string> subws):
    hash(hash),
    rng({i,j}),
    
    word(word),
    pos(pos),
    
    known(known),
    start_with_space(false),

    tags(tags),

    inds(inds),
    subws(subws)
  {
    verify();
  }    
  
  void word_token::verify()
  {
    if(word.size()>48)
      {
        word = DEFAULT_WORD;
        pos = UNDEF_POS;
      }

    if(pos=="" or pos=="NULL")
      {
	pos = UNDEF_POS;
      }
  }
  
  void word_token::set_word(std::string word)
  {
    this->word = word;
  }
  
  void word_token::set_pos(std::string pos)
  {
    this->pos = pos;
    verify();
  }

  void word_token::set_known(bool known)
  {
    this->known = known;
  }

  void word_token::set_tag(std::string tag)
  {
    this->tags.insert(tag);
  }

  void word_token::remove_tag(std::string tag)
  {
    this->tags.erase(tag);
  }

  bool word_token::has_tag(std::string tag) const
  {
    return ((this->tags.count(tag))>0);
  }

  std::string word_token::get_word(bool include_space, std::string space) const
  {
    if(include_space and start_with_space)
      {
	return space+word;
      }

    return word;
  }
  
  word_token word_token::get_word_token()
  {
    word_token token(rng[0], word, DEFAULT);
    return token;
  }

  word_token word_token::get_pos_token()
  {
    word_token token(rng[0], DEFAULT, pos);
    return token;
  }

  bool word_token::is_known()
  {
    return known;
  }
  
}

#endif
