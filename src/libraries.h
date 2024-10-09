//-*-C++-*-

// included libraries
#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif

#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>

#include "pcre2.h"

#ifdef _WIN32
#include <share.h> // to define _SH_DENYNO for loguru
#endif

#ifndef LOGURU_WITH_STREAMS
#define LOGURU_WITH_STREAMS 1
#endif

#include <loguru.hpp>

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
//#include <nlohmann/json-schema.hpp>

#include <utf8.h>
#include <fasttext/fasttext.h>
//#include <sentencepiece.pb.h>
#include <sentencepiece_processor.h>
#include <sentencepiece_trainer.h>

#include <mutex>
