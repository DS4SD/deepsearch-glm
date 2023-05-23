//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_BASE_TYPES_H
#define ANDROMEDA_SUBJECTS_BASE_TYPES_H

namespace andromeda
{
  struct base_types
  {
    typedef uint64_t                  index_type;
    typedef std::array<index_type, 2> range_type;
    
    typedef  int16_t flvr_type;
    typedef uint64_t hash_type;
    
    typedef float    val_type;
    typedef uint32_t cnt_type;
    typedef uint64_t ind_type;
    
    typedef float  sval_type;
    typedef double dval_type;      
  };

}

#endif
