
message(STATUS "entering in 'os_opts.cmake'")

if(WIN32)
   message(STATUS "compiling on windows")
   
   set(LIB_LINK)
   set(OS_DEPENDENCIES)

elseif(APPLE)
   message(STATUS "compiling on mac-osx")

   find_library(FoundationLib Foundation)
   find_package(ZLIB)
   find_library(SystemConfigurationLib SystemConfiguration)
   set(OS_DEPENDENCIES ZLIB::ZLIB)
   set(LIB_LINK ${FoundationLib} ${SystemConfigurationLib})
   
elseif(UNIX)
   message(STATUS "compiling on linux")

   set(LIB_LINK dl m pthread rt resolv z)
   set(OS_DEPENDENCIES)

endif()
