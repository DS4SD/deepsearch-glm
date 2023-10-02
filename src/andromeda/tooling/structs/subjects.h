//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_SUBJECTS_H_
#define ANDROMEDA_STRUCTS_SUBJECTS_H_

namespace andromeda
{
  template<subject_name name>
  class subject
  {};
}

#endif

#include <andromeda/tooling/structs/subjects/base.h>

#include <andromeda/tooling/structs/subjects/text.h>
#include <andromeda/tooling/structs/subjects/table.h>
#include <andromeda/tooling/structs/subjects/figure.h>

#include <andromeda/tooling/structs/subjects/document/doc_order.h>
#include <andromeda/tooling/structs/subjects/document/doc_captions.h>
#include <andromeda/tooling/structs/subjects/document/doc_maintext.h>
#include <andromeda/tooling/structs/subjects/document/doc_normalisation.h>

#include <andromeda/tooling/structs/subjects/document.h>


