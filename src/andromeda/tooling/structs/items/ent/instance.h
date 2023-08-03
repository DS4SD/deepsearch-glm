//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_INSTANCE_BASE_H_
#define ANDROMEDA_STRUCTS_ITEMS_INSTANCE_BASE_H_

namespace andromeda
{
  class base_instance: public base_types
  {
  public:

    const static inline hash_type DEFAULT_HASH = -1;

    const static inline index_type DEFAULT_INDEX = -1;
    const static inline range_type DEFAULT_RANGE = {DEFAULT_INDEX, DEFAULT_INDEX};

    const static inline range_type DEFAULT_COOR = {DEFAULT_INDEX, DEFAULT_INDEX};
    const static inline range_type DEFAULT_SPAN = {DEFAULT_INDEX, DEFAULT_INDEX};

    const static inline std::vector<std::string> HEADERS =
      { "type", "subtype",
	"subj_hash", "subj_name", "subj_path",
        "conf",
        "hash", "ihash",
        "coor_i", "coor_j",
        "char_i", "char_j",
        "ctok_i", "ctok_j",
        "wtok_i", "wtok_j",
        "wtok-match",
        "name", "original"
      };

    const static inline std::vector<std::string> TEXT_HEADERS =
      { "type", "subtype",
        "conf",
        "hash", "ihash",
        "char_i", "char_j",
        "ctok_i", "ctok_j",
        "wtok_i", "wtok_j",
        "wtok-match",
        "name", "original"
      };

    const static inline std::vector<std::string> TABLE_HEADERS =
      { "type", "subtype",
        "conf",
        "hash", "ihash",
        "coor_i", "coor_j",
        "char_i", "char_j",
        "ctok_i", "ctok_j",
        "wtok_i", "wtok_j",
        "wtok-match",
        "name", "original"
      };

    const static inline std::vector<std::string> SHORT_TEXT_HEADERS =
      { "type", "subtype",
        "conf",
        "hash", "ihash",
        "char_i", "char_j",
        "wtok-match",
        "name", "original"};

    const static inline std::vector<std::string> SHORT_TABLE_HEADERS =
      { "type", "subtype",
        "conf",
        "hash", "ihash",
        "coor_i", "coor_j",
        "char_i", "char_j",
        "wtok-match",
        "name", "original"};

  public:

    static std::vector<std::string> headers();
    static std::vector<std::string> headers(subject_name subj);

    static std::vector<std::string> short_text_headers();
    static std::vector<std::string> short_table_headers();

    base_instance();
    
    base_instance(hash_type subj_hash,
		  model_name type,
		  range_type char_range,
		  range_type ctok_range,
		  range_type wtok_range);
    
    // Paragraph entity
    base_instance(hash_type subj_hash,
		  model_name type, std::string subtype,
		  std::string name, std::string orig,
		  range_type char_range,
		  range_type ctok_range,
		  range_type wtok_range);
    
    // Table entity
    base_instance(hash_type subj_hash,
		  model_name type, std::string subtype,
		  std::string name, std::string orig,
		  range_type coor, range_type span,
		  range_type char_range,
		  range_type ctok_range,
		  range_type wtok_range);
    
    // Document entity
    base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
		  const base_instance& other);
    
    bool verify_wtok_range_match(std::vector<word_token>& wtokens);

    std::size_t char_len() { return (char_range[1]-char_range[0]);}
    std::size_t ctoken_len() { return (ctok_range[1]-ctok_range[0]);}
    std::size_t wtoken_len() { return (wtok_range[1]-wtok_range[0]);}

    std::string get_name() const;

    std::string get_reference() const;

    nlohmann::json to_json() const;

    nlohmann::json to_json_row() const;
    bool from_json_row(const nlohmann::json& row);

    nlohmann::json to_json_row(subject_name subj) const;

    std::vector<std::string> to_row(std::size_t col_width);

    std::vector<std::string> to_row(std::string& text,
                                    std::size_t name_width,
                                    std::size_t orig_width);

    friend bool operator<(const base_instance& lhs,
                          const base_instance& rhs);

  private:

    void initialise_hashes();

  public:

    hash_type subj_hash; // hash of the subject from which the entity comes
    
    subject_name subj_name;
    std::string subj_path;

    hash_type ehash; // entity-hash
    hash_type ihash; // instance-hash: combination of subj-hash, ent-hash and position

    val_type conf;

    range_type coor; // table-coors (ignore for text)
    range_type span; // table-spans (ignore for text)

    model_name model_type;
    std::string model_subtype;

    std::string name, orig;

    range_type char_range; // this allows the user to apply `substr` on the text
    range_type ctok_range; // this allows the user to iterate over the char_tokens
    range_type wtok_range; // this allows the user to iterate over the word_tokens

    bool wtok_range_match; // this indicates if the entity is a perfect match in the word_token vector, essentially
  };

  base_instance::base_instance()
  {}
  
  base_instance::base_instance(hash_type subj_hash,
                           model_name type,
                           range_type char_range,
                           range_type ctok_range,
                           range_type wtok_range):
    subj_hash(subj_hash),
    subj_name(PARAGRAPH),
    subj_path(""),


    ehash(DEFAULT_HASH),
    ihash(DEFAULT_HASH),

    conf(1.0),

    coor(DEFAULT_COOR),
    span(DEFAULT_SPAN),

    model_type(type),
    model_subtype(""),

    name(""),
    orig(""),

    char_range(char_range),
    ctok_range(ctok_range),
    wtok_range(wtok_range),

    wtok_range_match(true)
  {
    assert(char_range[0]<=char_range[1]);
    assert(ctok_range[0]<=ctok_range[1]);
    assert(wtok_range[0]<=wtok_range[1]);

    initialise_hashes();

    wtok_range_match = (wtok_range[0]<wtok_range[1]);
  }

  base_instance::base_instance(hash_type subj_hash,
                           model_name type, std::string subtype,
                           std::string name, std::string orig,
                           range_type char_range,
                           range_type ctok_range,
                           range_type wtok_range):

    subj_hash(subj_hash),
    subj_name(PARAGRAPH),
    subj_path("#"),

    ehash(DEFAULT_HASH),
    ihash(DEFAULT_HASH),

    conf(1.0),

    coor(DEFAULT_COOR),
    span(DEFAULT_SPAN),

    model_type(type),
    model_subtype(subtype),

    name(name),
    orig(orig),

    char_range(char_range),
    ctok_range(ctok_range),
    wtok_range(wtok_range),

    wtok_range_match(true)
  {
    assert(char_range[0]<char_range[1]);
    assert(ctok_range[0]<ctok_range[1]);
    assert(wtok_range[0]<=wtok_range[1]);

    initialise_hashes();

    wtok_range_match = (wtok_range[0]<wtok_range[1]);
  }

  base_instance::base_instance(hash_type subj_hash,
                           model_name type, std::string subtype,
                           std::string name, std::string orig,
                           range_type coor, range_type span,
                           range_type char_range,
                           range_type ctok_range,
                           range_type wtok_range):

    subj_hash(subj_hash),
    subj_name(TABLE),
    subj_path("#"),

    ehash(DEFAULT_HASH),
    ihash(DEFAULT_HASH),

    conf(1.0),

    coor(coor),
    span(span),

    model_type(type),
    model_subtype(subtype),

    name(name),
    orig(orig),

    char_range(char_range),
    ctok_range(ctok_range),
    wtok_range(wtok_range),

    wtok_range_match(true)
  {
    assert(char_range[0]<=char_range[1]);
    assert(ctok_range[0]<=ctok_range[1]);
    assert(wtok_range[0]<=wtok_range[1]);

    initialise_hashes();

    wtok_range_match = (wtok_range[0]<wtok_range[1]);
  }

  base_instance::base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
			   const base_instance& other):
    /*
                           model_name type, std::string subtype,
                           std::string name, std::string orig,
                           range_type coor, range_type span,
                           range_type char_range,
                           range_type ctok_range,
                           range_type wtok_range):
    */
    subj_hash(subj_hash),
    subj_name(subj_name),
    subj_path(subj_path),


    ehash(other.ehash),
    ihash(other.ihash),

    conf(other.conf),

    coor(other.coor),
    span(other.span),

    model_type(other.model_type),
    model_subtype(other.model_subtype),

    name(other.name),
    orig(other.orig),

    char_range(other.char_range),
    ctok_range(other.ctok_range),
    wtok_range(other.wtok_range),

    wtok_range_match(other.wtok_range_match)
  {}

  void base_instance::initialise_hashes()
  {
    ehash = utils::to_hash(name);

    std::vector<hash_type> hash_vec =
      {
       subj_hash,
       ehash,
       coor.at(0),
       coor.at(1),
       char_range.at(0),
       char_range.at(1)
      };

    ihash = utils::to_hash(hash_vec);
  }

  // do the char-ranges line up with the word-ranges ...
  bool base_instance::verify_wtok_range_match(std::vector<word_token>& word_tokens)
  {
    if(wtok_range[0]==wtok_range[1] or wtok_range[1]==0)
      {
        wtok_range_match = false;
        return wtok_range_match;
      }

    wtok_range_match =
      ((word_tokens.at(wtok_range[0]-0).get_rng(0)==char_range[0]) and
       (word_tokens.at(wtok_range[1]-1).get_rng(1)==char_range[1]) );

    return wtok_range_match;
  }

  std::vector<std::string> base_instance::headers()
  {
    return HEADERS;
  }
  
  nlohmann::json base_instance::to_json_row() const
  {
    auto row = nlohmann::json::array({to_key(model_type), model_subtype,
				      subj_hash, to_string(subj_name), subj_path,
				      conf, ehash, ihash,
				      coor[0], coor[1],
				      char_range[0], char_range[1],
				      ctok_range[0], ctok_range[1],
				      wtok_range[0], wtok_range[1],
				      wtok_range_match,
				      name, orig});

    assert(row.size()==headers().size());

    return row;    
  }

  bool base_instance::from_json_row(const nlohmann::json& row)
  {
    if((not row.is_array()) or row.size()!=19)
      {
	LOG_S(ERROR) << "inconsistent entity-row: " << row.dump();
	return false;
      }
    
    model_type = to_modelname(row.at(0).get<std::string>());
    model_subtype = row.at(1).get<std::string>();

    subj_hash = row.at(2).get<hash_type>();
    subj_name = to_subject_name(row.at(3).get<std::string>());
    subj_path = row.at(4).get<std::string>();

    conf = row.at(5).get<val_type>();

    ehash = row.at(6).get<hash_type>();
    ihash = row.at(7).get<hash_type>();

    coor.at(0) = row.at(8).get<ind_type>();
    coor.at(1) = row.at(9).get<ind_type>();

    char_range.at(0) = row.at(10).get<ind_type>();
    char_range.at(1) = row.at(11).get<ind_type>();

    ctok_range.at(0) = row.at(12).get<ind_type>();
    ctok_range.at(1) = row.at(13).get<ind_type>();

    wtok_range.at(0) = row.at(14).get<ind_type>();
    wtok_range.at(1) = row.at(15).get<ind_type>();

    wtok_range_match = row.at(16).get<bool>();
    
    name = row.at(17).get<std::string>();
    orig = row.at(18).get<std::string>();    
    
    return true;
  }
  
  std::vector<std::string> base_instance::headers(subject_name subj)
  {
    switch(subj)
      {
      case PARAGRAPH:
        {
          return TEXT_HEADERS;
        }
        break;

      case TABLE:
        {
          return TABLE_HEADERS;
        }
        break;

      default:
        {
          return HEADERS;
        }
      }
  }

  nlohmann::json base_instance::to_json_row(subject_name subj) const
  {
    nlohmann::json row;

    switch(subj)
      {
      case PARAGRAPH:
        {
          row = nlohmann::json::array({to_key(model_type), model_subtype,
                                       conf, ehash, ihash,
                                       char_range[0], char_range[1],
                                       ctok_range[0], ctok_range[1],
                                       wtok_range[0], wtok_range[1],
                                       wtok_range_match,
                                       name, orig});
        }
        break;

      case TABLE:
        {
          row = nlohmann::json::array({to_key(model_type), model_subtype,
                                       conf, ehash, ihash,
                                       coor[0], coor[1],
                                       char_range[0], char_range[1],
                                       ctok_range[0], ctok_range[1],
                                       wtok_range[0], wtok_range[1],
                                       wtok_range_match,
                                       name, orig});

        }
        break;

      default:
        {
          row = nlohmann::json::array({to_key(model_type), model_subtype,
                                       conf, ehash, ihash,
                                       coor[0], coor[1],
                                       char_range[0], char_range[1],
                                       ctok_range[0], ctok_range[1],
                                       wtok_range[0], wtok_range[1],
                                       wtok_range_match,
                                       name, orig});
        }
      }

    if(row.size()!=headers(subj).size())
      {
        LOG_S(ERROR);
      }

    return row;
  }

  std::vector<std::string> base_instance::short_text_headers()
  {
    return SHORT_TEXT_HEADERS;
  }

  std::vector<std::string> base_instance::short_table_headers()
  {
    return SHORT_TABLE_HEADERS;
  }

  std::string base_instance::get_name() const
  {
    return name;
  }

  std::string base_instance::get_reference() const
  {
    std::string ref = subj_path;

    ref += std::to_string(ehash)+"_"+
      std::to_string(model_type)+"_coor_"+
      std::to_string(coor.at(0))+"-"+
      std::to_string(coor.at(1))+"_char_"+
      std::to_string(char_range.at(0))+"-"+
      std::to_string(char_range.at(1));

    return ref;
  }

  nlohmann::json base_instance::to_json() const
  {
    nlohmann::json result = nlohmann::json::object();
    {
      result["ehash"] = ehash;
      result["ihash"] = ihash;

      result["confidence"] = conf;

      result["model-type"] = to_key(model_type);
      result["model-subtype"] = model_subtype;

      result["name"] = name;
      result["orig"] = orig;

      result["coor"] = coor;
      result["span"] = span;

      result["char-range"] = char_range;
      result["ctok-range"] = ctok_range;
      result["wtok-range"] = wtok_range;

      result["wtok-range-match"] = wtok_range_match;
    }

    return result;
  }


  std::vector<std::string> base_instance::to_row(std::size_t col_width)
  {
    switch(subj_name)
      {
      case PARAGRAPH:
        {
          std::vector<std::string> row =
            {
             to_key(model_type),
             model_subtype,

             std::to_string(conf),

             std::to_string(ehash),
             std::to_string(ihash),

             std::to_string(char_range[0]),
             std::to_string(char_range[1]),

             wtok_range_match? "true":"false",

             utils::to_fixed_size(name, col_width),
             utils::to_fixed_size(orig, col_width)
            };
          assert(row.size()==SHORT_TEXT_HEADERS.size());
          return row;
        }
        break;

      case TABLE:
        {
          std::vector<std::string> row =
            {
             to_key(model_type),
             model_subtype,

             std::to_string(conf),

             std::to_string(ehash),
             std::to_string(ihash),

             std::to_string(coor[0]),
             std::to_string(coor[1]),

             std::to_string(char_range[0]),
             std::to_string(char_range[1]),

             wtok_range_match? "true":"false",

             utils::to_fixed_size(name, col_width),
             utils::to_fixed_size(orig, col_width)
            };
          assert(row.size()==SHORT_TABLE_HEADERS.size());

          return row;
        }
        break;

      default:
        {
          std::vector<std::string> row =
            {
             to_key(model_type),
             model_subtype,

             std::to_string(conf),

             std::to_string(ehash),
             std::to_string(ihash),

             std::to_string(char_range[0]),
             std::to_string(char_range[1]),

             wtok_range_match? "true":"false",

             utils::to_fixed_size(name, col_width),
             utils::to_fixed_size(orig, col_width)
            };
          assert(row.size()==SHORT_TEXT_HEADERS.size());

          return row;
        }
      }
  }

  std::vector<std::string> base_instance::to_row(std::string& text,
                                               std::size_t name_width,
                                               std::size_t orig_width)
  {
    std::string tmp_0=name;

    std::string tmp_1=orig;
    if(tmp_1.size()==0)
      {
        tmp_1 = text.substr(char_range[0], char_range[1]-char_range[0]);
      }

    std::vector<std::string> row =
      { to_key(model_type), model_subtype,
        std::to_string(conf), std::to_string(ehash), std::to_string(ihash),
        std::to_string(char_range[0]), std::to_string(char_range[1]),
        wtok_range_match? "true":"false",
        utils::to_fixed_size(tmp_0, name_width),
        utils::to_fixed_size(tmp_1, orig_width)
      };
    assert(row.size()==SHORT_TEXT_HEADERS.size());

    return row;
  }
  
  bool operator<(const base_instance& lhs,
                 const base_instance& rhs)
  {
    if(lhs.subj_path==rhs.subj_path)
      {
        if(lhs.coor[0]==rhs.coor[0])
          {
            if(lhs.coor[1]==rhs.coor[1])
              {
                if(lhs.char_range[0]==rhs.char_range[0])
                  {
                    return lhs.char_range[1]>rhs.char_range[1];
                  }
                else
                  {
                    return lhs.char_range[0]<rhs.char_range[0];
                  }
              }
            else
              {
                return lhs.coor[1]<rhs.coor[1];
              }
          }
        else
          {
            return lhs.coor[0]<rhs.coor[0];
          }
      }
    else
      {
        return (lhs.subj_path==rhs.subj_path);
      }
  }

}

#endif
