//-*-C++-*-

#ifndef ANDROMEDA_CRF_PARENTHESIS_CONVERTER_H
#define ANDROMEDA_CRF_PARENTHESIS_CONVERTER_H

#include <string>
#include <vector>
#include <map>

namespace andromeda_crf
{
  namespace utils
  {
    class parenthesis_converter
    {
    public:

      parenthesis_converter();

      std::string Ptb2Pos(const std::string& text);
      std::string Pos2Ptb(const std::string& text);

    private:

      std::map<std::string, std::string> ptb2pos;
      std::map<std::string, std::string> pos2ptb;
    };
    
    parenthesis_converter::parenthesis_converter():
      ptb2pos(),
      pos2ptb()
    {
      ptb2pos = {{"-LRB-", "("},
                 {"-RRB-", ")"},
                 {"-LSB-", "["},
                 {"-RSB-", "]"},
                 {"-LCB-", "{"},
                 {"-RCB-", "}"},
                 {"***", "***"}};

      for(auto itr=ptb2pos.begin(); itr!=ptb2pos.end(); itr++)
        {
          pos2ptb.insert({itr->second, itr->first});
        }

      /*
        const static char* table[] = {
        "-LRB-", "(",
        "-RRB-", ")",
        "-LSB-", "[",
        "-RSB-", "]",
        "-LCB-", "{",
        "-RCB-", "}",
        "***", "***",
        };

        for (int i = 0;; i+=2)
        {
        if (std::string(table[i]) == "***")
        {
        break;
        }

        ptb2pos.insert(std::make_pair(table[i], table[i+1]));
        pos2ptb.insert(std::make_pair(table[i+1], table[i]));
        }
      */
    }

    std::string parenthesis_converter::Ptb2Pos(const std::string& text)
    {
      auto itr = ptb2pos.find(text);

      if(itr==ptb2pos.end())
        {
          return text;
        }

      return itr->second;
    }

    std::string parenthesis_converter::Pos2Ptb(const std::string& text)
    {
      auto itr = pos2ptb.find(text);

      if(itr==pos2ptb.end())
        {
          return text;
        }

      return itr->second;
    }

  }

}

#endif
