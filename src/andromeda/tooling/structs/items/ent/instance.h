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

    const static inline table_index_type DEFAULT_TABLE_INDEX = -1;

    const static inline table_coor_type DEFAULT_COOR = {DEFAULT_TABLE_INDEX, DEFAULT_TABLE_INDEX};
    const static inline table_range_type DEFAULT_SPAN = {DEFAULT_TABLE_INDEX, DEFAULT_TABLE_INDEX};

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

    base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
                  model_name type,
                  range_type char_range,
                  range_type ctok_range,
                  range_type wtok_range);

    // Text entity
    base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
                  model_name type, std::string subtype,
                  std::string name, std::string orig,
                  range_type char_range,
                  range_type ctok_range,
                  range_type wtok_range);

    // Text entity
    base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
		  val_type conf,
		  model_name type, std::string subtype,
                  std::string name, std::string orig,
                  range_type char_range,
                  range_type ctok_range,
                  range_type wtok_range);

    // Table entity
    base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
                  model_name type, std::string subtype,
                  std::string name, std::string orig,
                  table_range_type coor,
                  table_range_type row_span,
                  table_range_type col_span,
                  range_type char_range,
                  range_type ctok_range,
                  range_type wtok_range);

    bool is_wtok_range_match() { return wtok_range_match; }

    bool verify_wtok_range_match(std::vector<word_token>& wtokens);

    std::size_t char_len() { return (char_range[1]-char_range[0]);}
    std::size_t ctoken_len() { return (ctok_range[1]-ctok_range[0]);}
    std::size_t wtoken_len() { return (wtok_range[1]-wtok_range[0]);}

    bool is_subject(subject_name name) { return name==this->subj_name; }
    
    hash_type get_subj_hash() const { return subj_hash; }
    subject_name get_subj_name() const { return subj_name; }
    std::string get_subj_path() const { return subj_path; }

    hash_type get_ehash() const { return ehash; } // entity-hash
    hash_type get_ihash() const { return ihash; }  // instance-hash: combination of subj-hash, ent-hash and position

    std::string get_name() const { return name; }
    std::string get_orig() const { return orig; }

    model_name get_model() const { return model_type; }

    std::string get_type() const { return to_key(model_type); }
    std::string get_subtype() const { return model_subtype; }

    bool is_in(subject_name sn) const { return (sn==subj_name);}
    bool is_model(model_name mt) const { return (mt==model_type);}
    bool is_type(std::string t) const { return (t==to_key(model_type));}
    bool is_subtype(std::string st) const { return (st==model_subtype);}

    range_type get_char_range() const { return char_range; }
    index_type get_char_range(index_type i) const { return char_range.at(i); }

    range_type get_ctok_range() const { return ctok_range; }
    index_type get_ctok_range(index_type i) const { return ctok_range.at(i); }

    range_type get_wtok_range() const { return wtok_range; }
    index_type get_wtok_range(index_type i) const { return wtok_range.at(i); }

    val_type get_conf() const { return conf; }
    
    range_type get_coor() const { return coor; }
    index_type get_coor(index_type i) const { return coor.at(i); }

    void set_ctok_range(range_type cr) { ctok_range = cr; }
    void set_wtok_range(range_type wr) { wtok_range = wr; }

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

    friend bool operator==(const base_instance& lhs,
                           const base_instance& rhs);

  private:

    void initialise_hashes();

  protected:

    hash_type subj_hash; // hash of the subject from which the entity comes
    subject_name subj_name;
    std::string subj_path;

    hash_type ehash; // entity-hash
    hash_type ihash; // instance-hash: combination of subj-hash, ent-hash and position

    val_type conf;

    table_range_type coor; // table-coors (ignore for text)
    table_range_type row_span; // table-spans (ignore for text)
    table_range_type col_span; // table-spans (ignore for text)

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

  base_instance::base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
                               model_name type,
                               range_type char_range,
                               range_type ctok_range,
                               range_type wtok_range):
    subj_hash(subj_hash),
    subj_name(subj_name),
    subj_path(subj_path),

    ehash(DEFAULT_HASH),
    ihash(DEFAULT_HASH),

    conf(1.0),

    coor(DEFAULT_COOR),
    row_span(DEFAULT_SPAN),
    col_span(DEFAULT_SPAN),

    model_type(type),
    model_subtype(""),

    name(""),
    orig(""),

    char_range(char_range),
    ctok_range(ctok_range),
    wtok_range(wtok_range),

    wtok_range_match(true)
  {
    //LOG_S(WARNING) << "copy constructor " << __FILE__ << ":" << __LINE__;
    
    assert(char_range[0]<=char_range[1]);
    assert(ctok_range[0]<=ctok_range[1]);
    assert(wtok_range[0]<=wtok_range[1]);

    assert(subj_name==TEXT);
    assert(subj_path!="");

    initialise_hashes();

    wtok_range_match = (wtok_range[0]<wtok_range[1]);
  }

  base_instance::base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
                               model_name type, std::string subtype,
                               std::string name, std::string orig,
                               range_type char_range,
                               range_type ctok_range,
                               range_type wtok_range):
    subj_hash(subj_hash),
    subj_name(subj_name),
    subj_path(subj_path),

    ehash(DEFAULT_HASH),
    ihash(DEFAULT_HASH),

    conf(1.0),

    coor(DEFAULT_COOR),
    row_span(DEFAULT_SPAN),
    col_span(DEFAULT_SPAN),

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

    assert(subj_name==TEXT or subj_name==DOCUMENT);
    assert(subj_path!="");

    initialise_hashes();

    wtok_range_match = (wtok_range[0]<wtok_range[1]);
  }

  base_instance::base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
			       val_type conf,
                               model_name type, std::string subtype,
                               std::string name, std::string orig,
                               range_type char_range,
                               range_type ctok_range,
                               range_type wtok_range):
    subj_hash(subj_hash),
    subj_name(subj_name),
    subj_path(subj_path),

    ehash(DEFAULT_HASH),
    ihash(DEFAULT_HASH),

    conf(conf),

    coor(DEFAULT_COOR),
    row_span(DEFAULT_SPAN),
    col_span(DEFAULT_SPAN),

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

    assert(subj_name==TEXT or subj_name==DOCUMENT);
    assert(subj_path!="");

    initialise_hashes();

    wtok_range_match = (wtok_range[0]<wtok_range[1]);
  }  

  base_instance::base_instance(hash_type subj_hash, subject_name subj_name, std::string subj_path,
                               model_name type, std::string subtype,
                               std::string name, std::string orig,
                               table_range_type coor,
                               table_range_type row_span,
                               table_range_type col_span,
                               range_type char_range,
                               range_type ctok_range,
                               range_type wtok_range):

    subj_hash(subj_hash),
    subj_name(subj_name),
    subj_path(subj_path),

    ehash(DEFAULT_HASH),
    ihash(DEFAULT_HASH),

    conf(1.0),

    coor(coor),
    row_span(row_span),
    col_span(col_span),

    model_type(type),
    model_subtype(subtype),

    name(name),
    orig(orig),

    char_range(char_range),
    ctok_range(ctok_range),
    wtok_range(wtok_range),

    wtok_range_match(true)
  {
    assert(subj_name==TABLE);
    assert(subj_path!="");

    assert(name.size()>0);
    assert(orig.size()>0);

    assert(char_range[0]<=char_range[1]);
    assert(ctok_range[0]<=ctok_range[1]);
    assert(wtok_range[0]<=wtok_range[1]);

    initialise_hashes();

    wtok_range_match = (wtok_range[0]<wtok_range[1]);
  }

  void base_instance::initialise_hashes()
  {
    //ehash = utils::to_hash(name);
    ehash = utils::to_reproducible_hash(name);

    std::vector<hash_type> hash_vec =
      {
        subj_hash,
        ehash,
        coor.at(0),
        coor.at(1),
        char_range.at(0),
        char_range.at(1)
      };

    //LOG_S(INFO) << "'" << name << "' => ehash: " << ehash << " => ihash: " << ihash;

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
    nlohmann::json row;

    switch(subj_name)
      {
      case TEXT:
        {
          row = nlohmann::json::array({to_key(model_type), model_subtype,
              subj_hash, to_string(subj_name), subj_path,
              std::round(100.0*conf)/100.0,
              ehash, ihash,              
	      nlohmann::json::value_t::null, nlohmann::json::value_t::null, //coor[0], coor[1],
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
              subj_hash, to_string(subj_name), subj_path,
              std::round(100.0*conf)/100.0,
              ehash, ihash,
              coor[0], coor[1],
              char_range[0], char_range[1],
              ctok_range[0], ctok_range[1],
              wtok_range[0], wtok_range[1],
              wtok_range_match,
              name, orig});
        }
	break;
	  
      case DOCUMENT:
        {
          row = nlohmann::json::array({to_key(model_type), model_subtype,
              subj_hash, to_string(subj_name), subj_path,
              std::round(100.0*conf)/100.0,
              ehash, ihash,              
	      nlohmann::json::value_t::null, nlohmann::json::value_t::null,
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
              subj_hash, to_string(subj_name), subj_path,
              std::round(100.0*conf)/100.0,
              ehash, ihash,
              coor[0], coor[1],
              char_range[0], char_range[1],
              ctok_range[0], ctok_range[1],
              wtok_range[0], wtok_range[1],
              wtok_range_match,
              name, orig});	  
	}
      }
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

      conf = (row.at(5).get<val_type>());

      ehash = row.at(6).get<hash_type>();
      ihash = row.at(7).get<hash_type>();

      coor = DEFAULT_COOR;
      if(not row.at(8).is_null())
	{
	  coor.at(0) = row.at(8).get<ind_type>();
	}
      
      if(not row.at(9).is_null())
	{
	  coor.at(1) = row.at(9).get<ind_type>();
	}
      
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
        case TEXT:
          {
            return TEXT_HEADERS;
          }
          break;

        case TABLE:
          {
            return TABLE_HEADERS;
          }
          break;

	case DOCUMENT:
          {
            return TEXT_HEADERS;
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
        case TEXT:
          {
            row = nlohmann::json::array({to_key(model_type), model_subtype,
                utils::round_conf(conf),
                ehash, ihash,
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
                utils::round_conf(conf),
                ehash, ihash,
                coor[0], coor[1],
                char_range[0], char_range[1],
                ctok_range[0], ctok_range[1],
                wtok_range[0], wtok_range[1],
                wtok_range_match,
                name, orig});

          }
          break;

        case DOCUMENT:
          {
            row = nlohmann::json::array({to_key(model_type), model_subtype,
                utils::round_conf(conf),
                ehash, ihash,
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
                utils::round_conf(conf),
                ehash, ihash,
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
        result["subj_hash"] = subj_hash;
        result["subj_name"] = to_string(subj_name);
        result["subj_path"] = subj_path;

        result["ehash"] = ehash;
        result["ihash"] = ihash;

        result["confidence"] = utils::round_conf(conf);

        result["model-type"] = to_key(model_type);
        result["model-subtype"] = model_subtype;

        result["name"] = name;
        result["orig"] = orig;

        result["coor"] = coor;
        result["row-span"] = row_span;
        result["col-span"] = col_span;

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
        case TEXT:
          {
            std::vector<std::string> row =
              {
                to_key(model_type),
                model_subtype,

                std::to_string(utils::round_conf(conf)),

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

                std::to_string(utils::round_conf(conf)),

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

        case DOCUMENT:
          {
            std::vector<std::string> row =
              {
                to_key(model_type),
                model_subtype,

                std::to_string(utils::round_conf(conf)),

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
	  
        default:
          {
            std::vector<std::string> row =
              {
                to_key(model_type),
                model_subtype,

                std::to_string(utils::round_conf(conf)),

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
          std::to_string(utils::round_conf(conf)),
          std::to_string(ehash), std::to_string(ihash),
          std::to_string(char_range[0]), std::to_string(char_range[1]),
          wtok_range_match? "true":"false",
          utils::to_fixed_size(tmp_0, name_width),
          utils::to_fixed_size(tmp_1, orig_width)
        };
      assert(row.size()==SHORT_TEXT_HEADERS.size());

      return row;
    }

    bool operator==(const base_instance& lhs,
                    const base_instance& rhs)
    {
      return ((lhs.model_type==rhs.model_type) and
              (lhs.model_subtype==rhs.model_subtype) and
              (lhs.subj_name==rhs.subj_name) and
              (lhs.subj_path==rhs.subj_path) and
              (lhs.coor[0]==rhs.coor[0]) and
              (lhs.coor[1]==rhs.coor[1]) and
              (lhs.char_range[0]==rhs.char_range[0]) and
              (lhs.char_range[1]==rhs.char_range[1]));
    }

    bool operator<(const base_instance& lhs,
                   const base_instance& rhs)
    {
      if(lhs.subj_name==rhs.subj_name)
        {
          if(lhs.subj_path==rhs.subj_path)
            {
              if(lhs.coor[0]==rhs.coor[0])
                {
                  if(lhs.coor[1]==rhs.coor[1])
                    {
                      if(lhs.char_range[0]==rhs.char_range[0])
                        {
                          if(lhs.char_range[1]==rhs.char_range[1])
                            {
			      if(lhs.model_type==rhs.model_type)
				{
				  return (lhs.model_subtype<rhs.model_subtype);
				}
			      else
				{
				  return (lhs.model_type<rhs.model_type);
				}
			      
			      /*
                              //LOG_S(INFO) << lhs.model_type << "\t" << rhs.model_type;

                              const auto& ltype = lhs.model_type;
                              const auto& rtype = rhs.model_type;

                              if(ltype==rtype)
                                {
                                  const auto& lstype = lhs.model_subtype;
                                  const auto& rstype = rhs.model_subtype;

                                  //LOG_S(INFO) << lhs.model_subtype << "\t" << rhs.model_subtype;
                                  return ((lstype.compare(rstype))<0);
                                }
                              else
                                {
                                  return (ltype<rtype);
                                }
			      */
                            }
                          else
                            {
                              return lhs.char_range[1]>rhs.char_range[1];
                            }
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
              /*
                auto lhs_parts = utils::split(lhs.subj_path, "/");
                auto rhs_parts = utils::split(rhs.subj_path, "/");

                if(lhs_parts.size()==rhs_parts.size())
                {
                if(lhs_parts.size()==3)
                {
                return std::stoi(lhs_parts.at(2))<std::stoi(rhs_parts.at(2));
                }
                else
                {
                return (lhs.subj_path<rhs.subj_path);
                }
                }
                else
                {
                return lhs_parts.size()<rhs_parts.size();
                }
              */

              return utils::compare_paths(lhs.subj_path, rhs.subj_path);
            }
        }
      else
        {
          return (lhs.subj_name<rhs.subj_name);
        }
    }

  }

#endif
