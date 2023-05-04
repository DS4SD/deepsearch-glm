//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_NODE_H_
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_NODE_H_

namespace andromeda
{  
  namespace glm
  {   
    class query_node: public base_types
    {
    public:

      query_node(hash_type hash);
      
      query_node(hash_type hash,
		 cnt_type count,
		 val_type weight);

    public:

      hash_type hash;

      cnt_type count;
      val_type weight, prob, cumul;
    };

    query_node::query_node(hash_type hash):
      hash(hash),

      count(0),
      weight(0.0),

      prob(0.0),
      cumul(0.0)
    {}
    
    query_node::query_node(hash_type hash,
			   cnt_type count,
			   val_type weight):
      hash(hash),

      count(count),
      weight(weight),

      prob(0.0),
      cumul(0.0)
    {}
    
  }

}

#endif
