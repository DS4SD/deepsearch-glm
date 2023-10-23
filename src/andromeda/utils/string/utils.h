//-*-C++-*-

#ifndef ANDROMEDA_UTILS_STRING_UTILS_H_
#define ANDROMEDA_UTILS_STRING_UTILS_H_

#include <string>
#include <list>
#include <vector>

namespace andromeda
{
  namespace utils
  {
    bool is_alphabetic(const char c)
    {
      return (('a'<=c and c<='z') or
	      ('A'<=c and c<='Z') );      
    }

    bool is_numeric(const uint32_t c)
    {
      return ('0'<=c and c<='9');
    }
    
    bool is_alphabetic(const uint32_t c)
    {
      return (('a'<=c and c<='z') or
	      ('A'<=c and c<='Z') );
    }

    bool is_greek(const uint32_t c)
    {
      /* α = U+03B1; 945 
	 ω = U+03C9; 53129
	 Α = U+0391; 52881
	 Ω = U+03A9; 52905 */	
      
      return ((945<=c and c<=969) or
	      (913<=c and c<=937));	
    }
    
    bool contains(const std::string& text, const std::string& word)
    {
      return (text.find(word)==std::string::npos? false : true);
    }

    std::string replace(std::string text, std::string word_0, std::string word_1)
    {
      if(word_0==word_1)
	{
	  return text;
	}

      std::size_t pos=0;
      while(pos<text.size())
	{			          
	  pos = text.find(word_0, pos);
	  if(pos==std::string::npos)
	    {
	      break;
	    }

	  text.replace(pos, word_0.size(), word_1);
	  pos += word_1.size();
	}
      
      return text;
    }
    
    std::string strip(const std::string& s)
    {
      std::size_t i=0;
      std::size_t j=s.size();

      while(i<s.size() and s[i]==' ')
        {
          i++;
        }

      while(j>0 and s[j-1]==' ')
        {
          j--;
        }

      return s.substr(i, j-i);
    }

    std::string to_string(std::set<std::string> coll, char sep=';')
    {
      std::stringstream ss;
      ss << "";
      
      for(auto itr=coll.begin(); itr!=coll.end(); itr++)
	{
	  if(itr==coll.begin())
	    {
	      ss << *itr;
	    }
	  else
	    {
	      ss << sep << *itr;
	    }
	}

      return ss.str();
    }

    std::string to_string(std::vector<std::string> coll, char sep=';')
    {
      std::stringstream ss;
      ss << "";
      
      for(auto itr=coll.begin(); itr!=coll.end(); itr++)
	{
	  if(itr==coll.begin())
	    {
	      ss << *itr;
	    }
	  else
	    {
	      ss << sep << *itr;
	    }
	}

      return ss.str();
    }    

    std::string to_string(std::list<std::string> coll, char sep=';')
    {
      std::stringstream ss;
      ss << "";
      
      for(auto itr=coll.begin(); itr!=coll.end(); itr++)
	{
	  if(itr==coll.begin())
	    {
	      ss << *itr;
	    }
	  else
	    {
	      ss << sep << *itr;
	    }
	}

      return ss.str();
    }
    
    std::string to_lower(const std::string& s)
    {
      std::string result=s;
      for(std::size_t l=0; l<s.size(); l++)
        {
          result[l] = std::tolower(s[l]);
        }

      return result;
    }

    std::string to_upper(const std::string& s)
    {
      std::string result=s;
      for(std::size_t l=0; l<s.size(); l++)
        {
          result[l] = std::toupper(s[l]);
        }

      return result;
    }

    std::string to_fixed_size(const std::string& line, std::size_t N)
    {      
      std::size_t dst = utf8::distance(line.c_str(), line.c_str()+line.size());

      std::string result = line;

      //LOG_S(INFO) << N << "\t" << line.size() << "\t" << dst << "\t'" << line << "'";
      
      /*
      if(dst<=N)
	{
	  std::string spaces(N-dst, ' ');

	  std::stringstream ss;
	  ss << spaces << line;
	  
	  return ss.str();
	}
      else
      */
      if(dst==N)
	{
	  return result;
	}
      
      if(dst>N)
	{
	  std::vector<std::pair<std::size_t, std::string> > chars;
	  
	  auto start = line.c_str();
	  auto end   = line.c_str()+line.size();
	  
	  auto itr = start;
	  while(itr!=end)
	    {
	      uint32_t c = utf8::next(itr, end);
	      
	      std::string tmp;
	      utf8::append(c, std::back_inserter(tmp));

	      std::size_t dst = utf8::distance(tmp.c_str(), tmp.c_str()+tmp.size());
	      
	      chars.emplace_back(dst, tmp);
	    }

	  std::set<std::size_t> inds={};

	  std::size_t M=0, d=0;
	  while(M<N-5)
	    {
	      std::size_t i0 = d;
	      std::size_t i1 = chars.size()-1-d;

	      if(M+chars.at(i0).first>=N-5)
		{		  
		  break;
		}
	      
	      inds.insert(i0);	      
	      M += chars.at(i0).first;

	      if(M+chars.at(i1).first>=N-5)
		{
		  break;
		}
	      
	      inds.insert(i1);
	      M += chars.at(i1).first;

	      d += 1;
	    }
	 	  
	  std::stringstream ss;

	  std::size_t prev=-1, mONE=-1;
	  for(auto ind:inds)
	    {
	      if(prev!=mONE and ind>prev+1)
		{
		  ss << " ... ";
		}	      
	      ss << chars.at(ind).second;
	      
	      prev=ind;
	    }

	  result = ss.str();
	}

      dst = utf8::distance(result.c_str(), result.c_str()+result.size());
      
      if(dst<N)
	{
	  std::stringstream ss;
	  ss << std::setw(N+(result.size()-dst)) << result;

	  result = ss.str();
	}

      //dst = utf8::distance(result.c_str(), result.c_str()+result.size());
      //LOG_S(INFO) << N << "\t" << result.size() << "\t" << dst << "\t'" << result << "'";
      
      return result;
    }
    
    std::vector<std::string> split(const std::string& s, char seperator)
    {
      std::vector<std::string> output;

      std::string::size_type prev_pos = 0, pos = 0;
      while((pos = s.find(seperator, pos)) != std::string::npos)
        {
          std::string substring(s.substr(prev_pos, pos-prev_pos) );

          output.push_back(substring);

          prev_pos = ++pos;
        }

      output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

      return output;
    }

    std::vector<std::string> split(const std::string& s, std::string seperator)
    {
      std::vector<std::string> output;

      std::string::size_type prev_pos = 0, pos = 0;
      while((pos = s.find(seperator, pos)) != std::string::npos)
        {
          std::string substring( s.substr(prev_pos, pos-prev_pos) );

          output.push_back(substring);

          pos += seperator.size();
          prev_pos = pos;
        }

      output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

      return output;
    }

    void from_string(const std::string& orig, std::set<std::string>& coll, char sep=';')
    {
      coll.clear();

      if(orig!="")
	{
	  std::vector<std::string> parts = split(orig, sep);

	  for(auto part:parts)
	    {	  
	      coll.insert(part);
	    }
	}
    }

    template<typename index_type>
    bool mask(std::string& text, std::array<index_type, 2> rng)
    {
      assert(rng[0]<=rng[1]);
      assert(rng[1]<=text.size());
      
      for(std::size_t l=rng[0]; l<rng[1]; l++)
	{
	  text[l] = ' ';
	}

      return true;
    }

    int count(const std::string text, char c)
    {
      int result = 0;

      for(char _:text)
	{
	  result += _==c? 1:0;
	}

      return result;
    }
    
    int count_imbalance(const std::string text, char l, char r)
    {
      int result = 0;

      for(char c:text)
	{
	  result += (c==l)? 1:0;
	  result -= (c==r)? 1:0;

	  // make sure 
	  if(result<0)
	    {
	      break;
	    }
	}
      
      return result;
    }

    std::size_t index_of(std::string header, const std::vector<std::string>& headers)
    {
      auto itr = std::find(headers.begin(), headers.end(), header);
      
      if(itr==headers.end())
	{
	  return -1;
	}

      return (itr-headers.begin());
    }
    
  }

}

#endif
