//-*-C++-*-

#ifndef ANDROMEDA_ENUMS_MODELS_H_
#define ANDROMEDA_ENUMS_MODELS_H_

namespace andromeda
{

  enum model_type
    {
     TOK=0, // tokenizer for strings
     POS=1, // part-of-speach (POS) tagging of text, table
     CLS=2, // classification of text, table
     ENT=3, // entity (with normalisation) in text, table
     REL=4, // binary relation between entities
     REC=5, // record (combination between entities and relations)
     GLM=6 // graph language model
    };

  // the numbers are only there to make sure that the
  // ordering is consistent
  enum model_name
    {
     // TOK
     SPM=64,
     
     // POS
     LAPOS=128,

     // CLS
     LANGUAGE=256,
     SEMANTIC=257, // header, text, reference, etc
     TOPIC=258,

     // ENT
     NUMVAL=512, // numerical value's
     GEOLOC=513,
     DATE=514, // dates
     CITE=515,
     LINK=516, // weblinks in text
     NAME=517, // person/organisation names
     
     
     QUOTE=564, // numerical value's
     PARENTHESIS=565, // anything in brackets
     EXPRESSION=566, // anything concatencated
     
     SENTENCE=600,
     REFERENCE=601,

     CUSTOM_CRF=666,
     
     // POS-patterns
     CONN=700,
     TERM=701,
     VERB=702,
     
     // REL
     ABBREVIATION=1000,

     NULL_MODEL=-1
    };

  const static std::vector<model_name> MODEL_NAMES =
    {
     // TOK
     SPM,
     
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

     CUSTOM_CRF,
          
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
	// TOK
      case SPM: return "SPM";
	
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
	//case ORGANISATION: return "ORGANISATION";

      case CUSTOM_CRF: return "CUSTOM_CRF";
	
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

    if(uname.starts_with(to_string(CUSTOM_CRF)))
      {
	return CUSTOM_CRF;
      }
    
    for(auto mname:MODEL_NAMES)
      {
	if(uname==to_string(mname))
	  {
	    return mname;
	  }
      }

    LOG_S(WARNING) << "could not find model with name: " << name;    
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
