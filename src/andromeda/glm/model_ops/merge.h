//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODELOPS_MERGE_H_
#define ANDROMEDA_MODELS_GLM_MODELOPS_MERGE_H_

namespace andromeda
{
  namespace glm
  {

    template<>
    class model_op<MERGE>
    {
      typedef andromeda::glm::model model_type;
      
    public:

      model_op();
      
      void merge(std::shared_ptr<model_type> current,
		 std::shared_ptr<model_type> other);

    private:
      
      void merge_nodes(std::shared_ptr<model_type> current,
		       std::shared_ptr<model_type> other);

      void merge_edges(std::shared_ptr<model_type> current,
		       std::shared_ptr<model_type> other);
      
    private:

      bool check_size;
    };

    model_op<MERGE>::model_op():
      check_size(false)
    {}

    void model_op<MERGE>::merge(std::shared_ptr<model_type> current,
				std::shared_ptr<model_type> other)
    {
      merge_nodes(current, other);

      merge_edges(current, other);
    }

    void model_op<MERGE>::merge_nodes(std::shared_ptr<model_type> current,
				      std::shared_ptr<model_type> other)
    {
      auto& curr_nodes = current->get_nodes();
      auto& other_nodes = other->get_nodes();
      
      for(auto itr=other_nodes.begin(); itr!=other_nodes.end(); itr++)
	{
	  auto& flvr_coll = itr->second;
	  for(auto& node:flvr_coll)
	    {
	      curr_nodes.insert(node, check_size);
	    }
	}
    }

    void model_op<MERGE>::merge_edges(std::shared_ptr<model_type> current,
				      std::shared_ptr<model_type> other)
    {
      auto& curr_edges = current->get_edges();
      auto& other_edges = other->get_edges();
      
      for(auto itr=other_edges.begin(); itr!=other_edges.end(); itr++)
	{
	  auto& flvr_coll = itr->second;
	  for(auto& edge:flvr_coll)
	    {
	      curr_edges.insert(edge, check_size);
	    }	  
	}
    }
    
  }

}

#endif
