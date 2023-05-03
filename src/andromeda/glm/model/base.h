//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_BASE_TYPES_H_
#define ANDROMEDA_MODELS_GLM_BASE_TYPES_H_

namespace andromeda
{
  namespace glm
  {
    struct base_types
    {
      typedef uint64_t                index_type; // coming from NLP
      typedef std::array<uint64_t, 2> range_type; // coming from NLP
      
      typedef  int16_t flvr_type;
      typedef uint64_t hash_type;

      typedef float    val_type;
      typedef uint32_t cnt_type;
      typedef uint64_t ind_type;
      
      typedef float  sval_type;
      typedef double dval_type;      

      // FIXME: remove these types at some point ...
      //typedef flvr_type flavor_type;
      //typedef sval_type value_type;

    };

  }

}

#endif
