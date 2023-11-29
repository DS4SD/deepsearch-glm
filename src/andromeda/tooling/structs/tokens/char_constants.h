//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_CONSTANTS_H_
#define ANDROMEDA_SUBJECTS_CONSTANTS_H_

namespace andromeda
{
  struct constants
  {
    const static std::set<std::string> spaces;
    const static std::set<std::string> brackets;
    const static std::set<std::string> numbers;

    const static std::set<std::string> punktuation;

    const static std::set<std::string> special_words;

    const static std::set<std::string> abbreviations;
  };

  const std::set<std::string> constants::spaces={" ", "\n", "\r", "\t", " "}; // non-breaking spaces ...
  const std::set<std::string> constants::brackets={"(", ")", "[", "]", "{", "}"};
  const std::set<std::string> constants::numbers={"0", "1", "2", "3", "4",
						  "5", "6", "7", "8", "9"};
  const std::set<std::string> constants::punktuation={".", ",", ":", ";", "?", "!", 
						      "~", "=", "<", ">", "$", "%",
						      "\"", "'",
						      "/", "+", "-", "_", "^",
						      "‹", "›", "«", "»",
						      "#"};

  const std::set<std::string> constants::special_words={"''"};
  
  const std::set<std::string> constants::abbreviations={"e.g.", "i.e.", "et al.", "etc."};
}

#endif
