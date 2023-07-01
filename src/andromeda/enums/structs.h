//-*-C++-*-

#ifndef ANDROMEDA_ENUMS_STRUCTS_H_
#define ANDROMEDA_ENUMS_STRUCTS_H_

namespace andromeda
{
  enum subject_name { UNDEF,
		      TEXT, PROMPT,
		      PARAGRAPH, TABLE, FIGURE,
		      DOCUMENT};

  const static std::vector<subject_name> SUBJECT_NAMES =
    {
     UNDEF,
     TEXT, PROMPT,
     PARAGRAPH, TABLE, FIGURE,
     DOCUMENT
    }; 
  
  std::string to_string(subject_name name)
  {
    switch(name)
      {
      case UNDEF: return "UNDEF";

      case TEXT: return "TEXT";
      case PROMPT: return "PROMPT";
	
      case PARAGRAPH: return "PARAGRAPH";
      case TABLE: return "TABLE";
      case FIGURE: return "FIGURE";
	
      case DOCUMENT: return "DOCUMENT";
      }

    return "UNKNOWN_SUBJECT";
  }

  subject_name to_subject_name(std::string name)    
  {
    std::string uname = utils::to_upper(utils::strip(name));

    for(auto subj_name:SUBJECT_NAMES)
      {
	if(uname==to_string(subj_name))
	  {
	    return subj_name;
	  }
      }

    return UNDEF;
  }
  
}

#endif
