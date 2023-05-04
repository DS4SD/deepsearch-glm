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

      const static inline flvr_type TOKEN = 0;
      const static inline flvr_type SYNTX = 1;

      const static inline flvr_type    LABEL = 2;
      const static inline flvr_type SUBLABEL = 3;

      const static inline flvr_type CONT = 8;  // contraction
      const static inline flvr_type CONN = 9;  // connector (eg `of a`, `with these`, etc)
      const static inline flvr_type TERM = 10;  // term
      const static inline flvr_type VERB = 11;  // verb
      
      const static inline flvr_type SENT = 16; // sentence
      const static inline flvr_type TEXT = 32; // text (multiple sentences)
      const static inline flvr_type TABL = 48; // table

      const static inline flvr_type SECT = 64; // section (multiple texts)

      const static inline flvr_type FDOC = 96; // book (multiple sections)
      const static inline flvr_type BOOK = 128; // book (multiple sections)
      
      const static inline std::string UNKNOWN = "__unknown__";
      const static inline std::string MASK = "__mask__";
      
      const static inline std::string NUMVAL = "__numval__";

      const static inline std::string BEG_TERM = "__beg_term__";
      const static inline std::string END_TERM = "__end_term__";

      const static inline std::string BEG_SENT = "__beg_sent__";
      const static inline std::string END_SENT = "__end_sent__";

      const static inline std::string BEG_TEXT = "__beg_text__";
      const static inline std::string END_TEXT = "__end_text__";

      const static inline std::vector<std::string> NAMES =
        {
         UNKNOWN, UNDEFINED_POS,

	 MASK,
	 NUMVAL,

	 BEG_TERM, END_TERM,
         BEG_SENT, END_SENT,
         BEG_TEXT, END_TEXT
        };

      static inline std::map<std::string, hash_type> to_hash = {};

      static inline std::map<flvr_type, std::string> to_name =
        {
	 { TOKEN, "token"},
	 { SYNTX, "syntax"},

	 { LABEL, "label"},
	 { SUBLABEL, "sublabel"},
	 
         { CONT, "cont"},
	 { CONN, "conn"},
	 { TERM, "term"},
	 { VERB, "verb"},

	 { SENT, "sentence"},
	 
	 { TEXT, "text"},
	 { TABL, "table"},

	 { SECT, "section"},

	 { FDOC, "documemt"}, 
	 { BOOK, "book"	} 
        };

      static flvr_type to_flavor(std::string name)
      {
	for(auto itr=to_name.begin(); itr!=to_name.end(); itr++)
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
	    for(auto itr=to_name.begin(); itr!=to_name.end(); itr++)
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
	    for(auto itr=to_name.begin(); itr!=to_name.end(); itr++)
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
         { UNKNOWN, word_token(DEFAULT_WORD, DEFAULT_POS)},
	 { UNDEFINED_POS, word_token(DEFAULT_WORD, UNDEFINED_POS)},

	 { MASK, word_token(DEFAULT_WORD, DEFAULT_WORD)},
	 { NUMVAL, word_token(NUMVAL, DEFAULT_WORD)},
	 
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
