message(STATUS "Entering extlib_loguru.cmake")

set(ext_name "loguru")

if(USE_SYSTEM_DEPS)
    find_package(loguru CONFIG REQUIRED)
    add_library(${ext_name} ALIAS loguru::loguru)
else()
    include(FetchContent)
    FetchContent_Declare(LoguruGitRepo
        GIT_REPOSITORY "https://github.com/emilk/loguru"
        GIT_TAG        "4adaa185883e3c04da25913579c451d3c32cfac1" # pin current master, because tag v2.1.0 branch does not have full make support, that was introduced later , this SHA also matches the systen package .rpm https://koji.fedoraproject.org/koji/rpminfo?rpmID=40293153
    )

    set(LOGURU_WITH_STREAMS TRUE)
    set(STACKTRACES TRUE)
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

    if(WIN32)
        # https://digitalmars.com/rtl/constants.html
        # value of _SH_DENYNO is 0x40 = 64 
        add_compile_definitions(_SH_DENYNO=64)
    endif()
    
    FetchContent_MakeAvailable(LoguruGitRepo) # defines target 'loguru::loguru'

endif()
