//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_BASE_TYPES_H
#define ANDROMEDA_SUBJECTS_BASE_TYPES_H

namespace andromeda
{
  struct base_types
  {
  public:
    
    typedef uint64_t                  index_type;
    typedef std::array<index_type, 2> range_type;

    typedef uint64_t                        table_index_type;
    typedef std::array<table_index_type, 2> table_coor_type;
    typedef std::array<table_index_type, 2> table_range_type;
    
    //typedef  int16_t flvr_type;
    typedef uint16_t flvr_type;
    typedef uint64_t hash_type;
    
    typedef float    val_type;
    typedef uint32_t cnt_type;
    typedef uint64_t ind_type;
    //typedef int64_t ind_type;
    
    typedef float  sval_type;
    typedef double dval_type;

    typedef uint32_t char_ind_type;

  public:
    
    const static inline flvr_type UNDEF_FLVR = std::numeric_limits<flvr_type>::max();
    const static inline hash_type UNDEF_HASH = -1;
    
    const static inline index_type DEFAULT_INDEX = -1;
  };

}

#endif
