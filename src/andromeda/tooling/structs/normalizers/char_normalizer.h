//-*-C++-*-

#ifndef ANDROMEDA_UTILS_CHAR_NORMALISER_H_
#define ANDROMEDA_UTILS_CHAR_NORMALISER_H_

namespace andromeda
{
  namespace utils
  {
    class char_normaliser
    {
    public:

      char_normaliser(bool verbose);

      std::string get(uint32_t& c, std::string& w);

      void tabulate();

    private:
      
      struct norm_token
      {
      public:
	
	uint32_t    orig_int;
	std::string orig_str;
	
	std::vector<uint32_t> norm_ints;
	std::string norm_str;

      public:

	norm_token():
	  orig_int(-1),
	  orig_str("__null__"),
	  norm_ints({}),
	  norm_str("__null__")
	{}
	
	norm_token(uint32_t orig_int,
		   std::string orig_str,
		   std::vector<uint32_t> norm_ints,
		   std::string norm_str):
	  orig_int(orig_int),
	  orig_str(orig_str),
	  norm_ints(norm_ints),
	  norm_str(norm_str)
	{}
      };
      
    private:

      bool initialise(std::filesystem::path resources_dir, bool verbose);
      
      void update_map(std::vector<std::string>& lines,
		      std::vector<norm_token>& tokens);

    private:

      std::filesystem::path confusables_file;

      std::map<uint32_t, norm_token> char_map;
    };

    char_normaliser::char_normaliser(bool verbose)
    {
      initialise(glm_variables::get_resources_dir(), verbose);      
      //tabulate();
    }

    std::string char_normaliser::get(uint32_t& c, std::string& w)
    {
      std::string result;
      
      auto itr = char_map.find(c);
      if(itr!=char_map.end())
	{
	  result = (itr->second).norm_str;
	}
      else if(c<32)
	{
	  result = " ";
	}
      else
	{
	  result = w;
	}

      return result;
    }
    
    void char_normaliser::tabulate()
    {
      std::stringstream ss;
      ss << "\n";

      for(auto itr=char_map.begin(); itr!=char_map.end(); itr++)
	{
	  norm_token& token = itr->second;
	  ss << std::setw(8) << token.orig_int << " | "
	     << std::setw(8) << token.orig_str << " | "
	     << std::setw(16) << token.norm_str << " |\n";
	}
      LOG_S(INFO) << ss.str();
    }
    
    bool char_normaliser::initialise(std::filesystem::path resources_dir, bool verbose)
    {
      char_map.clear();

      confusables_file = resources_dir / "confusables/confusablesRestricted.txt";
      std::ifstream ifs(confusables_file.c_str());

      if(ifs.good() and verbose)
	{
	  LOG_S(INFO) << "reading confusables-file: " << confusables_file.string();
	}
      else if(ifs.good())
	{ }
      else //if(verbose)	
        {
          LOG_S(ERROR) << "could not open confusables-file: " << confusables_file.string();
          return false;
        }
      
      std::vector<std::string> lines={};

      std::size_t cnt=0;
      std::string line="";
      while(std::getline(ifs, line))
        {
          line = strip(line);
          //LOG_S(INFO) << "[" << cnt << "]: " << line;

          if(line.size()==0)
            {

            }
          else if(line.starts_with("#") or cnt==0)
            {
              if(lines.size()>0)
                {
		  std::vector<norm_token> tokens;

                  update_map(lines, tokens);
                  lines.clear();

		  for(auto& token:tokens)
		    {
		      char_map[token.orig_int] = token;
		    }
                }
            }
          else
            {
              lines.push_back(line);
            }

          cnt += 1;
        }

      return true;
    }

    void char_normaliser::update_map(std::vector<std::string>& lines,
				     std::vector<norm_token>& tokens)
    {
      std::vector<std::vector<std::string> > parts;
      for(auto& line:lines)
        {
          std::vector<std::string> part = split(line, '\t');
          parts.push_back(part);
        }

      tokens.clear();
      for(std::size_t l=0; l<parts.size(); l++)
        {
	  std::vector<std::string> tmp = split(parts.at(l).at(2), " ");

	  std::vector<unsigned int> inds={};
	  for(auto& _:tmp)
            {
	      unsigned int ind = std::stoul(_, nullptr, 16);
	      inds.push_back(ind);
            }

	  if(inds.size()==0)
	    {
	      continue;
	    }
	  
          std::string orig_str="";
	  utf8::utf32to8(inds.begin(), inds.end(), std::back_inserter(orig_str));

	  auto itr = orig_str.c_str();
	  auto end = itr+orig_str.size();

	  std::vector<uint32_t> orig_ints={};
	  while(itr!=end)
	    {
	      orig_ints.push_back(utf8::next(itr, end));
	    }
	  	  
          //std::string label = parts.at(l).at(3);

	  if(orig_ints.size()==1)
	    {
	      tokens.emplace_back(orig_ints.at(0), orig_str,
				  orig_ints, orig_str);//,
				  //label);
	    }
	  /*
	  else if(l==0)
	    {
	      tokens.emplace_back(-1, orig_str,
				  orig_ints, orig_str,
				  label);	      
	    }
	  */
	  else
	    {
	      // we ignore the composed (multiple unicode characters) confusables.
	    }
        }

      for(std::size_t l=1; l<tokens.size(); l++)
        {
          tokens.at(l).norm_ints = tokens.at(0).norm_ints;
          tokens.at(l).norm_str = tokens.at(0).norm_str;

	  /*
	  std::stringstream ss;
	  for(auto _:tokens.at(l).norm_ints)
	    ss << _ << "-";
	  
	  LOG_S(INFO) << "\t" << l << "\t"
		      << "(" << tokens.at(l).orig_str << " ; " << tokens.at(l).orig_int << ") => "
		      << "(" << tokens.at(l).norm_str << " ; " << ss.str() << ")";
	  */
        }
    }

  }

}

#endif
