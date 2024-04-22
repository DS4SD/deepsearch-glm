//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ELEMENTS_TEXT_ELEMENT_H_
#define ANDROMEDA_STRUCTS_ELEMENTS_TEXT_ELEMENT_H_

namespace andromeda
{
  class text_element: public base_types
  {
  protected:

    //typedef std::tuple<index_type, index_type, std::string> candidate_type;
    struct replacement_token
    {
      replacement_token(index_type word_ind_0,
			index_type word_ind_1,
			std::string word,
			std::vector<int> inds,
			std::vector<std::string> subws):
	word_ind_0(word_ind_0),
	word_ind_1(word_ind_1),
	word(word),
	inds(inds),
	subws(subws)
      {}
      
      index_type word_ind_0, word_ind_1;
      std::string word;
      std::vector<int> inds;
      std::vector<std::string> subws;
    };
    typedef replacement_token candidate_type;

  public:
    
    const static inline std::string text_lbl = "text";
    const static inline std::string orig_lbl = "orig";

    const static inline std::string text_hash_lbl = "text_hash";
   
    const static inline std::string char_tokens_lbl = "char_tokens"; 
    const static inline std::string word_tokens_lbl = "word_tokens"; 
    
  public:

    static std::shared_ptr<utils::char_normaliser> create_char_normaliser(bool verbose);
    static std::shared_ptr<utils::text_normaliser> create_text_normaliser(bool verbose);

  public:

    text_element();

    bool is_valid();

    std::size_t get_len() const { return text.size(); } // number-of-chars
    std::size_t get_dst() const { return utf8::distance(text.c_str(), text.c_str()+text.size()); }

    bool is_text_valid() { return text_valid; }
    
    void clear();

    hash_type get_text_hash() const { return text_hash; }

    std::size_t get_num_wtokens() const { return word_tokens.size(); }
    const word_token& get_wtoken(std::size_t i) const { return word_tokens.at(i); }

    std::vector<word_token>& get_word_tokens() { return word_tokens; }
    //

    void init_pos() { for(auto& wtoken:word_tokens) { wtoken.set_pos(word_token::UNDEF_POS); } }

    void set_pos(std::size_t i, std::string pos) { word_tokens.at(i).set_pos(pos); }
    void set_tag(std::size_t i, std::string tag) { word_tokens.at(i).set_tag(tag); }
    void set_word(std::size_t i, std::string wrd) { word_tokens.at(i).set_word(wrd); } 
    
    bool set_text(const std::string& ctext);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);
    
    bool set(const std::string& ctext,
             std::shared_ptr<utils::char_normaliser> char_normaliser,
             std::shared_ptr<utils::text_normaliser> text_normaliser);
    
    std::string from_char_range(range_type char_range);
    std::string from_ctok_range(range_type ctok_range);
    std::string from_wtok_range(range_type wtok_range);

    // the bare char range (eg coming from PCRE expression) need to be
    // mapped to char (index-)ranges. For ASCII, this is the identity-operation, but
    // for text with unicode, it is not!
    range_type get_char_token_range(range_type char_range);
    range_type get_word_token_range(range_type char_range);

    bool is_a_connected_word_token(range_type inds);

  protected:

    std::string get_text(range_type rng);

    void apply_word_contractions(std::vector<candidate_type>& candidates);

    nlohmann::json _to_json(const std::set<std::string>& filters);
    bool _from_json(const nlohmann::json& json_cell);
    
  private:

    void update_text(std::shared_ptr<utils::text_normaliser> text_normaliser);

    void set_chars(std::shared_ptr<utils::char_normaliser> char_normaliser);
    void set_tokens();

    void contract_char_tokens();
    void contract_word_tokens();

  private:
    
    bool text_valid;
    uint64_t text_hash; // hash of normalised text

  protected:

    std::string orig; // original text
    std::string text; // normalised text (removing confusables)

  protected:
    
    std::vector<char_token> char_tokens;
    std::vector<word_token> word_tokens;
  };

  std::shared_ptr<utils::char_normaliser> text_element::create_char_normaliser(bool verbose)
  {
    return std::make_shared<utils::char_normaliser>(verbose);
  }

  std::shared_ptr<utils::text_normaliser> text_element::create_text_normaliser(bool verbose)
  {
    return std::make_shared<utils::text_normaliser>(verbose);
  }

  text_element::text_element():
    text_valid(true),
    text_hash(-1),

    orig(""),
    text(""),

    char_tokens({}),
    word_tokens({})
  {}

  nlohmann::json text_element::_to_json(const std::set<std::string>& filters)
  {
    nlohmann::json elem = nlohmann::json::object({});

    elem[text_lbl] = text;
    elem[orig_lbl] = orig;

    elem[text_hash_lbl] = text_hash;

    // in the default setting, word-tokens will not be dumped
    if(filters.count(word_tokens_lbl))
      {
        elem[word_tokens_lbl] = andromeda::to_json(word_tokens, text);	
      }
    
    return elem;
  }

  bool text_element::_from_json(const nlohmann::json& elem)
  {
    bool result=true;
    
    this->clear();

    if(elem.count(orig_lbl))
      {
	auto ctext = elem.at(orig_lbl).get<std::string>();
	result = set_text(ctext);
      }
    else if(elem.count(text_lbl))
      {
	auto ctext = elem.at(text_lbl).get<std::string>();
	result = set_text(ctext);	
      }
    else
      {
	LOG_S(WARNING) << "no `orig` or `text` found in text-element: "
		       << elem.dump(2);

	return false;
      }
    
    if(elem.count(word_tokens_lbl))
      {
        const nlohmann::json& json_word_tokens = elem.at(word_tokens_lbl);
        andromeda::from_json(word_tokens, json_word_tokens);	
      }    

    return result;
  }
  
  void text_element::clear()
  {
    text_valid = true;
    text_hash = -1;

    orig="";
    text="";

    char_tokens.clear();
    word_tokens.clear();
  }

  bool text_element::is_valid()
  {
    return text_valid;
  }

  bool text_element::set(const std::string& ctext,
                         std::shared_ptr<utils::char_normaliser> char_normaliser,
                         std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    if(set_text(ctext))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }

  bool text_element::set_text(const std::string& ctext)
  {
    clear();
    
    orig = utils::strip(ctext);
    text = orig;
    
    if(orig.size()==0)
      {
        return false;
      }
    
    auto len = orig.size();
    
    text_valid = utf8::is_valid(orig.c_str(), orig.c_str()+len);
    text_hash = utils::to_reproducible_hash(orig);
    
    return text_valid;
  }

  bool text_element::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                                std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    update_text(text_normaliser);

    auto len = text.size();

    text_valid = utf8::is_valid(text.c_str(), text.c_str()+len);

    if(not text_valid)
      {
        return text_valid;
      }
    
    set_chars(char_normaliser);
    //LOG_S(INFO) << tabulate(char_tokens);
    
    set_tokens();
    //LOG_S(INFO) << tabulate(word_tokens);

    /*
    if(orig.find("``")!=std::string::npos)
      {
        LOG_S(INFO) << "orig: " << orig;
        LOG_S(INFO) << "text: " << text;

        LOG_S(INFO) << "chars: \n" << tabulate(char_tokens);
        LOG_S(INFO) << "words: \n" << tabulate(word_tokens);

        std::string tmp;
        std::cin >> tmp;
      }
    */
    
    return true;
  }

  void text_element::update_text(std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    this->text = this->orig;

    if(text_normaliser!=NULL)
      {
        text_normaliser->normalise(this->text);
      }
  }

  void text_element::set_chars(std::shared_ptr<utils::char_normaliser> char_normaliser)
  {
    char_tokens={};

    auto start = text.c_str();
    auto end = text.c_str()+text.size();

    auto itr = start;
    while(itr!=end)
      {
        uint32_t c = utf8::next(itr, end);

        std::string orig_str="";
        utf8::append(c, std::back_inserter(orig_str));

        if(char_tokens.size()>0 and char_tokens.back().get_norm()=="\\" and (9<=c and c<=10))
          {
            //skip = true;
            char_tokens.pop_back();
          }
        else if(char_normaliser!=NULL)
          {
            std::string norm_str = char_normaliser->get(c, orig_str);
            char_tokens.emplace_back(c, orig_str, norm_str);
          }
        else
          {
            char_tokens.emplace_back(c, orig_str, orig_str);
          }
      }

    std::size_t i=0, d=0;

    std::stringstream ss;
    for(auto& char_token:char_tokens)
      {
        d = char_token.len();
        char_token.set_rng(i,i+d);

        i += d;
        ss << char_token.str();
      }

    text = ss.str();
  }

  void text_element::set_tokens()
  {
    contract_char_tokens();

    contract_word_tokens();

    // internal consistency check ...
    /*
      if(false)
      {
      for(auto& word_token:word_tokens)
      {
      auto rng = word_token.get_rng();

      std::string token = text.substr(rng[0], rng[1]-rng[0]);
      if(token!=word_token.get_word())
      {
      LOG_S(ERROR) << token << " versus " << word_token.get_word();
      }
      }
      }
    */
  }

  void text_element::contract_char_tokens()
  {
    word_tokens={};

    std::size_t l=0, char_l=0;
    while(l<char_tokens.size())
      {
        bool stop=false;

        std::size_t i=l;
        std::size_t j=l;

        std::size_t dst=0;

        std::stringstream ss;
        while(j<char_tokens.size() and (not stop))
          {
            std::string tmp = char_tokens.at(j).str();

	    if(constants::spaces.count(tmp) or
               constants::brackets.count(tmp) or
               constants::punktuation.count(tmp) or
	       constants::numbers.count(tmp))
	      {
		stop = true;
	      }
	    
	    
            if((not stop) or (j-i)==0)
              {
                dst += char_tokens.at(j).len();
                j += 1;

                ss << tmp;
              }

            std::string word = ss.str();
            if(constants::special_words.count(word))
              {
                stop = true;
              }

	    //LOG_S(INFO) << stop << "\t" << tmp << "\t" << ss.str();
          }

        std::string word = ss.str();
        if(constants::spaces.count(word)==0)
          {
            word_tokens.emplace_back(char_l, word);
          }

        l = j;

        char_l += dst;
      }

    // contract all pure numbers (0-9) into integers
    auto curr = word_tokens.begin();
    auto prev = word_tokens.begin();
    while(curr != word_tokens.end())
      {
	if(curr==word_tokens.begin())
	  {
	    curr++;
	  }
	else
	  {
	    auto prev_wrd = prev->get_word();
	    auto curr_wrd = curr->get_word();
	    
	    auto prev_char = prev_wrd.back();
	    auto curr_char = curr_wrd.back();

	    if('0'<=prev_char and prev_char<='9' and
	       '0'<=curr_char and curr_char<='9' and
	       prev->get_rng(1)==curr->get_rng(0))
	      {
		prev_wrd += curr_wrd;
	
		word_token token(prev->get_rng(0), prev_wrd);
		*prev = token;
	
		curr = word_tokens.erase(curr);
	      }
	    else
	      {
		prev++;
		curr++;
	      }
	  }
      }

    for(std::size_t l=1; l<word_tokens.size(); l++)
      {
	if(word_tokens.at(l-1).get_rng(1)==word_tokens.at(l).get_rng(0))
	  {
	    word_tokens.at(l).set_start_with_space(false);
	  }
	else
	  {
	    word_tokens.at(l).set_start_with_space(true);
	  }
      }
  }

  void text_element::contract_word_tokens()
  {
    return;
  }

  bool text_element::is_a_connected_word_token(range_type range)
  {
    for(std::size_t ind=range[0]; ind<range[1]; ind++)
      {
        if(ind+1==word_tokens.size())
          {
            continue;
          }

        if(word_tokens.at(ind+0).get_rng(1) !=
           word_tokens.at(ind+1).get_rng(0))
          {
            return false;
          }
      }

    return true;
  }

  std::string text_element::get_text(range_type rng)
  {
    assert(rng[0]<=rng[1]);
    assert(rng[1]<=char_tokens.size());

    std::string text="";
    for(std::size_t i=rng[0]; i<rng[1]; i++)
      {
        text += char_tokens.at(i).get_norm();
      }

    return text;
  }

  void text_element::apply_word_contractions(std::vector<candidate_type>& candidates)
  {
    const std::string placeholder="__to_be_deleted__";

    // certain candidates are sometimes within a word-token (eg `--` in `Yang--Mills`)
    // To ensure we do not go out of bounds, we remove these candidates.
    for(auto itr=candidates.begin(); itr!=candidates.end(); )
      {
        std::size_t wtok_beg = itr->word_ind_0; //std::get<0>(*itr);
        std::size_t wtok_end = itr->word_ind_1; //std::get<1>(*itr);

        if(wtok_beg>=wtok_end)
          {
            itr = candidates.erase(itr);
          }
        else
          {
            itr++;
          }
      }

    std::sort(candidates.begin(), candidates.end(),
              [](const candidate_type& lhs,
                 const candidate_type& rhs)
              {
                std::size_t lhs_0 = lhs.word_ind_0; //std::get<0>(lhs);
                std::size_t lhs_1 = lhs.word_ind_1; //std::get<1>(lhs);

                std::size_t rhs_0 = rhs.word_ind_0; //std::get<0>(rhs);
                std::size_t rhs_1 = rhs.word_ind_1; //std::get<1>(rhs);

                if(lhs_0==rhs_0)
                  {
                    return lhs_1>rhs_1;
                  }

                return lhs_0<rhs_0;
              });

    for(auto& candidate:candidates)
      {
        std::size_t ind_0 = candidate.word_ind_0; //std::get<0>(candidate);
        std::size_t ind_1 = candidate.word_ind_1; //std::get<1>(candidate);
        std::string word = candidate.word; //std::get<2>(candidate);

        bool overlapping=false;
        for(auto ind=ind_0; ind<ind_1; ind++)
          {
            if(word_tokens.at(ind).has_tag(placeholder))
              {
                overlapping=true;
              }
          }

        if(overlapping)
          {
            continue;
          }

        {
          auto& beg = word_tokens.at(ind_0);
          auto& end = word_tokens.at(ind_1-1);

          word_token token_new(beg.get_rng(0),
                               end.get_rng(1),
                               word);

	  token_new.set_inds(candidate.inds);
	  token_new.set_subws(candidate.subws);
	  
          word_tokens.at(ind_0) = token_new;
        }

        for(auto ind=ind_0+1; ind<ind_1; ind++)
          {
            word_tokens.at(ind).set_tag(placeholder);
          }
      }

    for(auto itr=word_tokens.begin(); itr!=word_tokens.end(); )
      {
        if(itr->has_tag(placeholder))
          {
            itr = word_tokens.erase(itr);
          }
        else
          {
            itr++;
          }
      }
  }

  std::string text_element::from_char_range(range_type char_range)
  {
    std::size_t beg_ = char_range[0];
    std::size_t len_ = char_range[1]-beg_;

    if(char_range[1]<=text.size())
      {
        return text.substr(beg_, len_);
      }

    LOG_S(ERROR) << "char-range is out of bounds: text-length: " << text.size()
                 << " versus char-range: " << char_range[1];
    return "";
  }

  std::string text_element::from_ctok_range(range_type ctok_range)
  {
    std::stringstream ss;
    for(std::size_t l=ctok_range[0]; l<ctok_range[1]; l++)
      {
        ss << char_tokens.at(l).get_norm();
      }

    return ss.str();
  }

  std::string text_element::from_wtok_range(range_type wtok_range)
  {
    std::stringstream ss;
    for(std::size_t l=wtok_range[0]; l<wtok_range[1]; l++)
      {
        if(l>wtok_range[0] and
           (word_tokens.at(l).get_rng(0)-word_tokens.at(l-1).get_rng(1))>0)
          {
            ss << " ";
          }
        ss << word_tokens.at(l).get_word();
      }

    return ss.str();
  }

  /*
   * Here, we find the index-range of a word-token in the char_tokens vector (NOT the char-range
   * in the text). If the text is pure ascii, there will be no difference. However, if there is
   * unicode in the text, there will!
   */
  typename text_element::range_type text_element::get_char_token_range(range_type char_range)
  {
    range_type res={0,0};
    for(std::size_t l=0; l<char_tokens.size(); l++)
      {
        if(char_tokens[l].get_rng(0)<=char_range[0])
          {
            res[0]=l;
          }

        if(char_tokens[l].get_rng(1)<=char_range[1])
          {
            res[1]=l+1;
          }
      }

    return res;
  }

  typename text_element::range_type text_element::get_word_token_range(range_type char_range)
  {
    range_type res={0,0};
    for(std::size_t l=0; l<word_tokens.size(); l++)
      {
        if(word_tokens.at(l).get_rng(0)<=char_range[0])
          {
            res[0]=l;
          }

        if(word_tokens.at(l).get_rng(1)<=char_range[1])
          {
            res[1]=l+1;
          }
      }

    return res;
  }

}

#endif
