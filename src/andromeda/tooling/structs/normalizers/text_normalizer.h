//-*-C++-*-

#ifndef ANDROMEDA_UTILS_TEXT_NORMALISER_H_
#define ANDROMEDA_UTILS_TEXT_NORMALISER_H_

namespace andromeda
{
  namespace utils
  {
    class text_normaliser
    {
    public:

      text_normaliser(bool verbose);
      ~text_normaliser();

      void normalise(std::string& text);

    private:

      bool initialise(bool verbose);

      void apply_text_exprs(std::string& text);
      
      void apply_latex_chars(std::string& text);
      
      void apply_latex_cmds(std::string& text);
      
    private:

      std::vector<pcre2_expr> text_exprs;
      
      std::vector<pcre2_expr> latex_quotes;

      std::set<std::string>   latex_cmds;
      std::vector<pcre2_expr> latex_exprs;
    };

    text_normaliser::text_normaliser(bool verbose)
    {
      latex_cmds = {"rm", "it", "bf", "bm", "em", "emph", "sc", "sf", "Bbb",
                    "vec", "bar", "hat", "cal", "tilde", "widetilde",
                    "textit", "textbf", "textrm", "textsf", "texttt", "text",
                    "mathcal", "mathbb", "mathrm", "mathbf", "mathit", "mathsc", "mathsf"};
      
      initialise(verbose);
    }

    text_normaliser::~text_normaliser()
    {}

    bool text_normaliser::initialise(bool verbose)
    {
      {
	// `J¨urgen`, `St¨ utzle`, ...
        {
          std::string _ = R"((¨)\s?(?P<char>[A-Za-z]))";

          pcre2_expr expr("text-normalisation", "text-expr", _);
          text_exprs.push_back(expr);
        }	
      }
      
      // https://en.wikibooks.org/wiki/LaTeX/Special_Characters
      {
	// `Schr\"{o}dinger`
        {
          std::string _ = R"(\\(\"|\'|\`)\{(?P<char>[A-Za-z])\})";

          pcre2_expr expr("text-normalisation", "latex-quote", _);
          latex_quotes.push_back(expr);
        }
	
	// `Schr\"odinger`
        {
          std::string _ = R"(\\(\"|\'|\`)(?P<char>[A-Za-z]))";

          pcre2_expr expr("text-normalisation", "latex-quote", _);
          latex_quotes.push_back(expr);
        }
      }
      
      // latex commands
      {
        // `{\it metallic}`
        {
          std::string _ = R"(\{\\(?P<latex_command>[A-Za-z]+)\s(?P<content>([^\{\}]+))\})";

          pcre2_expr expr("text-normalisation", "latex-expressions-type-1", _);
          latex_exprs.push_back(expr);
        }

        // `\textit{metallic}`
        {
          std::string _ = R"(\\(?P<latex_command>[A-Za-z]+)\{(?P<content>([^\{\}]+))\})";

          pcre2_expr expr("text-normalisation", "latex-expressions-type-2", _);
          latex_exprs.push_back(expr);
        }
      }

      return true;
    }
    
    void text_normaliser::normalise(std::string& text)
    {
      apply_text_exprs(text);
      
      apply_latex_chars(text);

      apply_latex_cmds(text);
    }

    void text_normaliser::apply_text_exprs(std::string& text)
    {
      for(auto& expr:text_exprs)
        {
          std::vector<pcre2_item> items;
          expr.find_all(text, items);

          for(auto& item:items)
            {
              std::string org = item.text;
              std::string cmd = "";

              for(auto& grp:item.groups)
                {
                  if(grp.group_name=="char")
                    {
                      cmd = grp.text;
		    }
		}
	      
	      text = utils::replace(text, org, cmd);
	    }
	}
    }
    
    void text_normaliser::apply_latex_chars(std::string& text)
    {
      for(auto& expr:latex_quotes)
        {
          std::vector<pcre2_item> items;
          expr.find_all(text, items);

          for(auto& item:items)
            {
              std::string org = item.text;
              std::string cmd = "";

              for(auto& grp:item.groups)
                {
                  if(grp.group_name=="char")
                    {
                      cmd = grp.text;
		    }
		}
	      
	      text = utils::replace(text, org, cmd);
	    }
	}      
    }
    
    void text_normaliser::apply_latex_cmds(std::string& text)
    {
      for(auto& expr:latex_exprs)
        {
          std::vector<pcre2_item> items;
          expr.find_all(text, items);

          for(auto& item:items)
            {
              std::string org = item.text;
              std::string cmd = "";
              std::string cnt = "";

              for(auto& grp:item.groups)
                {
                  if(grp.group_name=="latex_command")
                    {
                      cmd = grp.text;
                    }
                  else if(grp.group_name=="content")
                    {
                      cnt = grp.text;
                    }
                  else
                    {}
                }

              if(latex_cmds.count(cmd))
                {
                  std::string tmp(org.size()-(cnt.size()+1), ' ');
                  tmp += cnt;
                  tmp += " ";

                  text = utils::replace(text, org, tmp);
                }
              else if(expr.get_subtype()=="latex-expressions-type-1")
                {
                  std::string tmp = " \\";
                  tmp += cmd;
                  tmp += "{";
                  tmp += cnt;
                  tmp += "}";
		  
                  text = utils::replace(text, org, tmp);
                }
              else
                {}
            }
        }
    }
    
  }

}

#endif
