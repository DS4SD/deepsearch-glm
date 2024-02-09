//-*-C++-*-

#ifndef ANDROMEDA_UTILS_H_
#define ANDROMEDA_UTILS_H_

namespace andromeda
{

  class glm_variables
  {
  public:

    const static inline std::filesystem::path package_name = "deepsearch_glm";
    const static inline std::filesystem::path resources_relative_path = "resources";

  private:
    
    static inline std::filesystem::path ROOT_DIR = ROOT_PATH;
    static inline std::filesystem::path PACKAGE_DIR = ROOT_PATH / package_name;
    static inline std::filesystem::path RESOURCES_DIR = PACKAGE_DIR / resources_relative_path;
    
  public:

    static bool set_resources_dir(std::filesystem::path path);

    static std::filesystem::path get_resources_dir(bool verify=true);

    static std::filesystem::path get_rgx_dir();    
    static std::filesystem::path get_fst_dir();
    static std::filesystem::path get_crf_dir();
  };
    
  bool glm_variables::set_resources_dir(std::filesystem::path path)
  {
    RESOURCES_DIR = path;
    
    if(std::filesystem::exists(RESOURCES_DIR))
      {
	PACKAGE_DIR = RESOURCES_DIR.parent_path();
	ROOT_DIR = PACKAGE_DIR.parent_path();

	//LOG_S(INFO) << "updated resourrces-dir: " << RESOURCES_DIR;
	return true;
      }
    else
      {
	LOG_S(ERROR) << "updated resources-dir to non-existant path: "
		     << path << " at " << __FILE__ << ":" << __LINE__;
	return false;
      }
  }
        
  std::filesystem::path glm_variables::get_resources_dir(bool verify)
  {
    if(verify and (not std::filesystem::exists(RESOURCES_DIR)))
      {
	LOG_S(ERROR) << "resource-directory does not exist: "
		     << RESOURCES_DIR << " at " << __FILE__ << ":" << __LINE__;
      }
    
    return RESOURCES_DIR;
  }

  std::filesystem::path glm_variables::get_rgx_dir()
  {
    auto path = get_resources_dir() / "models" / "rgx";

    if(not std::filesystem::exists(path))
      {
	LOG_S(ERROR) << "non-existent regex-path: "
		     << path << " at " << __FILE__ << ":" << __LINE__;
      }

    return path;
  }
  
  std::filesystem::path glm_variables::get_fst_dir()
  {
    auto path = get_resources_dir() / "models" / "fasttext";

    if(not std::filesystem::exists(path))
      {
	LOG_S(ERROR) << "non-existent fasttext-path: "
		     << path << " at " << __FILE__ << ":" << __LINE__;
      }

    return path;
  }
    
  std::filesystem::path glm_variables::get_crf_dir()
  {
    auto path = RESOURCES_DIR / "models" / "crf";

    if(not std::filesystem::exists(path))
      {
	LOG_S(ERROR) << "non-existent crf-path: "
		     << path << " at " << __FILE__ << ":" << __LINE__;
      }

    return path;
  }
  
}

#endif

#include <andromeda/utils/hash/utils.h>
#include <andromeda/utils/time/utils.h>

#include <andromeda/utils/table/utils.h>
#include <andromeda/utils/string/utils.h>

#include <andromeda/utils/regex/pcre2_item.h>
#include <andromeda/utils/regex/pcre2_expr.h>

//#include <andromeda/utils/normalisation/char_token.h>
//#include <andromeda/utils/normalisation/char_normalisation.h>
//#include <andromeda/utils/normalisation/text_normalisation.h>

#include <andromeda/utils/normalisation/text_tokenizer.h>

#include <andromeda/utils/interactive.h>
