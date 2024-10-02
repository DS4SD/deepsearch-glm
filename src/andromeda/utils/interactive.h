//-*-C++-*-

#ifndef ANDROMEDA_UTILS_INTERACTIVE_H_
#define ANDROMEDA_UTILS_INTERACTIVE_H_

namespace andromeda
{
  
  namespace utils
  {
    template<typename value_type>
    value_type round_value(value_type val)
    {
      return int((1.e4)*val)/(1.e2);
    }
    
    std::string show_string(std::string text, std::stringstream& ss,
                            std::size_t indent_=0, std::size_t max_width=70)
    {
      std::vector<std::string> words = split(text, " ");

      std::string indent(indent_, ' ');

      std::size_t len=indent_;
      for(auto word:words)
        {
          if(len==0)
            {
              ss << word << " ";
              len += word.size()+1;
            }
          else if(len>indent.size()+max_width)
            {
              ss << "\n" << indent << word << " ";
              len = word.size()+indent.size()+1;
            }
          else
            {
              ss << word << " ";
              len += word.size()+1;
            }
        }
      ss << "\n";

      return ss.str();
    }

    std::string show_list(std::vector<std::string>& data,
			  std::stringstream& ss, std::size_t max_width=70)
    {
      for(std::size_t l=0; l<data.size(); l++)
	{
	  std::string tag = "["+std::to_string(l)+"] ";

	  ss << tag;
	  show_string(data.at(l), ss, tag.size(), max_width);
	}
      ss << "\n";

      return ss.str();
    }

    std::string show_table(std::vector<std::vector<std::string> >& data,
                           std::stringstream& ss,
			   std::size_t max_width=32,
			   std::size_t max_cols=6)
    {
      //std::size_t max_cols=0;
      for(std::size_t i=0; i<data.size(); i++)
        {
	  max_cols = std::max(max_cols, data[i].size());
	}
      
      std::vector<std::size_t> widths(max_cols, 1);
      for(std::size_t i=0; i<data.size(); i++)
        {
          for(std::size_t j=0; j<data.at(i).size(); j++)
            {
              auto& elem = data.at(i).at(j);
              widths.at(j) = std::max(widths.at(j), elem.size());
              widths.at(j) = std::min(widths.at(j), max_width);
            }
        }

      for(std::size_t i=0; i<data.size(); i++)
        {
          ss << "\n| ";
          for(std::size_t j=0; j<data.at(i).size(); j++)
            {
              auto elem = data.at(i).at(j);
              ss << std::setw(widths.at(j)) << elem << " | ";
            }
        }
      ss << "\n";

      return ss.str();      
    }
    
    std::string show_table(std::vector<std::vector<std::string> >& data,
                           std::vector<std::string> headers,
                           std::stringstream& ss,
			   std::size_t max_width=70,
			   std::size_t max_cols=6)
    {
      std::vector<std::size_t> widths(headers.size(), 1);
      for(std::size_t j=0; j<headers.size(); j++)
        {
          auto elem = headers.at(j);
          widths.at(j) = std::max(widths.at(j), elem.size());
        }

      for(std::size_t i=0; i<data.size(); i++)
        {
          for(std::size_t j=0; j<headers.size(); j++)
            {
              auto elem = data.at(i).at(j);

              widths.at(j) = std::max(widths.at(j), elem.size());
              widths.at(j) = std::min(widths.at(j), max_width);
            }
        }

      ss << "\n| ";
      for(std::size_t j=0; j<headers.size(); j++)
        {
          auto header = headers.at(j);
          ss << std::setw(widths.at(j)) << header << " | ";
        }

      ss << "\n| ";
      for(std::size_t j=0; j<headers.size(); j++)
        {
          std::string tmp(widths.at(j), '-');
          ss << std::setw(widths.at(j)) << tmp << " | ";
        }

      for(std::size_t i=0; i<data.size(); i++)
        {
          ss << "\n| ";
          for(std::size_t j=0; j<headers.size(); j++)
            {
              auto elem = data.at(i).at(j);
              ss << std::setw(widths.at(j)) << elem << " | ";
            }
        }
      ss << "\n";

      return ss.str();
    }

  }

}

#endif
