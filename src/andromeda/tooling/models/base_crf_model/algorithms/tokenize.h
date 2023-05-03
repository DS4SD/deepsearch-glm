//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_TOKENIZE_H_
#define ANDROMEDA_BASE_CRF_TOKENIZE_H_

#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

namespace andromeda_crf
{
  static void replace(std::string & s,
                      const std::string & s1,
                      const std::string & s2,
                      const char skip = 0,
                      bool left = true);

  static void separate_commas(std::string & s)
  {
    const int n = s.size();

    std::string t;
    for (int i = 0; i < n; i++) {
      if (s[i] == ',') {
        if ( !(i > 0 && isdigit(s[i-1]) && i < n - 1 && isdigit(s[i+1]) ) ) {
          t += " , ";
          continue;
        }
      }
      t += std::string(1, s[i]);
    }

    s = t;
  }

  void tokenize(const std::string & s1, std::vector<std::string> & lt)
  {
    LOG_S(INFO) << "using U-Penn tokenization ...";

    if (s1.size() == 0) return;

    lt.clear();

    std::string s(s1);

    replace(s, "``", " `` ");

    if (s[0] == '"')
      {
        s.replace(0, 1, "`` ");
      }

    if (s.size() > 2 && s[0] == '`' && s[1] != '`')
      {
        s.replace(0, 1, "` ");
      }

    replace(s, " \"", "  `` ");
    replace(s, "(\"", "( `` ");
    replace(s, "[\"", "[ `` ");
    replace(s, "{\"", "{ `` ");
    replace(s, "<\"", "< `` ");
    replace(s, " `", "  ` ", '`', false);
    replace(s, "(`", "( ` ", '`', false);
    replace(s, "[`", "[ ` ", '`', false);
    replace(s, "{`", "{ ` ", '`', false);
    replace(s, "<`", "< ` ", '`', false);

    replace(s, "...", " ... ");

    //replace(s, ",", " , ");
    separate_commas(s);
    replace(s, ";", " ; ");
    replace(s, ":", " : ");
    replace(s, "@", " @ ");
    replace(s, "#", " # ");
    replace(s, "$", " $ ");
    replace(s, "%", " % ");
    replace(s, "&", " & ");

    int pos = s.size() - 1;

    while (pos > 0 && s[pos] == ' ') pos--;


    while (pos > 0)
      {
        char c = s[pos];
        if (c == '[' || c == ']' || c == ')' || c == '}' || c == '>' ||
            c == '"' || c == '\'') {
          pos--; continue;
        }
        break;
      }

    if (s[pos] == '.' && !(pos > 0 && s[pos-1] == '.'))
      s.replace(pos, 1, " .");

    replace(s, "?", " ? ");
    replace(s, "!", " ! ");

    replace(s, "[", " [ ");
    replace(s, "]", " ] ");
    replace(s, "(", " ( ");
    replace(s, ")", " ) ");
    replace(s, "{", " { ");
    replace(s, "}", " } ");
    replace(s, "<", " < ");
    replace(s, ">", " > ");

    replace(s, "--", " -- ");

    s.replace(std::string::size_type(0), 0, " ");
    s.replace(s.size(), 0, " ");

    replace(s, "''", " '' ");
    replace(s, "\"", " '' ");

    replace(s, "' ", " ' ", '\'');
    replace(s, "'s ", " 's ");
    replace(s, "'S ", " 'S ");
    replace(s, "'m ", " 'm ");
    replace(s, "'M ", " 'M ");
    replace(s, "'d ", " 'd ");
    replace(s, "'D ", " 'D ");
    replace(s, "'ll ", " 'll ");
    replace(s, "'re ", " 're ");
    replace(s, "'ve ", " 've ");
    replace(s, "n't ", " n't ");
    replace(s, "'LL ", " 'LL ");
    replace(s, "'RE ", " 'RE ");
    replace(s, "'VE ", " 'VE ");
    replace(s, "N'T ", " N'T ");

    replace(s, " Cannot ", " Can not ");
    replace(s, " cannot ", " can not ");
    replace(s, " D'ye ", " D' ye ");
    replace(s, " d'ye ", " d' ye ");
    replace(s, " Gimme ", " Gim me ");
    replace(s, " gimme ", " gim me ");
    replace(s, " Gonna ", " Gon na ");
    replace(s, " gonna ", " gon na ");
    replace(s, " Gotta ", " Got ta ");
    replace(s, " gotta ", " got ta ");
    replace(s, " Lemme ", " Lem me ");
    replace(s, " lemme ", " lem me ");
    replace(s, " More'n ", " More 'n ");
    replace(s, " more'n ", " more 'n ");
    replace(s, "'Tis ", " 'T is ");
    replace(s, "'tis ", " 't is ");
    replace(s, "'Twas ", " 'T was ");
    replace(s, "'twas ", " 't was ");
    replace(s, " Wanna ", " Wan na ");
    //  replace(s, " wanna ", " wanna ");
    replace(s, " wanna ", " wan na ");

    if (s[s.size() - 1] == '\'')
      s.replace(s.size() - 1, 1, " ' ");

    std::istringstream is(s);
    std::string t;

    while (is >> t)
      {
        lt.push_back(t);
      }
  }

  static void replace(std::string & s,
                      const std::string & s1,
                      const std::string & s2,
                      const char skip,
                      bool left)
  {
    std::string::size_type pos = 0;

    while (1)
      {
        std::string::size_type i = s.find(s1, pos);

        if (i == std::string::npos) break;

        if (skip)
          {
            if (left && i > 0 && s[i-1] == skip)
              {
                pos = i + 1;
                continue;
              }
            else if (i < s.size() - 1 && s[i+1] == skip)
              {
                pos = i + 1;
                continue;
              }
          }

        s.replace(i, s1.size(), s2);

        pos = i + s2.size();
      }
  }

  void tokenize(const std::string& s,
                std::vector<utils::crf_token> & vt,
                const bool use_upenn_tokenizer)
  {
    std::vector<std::string> vs;
    if(use_upenn_tokenizer)
      {
        tokenize(s, vs);
      }
    else
      {
        std::istringstream is(s);
        std::string token;

        while(is >> token)
          {
            vs.push_back(token);
          }
      }

    int begin = 0;
    for(std::vector<std::string>::const_iterator i = vs.begin(); i != vs.end(); i++)
      {
        std::string::size_type x = s.find(*i, begin);
        int strlen = i->size();

        if (*i == "''")
          {
            std::string::size_type y = s.find("\"", begin);
            if (y != std::string::npos && (x == std::string::npos || y < x))
              {
                x = y;
                strlen = 1;
              }
          }

        if (*i == "``")
          {
            std::string::size_type y = s.find("\"", begin);
            if (y != std::string::npos && (x == std::string::npos || y < x))
              {
                x = y;
                strlen = 1;
              }
          }

        if(x == std::string::npos)
          {
            LOG_S(ERROR) << "internal error: tokenization failed.";
            LOG_S(ERROR) << "input = " << s;

            LOG_S(FATAL) << "aborting ...";
          }

        const int end = x + strlen;
        vt.push_back(utils::crf_token(*i, x, end));
        begin = end;
      }
  }

  //}

}

#endif
