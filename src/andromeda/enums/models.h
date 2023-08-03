//-*-C++-*-

#ifndef ANDROMEDA_ENUMS_MODELS_H_
#define ANDROMEDA_ENUMS_MODELS_H_

namespace andromeda
{

  enum model_type
    {
     POS, // part-of-speach (POS) tagging of text, table
     CLS, // classification of text, table
     ENT, // entity (with normalisation) in text, table
     REL, // binary relation between entities
     REC, // record (combination between entities and relations)
     GLM // graph language model
    };

  enum model_name
    {
     // POS
     LAPOS,

     // CLS
     LANGUAGE,
     SEMANTIC, // header, text, reference, etc
     TOPIC,

     // ENT
     NUMVAL, // numerical value's
     GEOLOC,

     DATE, // dates
     CITE,
     LINK, // weblinks in text
     NAME, // person/organisation names

     QUOTE, // numerical value's
     PARENTHESIS, // anything in brackets
     EXPRESSION, // anything concatencated
     
     SENTENCE,
     REFERENCE,

     //LOCATION,
     ORGANISATION,
     
     // POS-patterns
     CONN,
     TERM,
     VERB,
     
     // REL
     ABBREVIATION,

     NULL_MODEL
    };

  const static std::vector<model_name> MODEL_NAMES =
    {
     // POS
     LAPOS,

     // CLS
     LANGUAGE,
     SEMANTIC, // header, text, reference, etc
     TOPIC,

     // ENT
     NUMVAL, // numerical value's
     GEOLOC, // numerical value's
     QUOTE, // numerical value's

     DATE, // dates
     CITE,
     LINK, // weblinks in text
     NAME, // person/organisation names
     
     PARENTHESIS, // anything in brackets
     EXPRESSION, // anything concatencated
     
     SENTENCE,
     REFERENCE,

     //LOCATION,
     ORGANISATION,
     
     // POS-patterns
     CONN,
     TERM,
     VERB,
     
     // REL
     ABBREVIATION,

     NULL_MODEL					 
  };
  
  std::string to_string(model_name name)
  {
    switch(name)
      {
        // POS
      case LAPOS: return "LAPOS";

        // CLS
      case LANGUAGE: return "LANGUAGE";
      case SEMANTIC: return "SEMANTIC";
      case TOPIC: return "TOPIC";

        // ENT
      case NUMVAL: return "NUMVAL";
      case GEOLOC: return "GEOLOC";

      case QUOTE: return "QUOTE";
      case PARENTHESIS: return "PARENTHESIS";
      case EXPRESSION: return "EXPRESSION";
	
      case SENTENCE: return "SENTENCE";
      case REFERENCE: return "REFERENCE";

      case DATE: return "DATE";
      case LINK: return "LINK";	
      case NAME: return "NAME";
      case CITE: return "CITE";

	//case LOCATION: return "LOCATION";
      case ORGANISATION: return "ORGANISATION";
	
      case CONN: return "CONN";
      case TERM: return "TERM";
      case VERB: return "VERB";

      case ABBREVIATION: return "ABBREVIATION";

      case NULL_MODEL: return "NULL";
      }

    return "UNKNOWN_NLP_MODEL";
  }

  model_name to_modelname(std::string name)
  {
    std::string uname = utils::to_upper(utils::strip(name));
    
    for(auto mname:MODEL_NAMES)
      {
	if(uname==to_string(mname))
	  {
	    return mname;
	  }
      }

    return NULL_MODEL;    
  }
  
  std::string to_key(model_name name)
  {
    std::string key = to_string(name);

    std::for_each(key.begin(), key.end(),
                  [](char & c){ c = std::tolower(c); } );

    return key;
  }

}

#endif
