//-*-C++-*-

#ifndef ANDROMEDA_UTILS_H_
#define ANDROMEDA_UTILS_H_

namespace andromeda
{

  class glm_variables
  {
  private:
    
#ifdef ROOT_PATH
  static inline std::filesystem::path ROOT_DIR = ROOT_PATH;  
  static inline std::filesystem::path RESOURCES_DIR = ROOT_DIR / "deepsearch_glm/resources";
#else
  static inline std::filesystem::path ROOT_DIR = std::filesystem::current_path() / "..";      
  static inline std::filesystem::path RESOURCES_DIR = std::filesystem::current_path() / "../deepsearch_glm/resources";  
#endif
    
  public:

    static bool set_resources_dir(std::filesystem::path path)
    {
      RESOURCES_DIR = path;

      if(std::filesystem::exists(RESOURCES_DIR))
	{
	  LOG_S(WARNING) << "updated resources-dir: " << path;
	  return true;
	}
      else
	{
	  LOG_S(FATAL) << "updated resources-dir to non-existant path: "
		       << path;
	  return false;
	}
    }
        
    static std::filesystem::path get_resources_dir()
    {
      if(not std::filesystem::exists(RESOURCES_DIR))
	{
	  LOG_S(FATAL) << "resource-directory does not exist: "
		       << RESOURCES_DIR;
	}
      
      return RESOURCES_DIR;
    }
    
    static std::filesystem::path get_fasttext_dir()
    {
      return get_resources_dir() / "models/fasttext";
    }
    
    static std::filesystem::path get_crf_dir()
    {
      return get_resources_dir() / "models/crf";
    }
    
  };
  
}

#endif

#include <andromeda/utils/hash/utils.h>
#include <andromeda/utils/time/utils.h>

#include <andromeda/utils/table/utils.h>
#include <andromeda/utils/string/utils.h>

#include <andromeda/utils/regex/pcre2_item.h>
#include <andromeda/utils/regex/pcre2_expr.h>

#include <andromeda/utils/normalisation/char_token.h>
#include <andromeda/utils/normalisation/char_normalisation.h>
#include <andromeda/utils/normalisation/text_normalisation.h>

#include <andromeda/utils/interactive.h>
