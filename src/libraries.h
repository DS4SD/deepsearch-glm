//-*-C++-*-

// included libraries
#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif

#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>

#include "pcre2.h"

#ifndef LOGURU_WITH_STREAMS
#define LOGURU_WITH_STREAMS 1
#endif

#include <loguru/loguru.cpp>

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
//#include <nlohmann/json-schema.hpp>

#include <utf8/utf8.h>
#include <fasttext/fasttext.h>
