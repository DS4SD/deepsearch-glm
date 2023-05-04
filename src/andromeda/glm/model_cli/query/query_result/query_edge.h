//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_EDGE_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_EDGE_H_

namespace andromeda
{  
  namespace glm
  {   
    class query_edge: public base_types
    {
    public:

      query_edge(hash_type hash);
      
      query_edge(hash_type hash,
		 val_type weight);

    public:

      hash_type hash;
      val_type weight, prob, cumul;
    };

    query_edge::query_edge(hash_type hash):
      hash(hash),
	
      weight(0.0),
      prob(0.0),
      cumul(0.0)
    {}
    
    query_edge::query_edge(hash_type hash,
			   val_type weight):
      hash(hash),

      weight(weight),
      prob(0.0),
      cumul(0.0)
    {}
    
  }

}

#endif
