//-*-C++-*-

#ifndef ANDROMEDA_ENUMS_STRUCTS_H_
#define ANDROMEDA_ENUMS_STRUCTS_H_

namespace andromeda
{
  enum subject_name { UNDEF=-1,
		      PROMPT=0,
		      DOCUMENT=1,
		      TEXT=2,
		      TABLE=3,
		      FIGURE=4};

  const static std::vector<subject_name> SUBJECT_NAMES =
    {
     UNDEF,
     PROMPT,
     DOCUMENT,
     TEXT,
     TABLE,
     FIGURE
    }; 
  
  std::string to_string(subject_name name)
  {
    switch(name)
      {
      case UNDEF: return "UNDEF";
	
      case PROMPT: return "PROMPT";

      case DOCUMENT: return "DOCUMENT";
	
      case TEXT: return "TEXT";
      case TABLE: return "TABLE";
      case FIGURE: return "FIGURE";
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
