//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_NODE_NAMES_H_
#define ANDROMEDA_MODELS_GLM_NODE_NAMES_H_

namespace andromeda
{
  namespace glm
  {
    class node_names: public base_types
    {
    public:

      const static inline flvr_type UNKNOWN_FLVR = std::numeric_limits<flvr_type>::min();
      const static inline hash_type UNKNOWN_HASH = std::numeric_limits<hash_type>::min();
      
      const static inline flvr_type DEFAULT_TYPE=-1;
      const static inline hash_type DEFAULT_HASH=-1;

      const static inline std::string DEFAULT_WORD=word_token::DEFAULT_WORD;
      const static inline std::string DEFAULT_POS=word_token::DEFAULT_POS;
      const static inline std::string UNDEFINED_POS=word_token::UNDEF_POS;

      const static inline flvr_type SUBW_TOKEN = 0;
      const static inline flvr_type WORD_TOKEN = 1;
      const static inline flvr_type SYNTX = 2;

      const static inline flvr_type LABEL = 4;
      const static inline flvr_type SUBLABEL = 5;

      const static inline flvr_type CONT = 8;  // contraction
      const static inline flvr_type CONN = 9;  // connector (eg `of a`, `with these`, etc)
      const static inline flvr_type TERM = 10;  // term
      const static inline flvr_type VERB = 11;  // verb
      const static inline flvr_type INST = 12;  // inst
      
      const static inline flvr_type SENT = 16; // sentence

      const static inline flvr_type TEXT = 32; // text (multiple sentences)
      const static inline flvr_type TABL = 48; // table

      const static inline flvr_type SECT = 64; // section (multiple texts)
      const static inline flvr_type CHAP = 65; // chapter (multiple sections)

      const static inline flvr_type FDOC = 96; // full documents (multiple sections)
      const static inline flvr_type BOOK = 97; // book (multiple sections)
      
      const static inline std::string UNKNOWN = "__unknown__";
      const static inline std::string MASK = "__mask__";
      
      const static inline std::string NUMVAL = "__numval__";

      const static inline std::string BEG_TERM = "__beg_term__";
      const static inline std::string END_TERM = "__end_term__";

      const static inline std::string BEG_SENT = "__beg_sent__";
      const static inline std::string END_SENT = "__end_sent__";

      const static inline std::string BEG_TEXT = "__beg_text__";
      const static inline std::string END_TEXT = "__end_text__";

      const static inline std::vector<std::string> TOKEN_NAMES =
        {
         UNKNOWN, UNDEFINED_POS,

	 MASK,
	 NUMVAL
	};
      
      const static inline std::vector<std::string> LABEL_NAMES =
	{
	 BEG_TERM, END_TERM,
         BEG_SENT, END_SENT,
         BEG_TEXT, END_TEXT
        };

      static inline std::map<std::string, hash_type> to_hash = {};

    private:
      
      static inline std::map<flvr_type, std::string> to_name_map =
        {
	 { SUBW_TOKEN, "subw_token"},
	 { WORD_TOKEN, "word_token"},

	 { SYNTX, "syntax"},

	 { LABEL, "label"},
	 { SUBLABEL, "sublabel"},
	 
         { CONT, "cont"},
	 { CONN, "conn"},
	 { TERM, "term"},
	 { VERB, "verb"},
	 { INST, "inst"},

	 { SENT, "sentence"},
	 
	 { TEXT, "text"},
	 { TABL, "table"},

	 { SECT, "section"},
	 { CHAP, "chapter"},

	 { FDOC, "document"}, 
	 { BOOK, "book"	} 
        };

    public:

      static typename std::map<flvr_type, std::string>::iterator begin()
      {
	return to_name_map.begin();
      }

      static typename std::map<flvr_type, std::string>::iterator end()
      {
	return to_name_map.end();
      }      

      static std::string to_name(flvr_type flvr)
      {
	return to_name_map.at(flvr);
      }
      
      static flvr_type to_flavor(std::string name)
      {
	for(auto itr=begin(); itr!=end(); itr++)
	  {
	    if(itr->second==name)
	      {
		return itr->first;
	      }
	  }
	
	LOG_S(ERROR) << "requesting undefined node-flavor `" << name << "`";
	return UNKNOWN_FLVR;
      }

      static std::vector<flvr_type> to_flavor(std::vector<std::string> names)
      {
	std::vector<flvr_type> result={};
	for(auto name:names)
	  {
	    for(auto itr=begin(); itr!=end(); itr++)
	      {
		if(itr->second==name)
		  {
		    result.push_back(itr->first);
		  }
	      }
	  }
		
	return result;
      }

      static std::set<flvr_type> to_flavor(std::set<std::string> names)
      {
	std::set<flvr_type> result={};
	for(auto name:names)
	  {
	    for(auto itr=begin(); itr!=end(); itr++)
	      {
		if(itr->second==name)
		  {
		    result.insert(itr->first);
		  }
	      }
	  }
		
	return result;
      }      
      
      static inline std::map<std::string, word_token> to_token =
        {
         { UNKNOWN      , word_token(DEFAULT_WORD, DEFAULT_POS)},
	 { UNDEFINED_POS, word_token(DEFAULT_WORD, UNDEFINED_POS)},

	 { MASK  , word_token(DEFAULT_WORD, DEFAULT_WORD)},
	 { NUMVAL, word_token(NUMVAL, DEFAULT_WORD)}
	};

      static inline std::map<std::string, word_token> to_label =
	{
	 { BEG_TERM, word_token(BEG_TERM, DEFAULT_WORD)},
	 { END_TERM, word_token(END_TERM, DEFAULT_WORD)},

	 { BEG_SENT, word_token(BEG_SENT, DEFAULT_WORD)},
	 { END_SENT, word_token(END_SENT, DEFAULT_WORD)},

	 { BEG_TEXT, word_token(BEG_TEXT, DEFAULT_WORD)},
	 { END_TEXT, word_token(END_TEXT, DEFAULT_WORD)}	 
        };
    };
    
  }

}

#endif
