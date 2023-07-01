//-*-C++-*-

#ifndef ANDROMEDA_UTILS_H_
#define ANDROMEDA_UTILS_H_

namespace andromeda
{
#ifdef ROOT_PATH
  static inline std::filesystem::path ROOT_DIR = ROOT_PATH;  
  static inline std::filesystem::path RESOURCES_DIR = ROOT_DIR / "resources";
#else
  static inline std::filesystem::path RESOURCES_DIR = std::filesystem::current_path() / "../resources";  
#endif
  
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
