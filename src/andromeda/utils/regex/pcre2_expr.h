//-*-C++-*-

#ifndef ANDROMEDA_UTILS_REGEX_PCRE2_EXPR_H_
#define ANDROMEDA_UTILS_REGEX_PCRE2_EXPR_H_

namespace andromeda
{

  class pcre2_expr
  {

  public:

    pcre2_expr();
    pcre2_expr(std::string type,
               std::string subtype,
               std::string expr);

    ~pcre2_expr();

    std::string get_type() { return type; }
    std::string get_subtype() { return subtype; }

    bool is_good();

    bool initialise(std::string expr_);

    bool match(std::string& text);
    bool match(std::string& text, pcre2_item& annots);
    bool match(std::string& text, nlohmann::json& annots);
    
    bool find_all(std::string& text, nlohmann::json& annots);
    bool find_all(std::string& text, std::vector<pcre2_item>& annots);

    bool replace_all(std::string& text, std::string repl);
    
  private:

    bool valid(int rc);

    bool get_groups(PCRE2_SIZE& ind, PCRE2_SIZE& len,
		    std::string& text, pcre2_item& item);

  private:

    std::string type;
    std::string subtype;

    std::string expr;

    PCRE2_SPTR pattern;
    PCRE2_SIZE plength;

    pcre2_code*       re;
    pcre2_match_data* match_data;

    std::vector<std::string> group_names;
  };

  pcre2_expr::pcre2_expr():
    expr("null")
  {}

  pcre2_expr::pcre2_expr(std::string type_,
                         std::string subtype_,
                         std::string expr_):
    type(type_),
    subtype(subtype_),

    expr(expr_)
  {
    initialise(expr_);
  }

  pcre2_expr::~pcre2_expr()
  {}

  bool pcre2_expr::initialise(std::string expr_)
  {
    //LOG_S(INFO) << "type: " << type << ", subtype: " << subtype;
    //LOG_S(INFO) << "expr: '" << expr << "'";

    expr = expr_;

    pattern = (PCRE2_SPTR) expr.c_str();
    plength = expr.size();

    int        errorcode = 0;
    PCRE2_SIZE erroroffset = 0;

    re = pcre2_compile(pattern, plength, 0, &errorcode, &erroroffset, NULL);

    if (re == NULL) {
      PCRE2_UCHAR buffer[256];
      pcre2_get_error_message(errorcode, buffer, sizeof(buffer));

      LOG_S(ERROR) << "PCRE2 compilation for `" << type << ", " << subtype << "` "
		   << "failed at offset " << (int)erroroffset << ": " << buffer
		   << " with expr: `" << expr_ << "`";
      return false;
    }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    PCRE2_SIZE namecount=0;

    pcre2_pattern_info(re,                   /* the compiled pattern */
                       PCRE2_INFO_NAMECOUNT, /* get the number of named substrings */
                       &namecount);          /* where to put the answer */

    if(namecount==0)
      {
        //LOG_S(WARNING) << "no named groups";
        group_names = {};
      }
    else
      {
        PCRE2_SPTR name_table;
        PCRE2_SPTR tabptr;
        uint32_t name_entry_size;

        //LOG_S(WARNING) << "named groups: " << namecount;

        /* Before we can access the substrings, we must extract the table for
           translating names to numbers, and the size of each entry in the table. */

        (void)pcre2_pattern_info(re,                       /* the compiled pattern */
                                 PCRE2_INFO_NAMETABLE,     /* address of the table */
                                 &name_table);             /* where to put the answer */

        (void)pcre2_pattern_info(re,                       /* the compiled pattern */
                                 PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
                                 &name_entry_size);        /* where to put the answer */

        /* Now we can scan the table and, for each entry, print the number, the name,
           and the substring itself. In the 8-bit library the number is held in two
           bytes, most significant first. */

        //LOG_S(INFO) << "before iterating: " << &name_table;

        group_names = {};

        tabptr = name_table;
        for(PCRE2_SIZE i=0; i<namecount; i++)
          {
            // int n = (tabptr[0] << 8) | tabptr[1];
            //LOG_S(WARNING) << i << ": [" << n << ", " << name_entry_size-3 << "] with name: " << tabptr+2;

            //printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2//,
            //(int)(ovector[2*n+1] - ovector[2*n]), subject + ovector[2*n]
            //     );

            std::string name= (const char*)(tabptr+2);
            group_names.push_back(name);

            tabptr += name_entry_size;
          }
      }

    return true;
  }

  bool pcre2_expr::match(std::string& text)
  {
    PCRE2_SPTR subject = (PCRE2_SPTR) text.c_str();
    PCRE2_SIZE length = text.size();
    
    PCRE2_SIZE ind=0;
    PCRE2_SIZE len=0;
    
    int rc = pcre2_match(re,           /* the compiled pattern */
                         subject,      /* the subject string */
                         length,       /* the length of the subject */
                         ind+len,      /* start at offset 0 in the subject */
                         0,            /* default options */
                         match_data,   /* block for storing the result */
                         NULL);        /* use default match context */
    
    if(not valid(rc)) { return false; }

    PCRE2_SIZE ocount = pcre2_get_ovector_count(match_data);
    if(ocount==0) { return false; }
    
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
    if(ovector[0]==0 and ovector[1]==text.size()) { return true; }
    
    return false;
  }

  bool pcre2_expr::match(std::string& text, pcre2_item& annots)
  {
    PCRE2_SPTR subject = (PCRE2_SPTR) text.c_str();
    PCRE2_SIZE length = text.size();

    PCRE2_SIZE ind=0;
    PCRE2_SIZE len=0;

    int rc = pcre2_match(re,           /* the compiled pattern */
                         subject,      /* the subject string */
                         length,       /* the length of the subject */
                         ind+len,      /* start at offset 0 in the subject */
                         0,            /* default options */
                         match_data,   /* block for storing the result */
                         NULL);        /* use default match context */

    if(not valid(rc)) { return false; }
    
    PCRE2_SIZE ocount = pcre2_get_ovector_count(match_data);
    if(ocount==0) { return false; }
    
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
   
    if(ovector[0]==0 and ovector[1]==text.size())
      {
	return get_groups(ind, len, text, annots);
      }

    return false;
  }
  
  bool pcre2_expr::match(std::string& text, nlohmann::json& annots)
  {
    pcre2_item item;
    if(match(text, item))
      {
	annots = item.to_json();
	return true;
      }

    return false;
  }

  bool pcre2_expr::find_all(std::string& text, nlohmann::json& annots)
  {
    if(not annots.is_array())
      {
        annots = nlohmann::json::array({});
      }

    std::vector<pcre2_item> items={};
    if(find_all(text, items))
      {
	for(auto& item:items)
	  {
	    annots.push_back(item.to_json());
	  }
      }

    return true;
  }
  
  bool pcre2_expr::find_all(std::string& text, std::vector<pcre2_item>& annots)    
  {
    PCRE2_SPTR subject = (PCRE2_SPTR) text.c_str();
    PCRE2_SIZE length = text.size();

    PCRE2_SIZE ind=0;
    PCRE2_SIZE len=0;

    while(ind+len<text.size())
      {
        int rc = pcre2_match(re,           /* the compiled pattern */
                             subject,      /* the subject string */
                             length,       /* the length of the subject */
                             ind+len,      /* start at offset 0 in the subject */
                             0,            /* default options */
                             match_data,   /* block for storing the result */
                             NULL);        /* use default match context */

        if(not valid(rc))
          {
            break;
          }

	pcre2_item ent;
	if(get_groups(ind, len, text, ent))
	  {
	    annots.push_back(ent);
	  }
      }

    return true;
  }

  bool pcre2_expr::valid(int rc)
  {
    if(rc<0)
      {
        switch(rc)
          {
          case PCRE2_ERROR_NOMATCH: break;
          default: { LOG_S(ERROR) << "pcre2 error with code " << rc; }
          }

        return false;
      }

    return true;
  }

  bool pcre2_expr::get_groups(PCRE2_SIZE& ind, PCRE2_SIZE& len,
			      std::string& text, pcre2_item& item)
  {
    if(match_data==NULL){
      //LOG_S(ERROR) << "no match-data ...";
      return false;
    }

    PCRE2_SIZE  ocount  = pcre2_get_ovector_count(match_data);
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);

    ind = ovector[0];
    len = ovector[1]-ovector[0];

    {
      item.text = text.substr(ind, len);
      
      item.type = type;
      item.subtype = subtype;

      //item.i = ind;
      //item.j = ind+len;
      item.rng = {ind, ind+len};
      
      item.groups = {};
    }

    for(PCRE2_SIZE l=0; l<ocount; l++)
      {
        PCRE2_SIZE i=ovector[2*l+0];
        PCRE2_SIZE j=ovector[2*l+1];

	if(j==std::string::npos)
	  {
	    // subgroup not present ...
	    continue;
	  }	
	else if(j>text.size())
	  {
	    LOG_S(WARNING) << " skipping group " << l << ": " << i << ", " << j << ", " << text.size();
	    continue;
	  }
	
	pcre2_group group;
        {
	  group.text = text.substr(i, j-i);
	  group.rng = {i,j};

	  group.group_name = "null";
	  group.group_index = l;		    
        }
        item.groups.push_back(group);
      }

    for(auto name:group_names)
      {
	std::size_t index = pcre2_substring_number_from_name(re, (PCRE2_SPTR)name.c_str());

        if(index<item.groups.size())
          {
            item.groups.at(index).group_name = name;
          }
	else
	  {
	    LOG_S(WARNING) << "index " << index << " out of bounds for group-name " << name << " "
			 << "for " << type << " (" << subtype << ")";

	    LOG_S(INFO) << "text: " << text;
	    LOG_S(INFO) << "expression: " << expr;
	    LOG_S(INFO) << "found groups are: ";
	    for(auto& group:item.groups)
	      {
		LOG_S(INFO) << " -> group-index: " << group.group_index << "; "
			    << "group-name: " << group.group_name << "; "
			    << "[i,j]: " << group.rng.at(0) << ", " << group.rng.at(1) << "; "
			    << "text: " << group.text;
	      }
	  }
      }

    return true;
  }

  bool pcre2_expr::replace_all(std::string& text, std::string repl)
  {
    std::vector<pcre2_item> items;
    this->find_all(text, items);

    for(auto& item:items)
      {
	text = utils::replace(text, item.text, repl);
      }

    return true;
  }
  
}

#endif
