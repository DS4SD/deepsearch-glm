//-*-C++-*-

#ifndef ANDROMEDA_UTILS_TIME_UTILS_H_
#define ANDROMEDA_UTILS_TIME_UTILS_H_

namespace andromeda
{
  namespace utils
  {
    std::string get_current_time()
    {
      std::string format("%Y:%m:%d_%H:%M:%S");

      auto        time = std::chrono::system_clock::now();
      std::time_t tt   = std::chrono::system_clock::to_time_t(time);
      std::tm     tm   = *std::gmtime(&tt); //GMT (UTC)

      std::stringstream ss;
      ss << std::put_time(&tm, format.c_str());

      return ss.str();
    }
    
  }

}

#endif
