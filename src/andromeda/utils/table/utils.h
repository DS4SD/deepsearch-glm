//-*-C++-*-

#ifndef ANDROMEDA_UTILS_TABLE_UTILS_H_
#define ANDROMEDA_UTILS_TABLE_UTILS_H_

namespace andromeda
{
  namespace utils
  {
    
    std::string to_string(std::vector<std::string>& header,
                          std::vector<std::vector<std::string> >& data,
                          std::size_t max_cols=-1)
    {
      max_cols = std::min(max_cols, header.size());

      std::vector<std::size_t> len(max_cols, 0);

      for(std::size_t j=0; j<len.size(); j++)
        {
	  std::size_t len_ = utf8::distance(header.at(j).c_str(),
					    header.at(j).c_str()+header.at(j).size());
	  
          len.at(j) = std::max(len.at(j), len_);
        }

      for(std::size_t i=0; i<data.size(); i++)
        {
          for(std::size_t j=0; j<max_cols; j++)
            {
	      std::size_t len_ = utf8::distance(data.at(i).at(j).c_str(),
						data.at(i).at(j).c_str()+data.at(i).at(j).size());
	      
              len.at(j) = std::max(len.at(j), len_);
            }
        }

      std::stringstream ss;

      ss << "| ";
      for(std::size_t j=0; j<max_cols; j++)
        {
          ss << std::string(len.at(j), '-') << " | ";
        }
      ss << "\n";

      ss << "| ";
      for(std::size_t j=0; j<max_cols; j++)
        {
          ss << std::setw(len.at(j)) << header.at(j) << " | ";
        }
      ss << "\n";

      ss << "| ";
      for(std::size_t j=0; j<max_cols; j++)
        {
          ss << std::string(len.at(j), '-') << " | ";
        }
      ss << "\n";

      for(std::size_t i=0; i<data.size(); i++)
        {
	  if(data.at(i).at(0)=="total (cnt)")
	    {
	      ss << "| ";
	      for(std::size_t j=0; j<max_cols; j++)
		{
		  ss << std::string(len.at(j), '-') << " | ";
		}
	      ss << "\n";	      
	    }
	  
          ss << "| ";
          for(std::size_t j=0; j<max_cols; j++)
            {
              ss << std::setw(len.at(j)) << data.at(i).at(j) << " | ";
            }
          ss << "\n";
        }

      ss << "| ";
      for(std::size_t j=0; j<max_cols; j++)
        {
          ss << std::string(len.at(j), '-') << " | ";
        }
      ss << "\n";

      return ss.str();
    }

    std::string to_string(std::string caption,
			  std::vector<std::string>& header,
                          std::vector<std::vector<std::string> >& data,
                          std::size_t max_cols=-1)
    {
      std::stringstream ss;

      ss << "\n" << "caption: " << caption << "\n\n" << to_string(header, data, max_cols);

      return ss.str();
    }

    
    bool to_csv(std::ofstream ofs,
		       std::vector<std::string>& header,
		       std::vector<std::vector<std::string> >& data)
    {
      if(not ofs)
	{
	  return false;
	}

      return true;
    }

    template<int ind, typename ... tuple_types>
    class write_row
    {
      typedef std::tuple<tuple_types ...> row_type;
      const inline static int N = std::tuple_size_v<row_type>;      
      
    public:

      static bool to_ofs(std::ofstream& ofs, std::tuple<tuple_types ...>& row)
      {	
	ofs << std::get<N-ind>(row) << ", ";
	return write_row<ind-1, tuple_types ... >::to_ofs(ofs, row);	
      }

      static bool to_row(std::tuple<tuple_types ...>& row,
			 std::vector<std::string>&    row_)
      {
	auto elem = std::get<N-ind>(row);	
	row_.push_back(std::to_string(elem));

	return write_row<ind-1, tuple_types ... >::to_row(row, row_);
      }      
    };

    template<typename ... tuple_types>
    class write_row<0, tuple_types ...>
    {
      typedef std::tuple<tuple_types ...> row_type;
      const inline static int N = std::tuple_size_v<row_type>;      
      
    public:

      static bool to_ofs(std::ofstream& ofs, std::tuple<tuple_types ...>& row)
      {
	ofs << "\n";	
	return true;
      }

      static bool to_row(std::tuple<tuple_types ...>& row,
			 std::vector<std::string>&    row_)
      {
	return true;
      }
    };
    
    template<typename ... tuple_types>
    bool to_csv(std::filesystem::path path,
		std::vector<std::string>& header,
		std::vector<std::tuple<tuple_types ...> >& data)
    {
      typedef std::tuple<tuple_types ...> row_type;

      std::ofstream ofs(path);
      if(not ofs)
	{
	  return false;
	}

      for(std::size_t l=0; l<header.size(); l++)
	{
	  if(l+1==header.size())
	    {
	      ofs << header.at(l) << "\n";
	    }
	  else
	    {
	      ofs << header.at(l) << ",";
	    }
	}

      for(std::size_t i=0; i<data.size(); i++)
	{
	  write_row<std::tuple_size_v<row_type>,
		    tuple_types ...>::to_ofs(ofs, data.at(i));
	}
      
      return true;
    }    

    template<typename ... tuple_types>
    std::string to_string(std::vector<std::string>& header,
			  std::vector<std::tuple<tuple_types ...> >& data)
    {
      typedef std::tuple<tuple_types ...> row_type;

      std::vector<std::vector<std::string> > rows={};
      for(std::size_t i=0; i<data.size(); i++)
	{
	  std::vector<std::string> row={};

	  write_row<std::tuple_size_v<row_type>,
		    tuple_types ...>::to_row(data.at(i), row);
	  
	  rows.push_back(row);
	}

      return to_string(header, rows);
    }
    
    template<typename ... tuple_types>
    std::string to_string(std::string caption,
			  std::vector<std::string>& header,
			  std::vector<std::tuple<tuple_types ...> >& data)
    {
      typedef std::tuple<tuple_types ...> row_type;

      std::vector<std::vector<std::string> > rows={};
      for(std::size_t i=0; i<data.size(); i++)
	{
	  std::vector<std::string> row={};

	  write_row<std::tuple_size_v<row_type>,
		    tuple_types ...>::to_row(data.at(i), row);

	  rows.push_back(row);
	}

      return to_string(caption, header, rows);
    }

    template<typename ... tuple_types>
    bool to_txt(std::filesystem::path path,
		std::vector<std::string>& header,
		std::vector<std::tuple<tuple_types ...> >& data)
    {
      std::ofstream ofs(path);
      if(not ofs)
	{
	  return false;
	}

      ofs << to_string(header, data);
      return true;
    }
    
  }

}

#endif
