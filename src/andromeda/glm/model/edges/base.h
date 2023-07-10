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

      const static inline flvr_type to_lower = 64; // `Superconductors` -> `superconductor`
      const static inline flvr_type to_upper = 65; // `superconductor` -> `Superconductors`

      const static inline flvr_type to_singular = 66; // `Superconductors` -> `superconductor`
      const static inline flvr_type to_plural = 67; // `superconductor` -> `Superconductors`

      const static inline flvr_type to_token = 96; //
      const static inline flvr_type from_token = 97; //
            
      const static inline flvr_type to_pos = 98; //
      const static inline flvr_type from_pos = 99; //

      const static inline flvr_type to_label = 100; // `Superconductors` -> `superconductor`
      const static inline flvr_type from_label = 101; // `superconductor` -> `Superconductors`

      const static inline flvr_type to_root = 102; // `Superconductors` -> `superconductor`
      const static inline flvr_type from_root = 103; // `superconductor` -> `Superconductors`

      // hierarchy
      const static inline flvr_type to_sent = 128; 
      const static inline flvr_type from_sent = 129;
      const static inline flvr_type to_text = 130; 
      const static inline flvr_type from_text = 131;
      const static inline flvr_type to_table = 132; 
      const static inline flvr_type from_table = 133;
      const static inline flvr_type to_doc = 134; 
      const static inline flvr_type from_doc = 135; 
      
      const static inline flvr_type custom = 256;
      
    private:

      static inline std::mutex mtx;
      
      static inline std::map<flvr_type, std::string> flvr_to_name_map = 
        {
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

	 
         { to_token, "to-token"},
         { from_token, "from-token"},

         { to_pos, "to-pos"},
         { from_pos, "from-pos"},

	 { to_label, "to-label"},
         { from_label, "from-label"},

	 { to_root, "to-root"},
         { from_root, "from-root"},

	 { to_sent, "to-sent"},
         { from_sent, "from-sent"},
	 { to_text, "to-text"},
         { from_text, "from-text"},
	 { to_table, "to-table"},
         { from_table, "from-table"},
	 { to_doc, "to-doc"},
         { from_doc, "from-doc"}
        };

    public:

      static flvr_type update_flvr(const std::string& name)
      {
	auto itr = flvr_to_name_map.begin();

	while(itr!=flvr_to_name_map.end() and itr->second!=name)
	  {
	    //LOG_S(INFO) << " -> name: " << name << " (" << itr->first << ", " << itr->second << ")";
	    itr++;
	  }

	flvr_type flvr=custom;
	if(itr==flvr_to_name_map.end())
	  {
	    std::scoped_lock<std::mutex> lock(mtx);

	    flvr = flvr_to_name_map.size()>0? flvr_to_name_map.rbegin()->first:0;

	    if(flvr<custom)
	      {
		flvr = custom;		
	      }
	    else
	      {
		flvr += 1;
	      }

	    //LOG_S(WARNING) << "updating edge-flavor `" << name << "`: " << flvr;

	    flvr_to_name_map[flvr] = name;
	  }
	else
	  {
	    flvr = itr->first;
	  }
	
	return flvr;
      }

      static typename std::map<flvr_type, std::string>::iterator begin()
      {
	return flvr_to_name_map.begin();
      }

      static typename std::map<flvr_type, std::string>::iterator end()
      {
	return flvr_to_name_map.end();
      }
      
      static flvr_type jump(flvr_type d=0)
      {
	return d;
      }
      
      static std::string to_name(flvr_type flvr)
      {
	return flvr_to_name_map.at(flvr);
      }

      static flvr_type to_flvr(std::string name)
      {
	for(auto itr=begin(); itr!=end(); itr++)
	  {
	    if(itr->second==name)
	      {
		return itr->first;
	      }
	  }
	
	flvr_type flvr = update_flvr(name);
	return flvr;
      }

      static std::set<flvr_type> to_flvr(std::vector<std::string> names)
      {
	std::set<flvr_type> flvrs={};
	for(std::string name:names)
	  {
	    auto flvr = to_flvr(name);
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
