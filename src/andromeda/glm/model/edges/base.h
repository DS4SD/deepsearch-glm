//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_EDGE_NAMES_H_
#define ANDROMEDA_MODELS_GLM_EDGE_NAMES_H_

namespace andromeda
{
  namespace glm
  {
    class edge_names: base_types
    {
    public:

      const static inline flvr_type UNKNOWN_FLVR = std::numeric_limits<flvr_type>::min();
      const static inline hash_type UNKNOWN_HASH = std::numeric_limits<hash_type>::min();

      const static inline flvr_type M6 = -6;
      const static inline flvr_type M5 = -5;
      const static inline flvr_type M4 = -4;
      const static inline flvr_type M3 = -3;
      const static inline flvr_type M2 = -2;
      const static inline flvr_type M1 = -1;

      const static inline flvr_type P1 = 1; // next or `plus 1`
      const static inline flvr_type P2 = 2; // next after next or `plus 2`
      const static inline flvr_type P3 = 3;
      const static inline flvr_type P4 = 4;
      const static inline flvr_type P5 = 5;
      const static inline flvr_type P6 = 6;

      const static inline flvr_type prev = M1;
      const static inline flvr_type next = P1;

      // e.g. `old wise man`
      const static inline flvr_type tax_dn = 32; // `BEG_TERM` -> `old` -> `wise` -> `man`
      const static inline flvr_type tax_up = 33; // `END_TERM` -> `man` -> `wise` -> old

      const static inline flvr_type to_lower = 60; // `Superconductors` -> `superconductor`
      const static inline flvr_type to_upper = 61; // `superconductor` -> `Superconductors`

      const static inline flvr_type to_singular = 62; // `Superconductors` -> `superconductor`
      const static inline flvr_type to_plural = 63; // `superconductor` -> `Superconductors`
      
      //const static inline flvr_type to_concept = 64; // `Superconductors` -> `superconductor`
      //const static inline flvr_type to_instance = 65; // `superconductor` -> `Superconductors`

      //const static inline flvr_type to_node = 96; //
      const static inline flvr_type to_word = 97; //
      const static inline flvr_type to_pos = 98; //

      const static inline flvr_type to_path = 99; //
      const static inline flvr_type from_path = 100; //

      const static inline flvr_type from_root_to_path = 101; //
      const static inline flvr_type from_path_to_root = 102; //
      const static inline flvr_type from_desc_to_path = 103; //
      const static inline flvr_type from_path_to_desc = 104; //

      const static inline flvr_type to_beg = 128; //
      const static inline flvr_type to_end = 129; //

      const static inline flvr_type from_beg = 130; //
      const static inline flvr_type from_end = 131; //

      const static inline flvr_type to_verb = 164; //
      const static inline flvr_type to_term = 166; //
      const static inline flvr_type to_conn = 168; //      
      const static inline flvr_type to_sent = 170; //
      const static inline flvr_type to_text = 172; //

      const static inline flvr_type from_verb = 165; //
      const static inline flvr_type from_term = 167; //
      const static inline flvr_type from_conn = 169; //
      const static inline flvr_type from_sent = 171; //
      const static inline flvr_type from_text = 173; //
      
      static flvr_type jump(flvr_type d=0) { return d; }

      const static inline std::map<flvr_type, std::string> flvr_to_name_map = 
        {
	 //{UNKNOWN_FLVR, "UNKNOWN_FLVR"},
	 
	 { M6, "prev-6"},
         { M5, "prev-5"},
         { M4, "prev-4"},
         { M3, "prev-3"},
         { M2, "prev-2"},
         { M1, "prev"},

         { P1, "next"},
         { P2, "next-2"},
         { P3, "next-3"},
	 { P4, "next-4"},
	 { P5, "next-5"},
	 { P6, "next-6"},

         { tax_dn, "tax-dn"},
         { tax_up, "tax-up"},

	 { to_lower, "to-lower"},
	 { to_upper, "to-upper"},
	 { to_singular, "to-singular"},
	 { to_plural, "to-plural"},
	 
         //{ to_concept, "to-concept"},
         //{ to_instance, "to-instance"},

         //{ to_node, "to-node"},
         { to_word, "to-word"},
         { to_pos , "to-pos"},

	 { to_beg, "to-begin"},
	 { to_end, "to-end"},

	 { from_beg, "from-begin"},
	 { from_end, "from-end"},

	 { to_path, "to-path"},
	 { from_path, "from-path"},

	 { from_root_to_path, "from-root-to-path" },
	 { from_path_to_root, "from-path-to-root" },
	 { from_desc_to_path, "from-desc-to-root" },
	 { from_path_to_desc, "from-path-to-desc" },

	 //{ related, "related" },
	 { to_conn, "to-conn"},
	 { to_verb, "to-verb"},
	 { to_term, "to-term"},
	 { to_sent, "to-sent"},
	 { to_text, "to-text"},

	 { from_conn, "from-conn"},
	 { from_verb, "from-verb"},
	 { from_term, "from-term"},
	 { from_sent, "from-sent"},
	 { from_text, "from-text"}
        };

      static std::string to_string(flvr_type flvr)
      {
	auto itr = flvr_to_name_map.find(flvr);

	if(itr!=flvr_to_name_map.end())
	  {
	    return itr->second;
	  }

	LOG_S(ERROR) << "requesting undefined edge-flavor `" << flvr << "`";
	return "UNKNOWN_FLVR";
      }
      
      static flvr_type to_flavor(std::string name)
      {
	for(auto itr=flvr_to_name_map.begin(); itr!=flvr_to_name_map.end(); itr++)
	  {
	    if(itr->second==name)
	      {
		return itr->first;
	      }
	  }
	
	LOG_S(ERROR) << "requesting undefined edge-flavor `" << name << "`";
	return UNKNOWN_FLVR;
      }

      static std::set<flvr_type> to_flavor(std::vector<std::string> names)
      {
	std::set<flvr_type> flvrs={};
	for(std::string name:names)
	  {
	    auto flvr = to_flavor(name);
	    if(flvr!=UNKNOWN_FLVR)
	      {
		flvrs.insert(flvr);
	      }
	  }	    
	
	return flvrs;
      }
      
    };
    
  }

}

#endif
