//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODELOPS_MERGE_H_
#define ANDROMEDA_MODELS_GLM_MODELOPS_MERGE_H_

namespace andromeda
{
  namespace glm
  {

    template<typename model_type>
    class model_op<MERGE, model_type>
    {
    public:

      model_op(std::shared_ptr<model_type> model);
      
      void merge(std::shared_ptr<model_type> other);

    private:
      
      void merge_nodes(std::shared_ptr<model_type> other);

      void merge_edges(std::shared_ptr<model_type> other);
      
    private:

      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    model_op<MERGE, model_type>::model_op(std::shared_ptr<model_type> model):
      model(model)
    {}

    template<typename model_type>
    void model_op<MERGE, model_type>::merge(std::shared_ptr<model_type> other)
    {
      merge_nodes(other);

      merge_edges(other);
    }

    template<typename model_type>
    void model_op<MERGE, model_type>::merge_nodes(std::shared_ptr<model_type> other)
    {
      auto& nodes = model->get_nodes();
      auto& other_nodes = other->get_nodes();
      
      for(auto itr=other_nodes.begin(); itr!=other_nodes.end(); itr++)
	{
	  nodes.insert(*itr);
	}

      //nodes.sort();
    }

    template<typename model_type>
    void model_op<MERGE, model_type>::merge_edges(std::shared_ptr<model_type> other)
    {
      auto& edges = model->get_edges();
      auto& other_edges = other->get_edges();
      
      for(auto itr=other_edges.begin(); itr!=other_edges.end(); itr++)
	{
	  edges.insert(*itr);
	}

      //edges.collapse();
    }
    
  }

}

#endif
