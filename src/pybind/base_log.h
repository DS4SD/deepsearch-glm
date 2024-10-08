//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_BASE_LOG_H
#define PYBIND_ANDROMEDA_BASE_LOG_H

namespace andromeda_py
{
  
  class base_log
  {
  public:

    base_log();

    bool set_loglevel(std::string level);
  };
  
  base_log::base_log()
  {
    loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
    
    /*
    std::string symbol="-v";
    std::string verbosity="WARNING";

    char* c0 = &symbol.at(0);//static_cast<char*>(symbol.c_str());
    char* c1 = &verbosity.at(0);//static_cast<char*>(verbosity.c_str());
    
    int argc=2;
    char* argv[] = {c0, c1};
    
    loguru::init(argc, argv);
    */
  }

  bool base_log::set_loglevel(std::string level)
  {
    if(level=="INFO")
      {
	loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
      }
    else if(level=="WARNING")
      {
	loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
      }
    else if(level=="ERROR")
      {
	loguru::g_stderr_verbosity = loguru::Verbosity_ERROR;
      }
    else
      {
	loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
	return false;
      }

    return true;
  }
  

  
}

#endif
