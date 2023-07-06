//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_BASE_LOG_H
#define PYBIND_ANDROMEDA_BASE_LOG_H

namespace andromeda_py
{
  
  class base_log
  {
  public:

    base_log();
  };
  
  base_log::base_log()
  {
    LOG_SCOPE_FUNCTION(WARNING);
  }

}

#endif
