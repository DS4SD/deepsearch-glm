//-*-C++-*-

// included libraries
#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.cpp>

#include <cxxopts.hpp>

#include <nlohmann/json.hpp>
//#include <nlohmann/json-schema.hpp>

#include <utf8/utf8.h>

#include <fasttext/fasttext.h>

//#include <pos/crf.h>
//#include <pos/lapos.h>

// andromeda

#include <andromeda/utils.h>

#include <andromeda/enums.h>

#include <andromeda/tooling.h>

#include <andromeda/nlp.h>
#include <andromeda/glm.h>

#ifdef INCLUDE_ANDROMEDA_PYBIND
#include <andromeda/pyb.h>
#endif
