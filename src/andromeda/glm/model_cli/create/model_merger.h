//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_MERGE_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_MERGE_H_

namespace andromeda
{

  namespace glm
  {

    template<typename model_type>
    class model_merger
    {
    public:

      model_merger(std::shared_ptr<model_type> model,
		   bool enforce_max_size);
      
      double merge(std::shared_ptr<model_type> other);

    private:
      
      void merge_nodes(std::shared_ptr<model_type> other);

      void merge_edges(std::shared_ptr<model_type> other);
      
    private:

      std::shared_ptr<model_type> model;

      bool enforce_max_size;
    };

    template<typename model_type>
    model_merger<model_type>::model_merger(std::shared_ptr<model_type> model,
					   bool enforce_max_size):
      model(model),
      enforce_max_size(enforce_max_size)
    {}

    template<typename model_type>
    double model_merger<model_type>::merge(std::shared_ptr<model_type> other)
    {
      auto start = std::chrono::system_clock::now();
      
      merge_nodes(other);

      merge_edges(other);

      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double, std::milli> delta = (end-start);

      return delta.count();
    }

    template<typename model_type>
    void model_merger<model_type>::merge_nodes(std::shared_ptr<model_type> other)
    {
      auto& these_nodes = model->get_nodes();
      auto& other_nodes = other->get_nodes();
      
      for(auto itr=other_nodes.begin(); itr!=other_nodes.end(); itr++)
	{
	  for(auto& node:itr->second)
	    {
	      these_nodes.insert(node, enforce_max_size);
	    }
	}
    }

    template<typename model_type>
    void model_merger<model_type>::merge_edges(std::shared_ptr<model_type> other)
    {
      auto& these_edges = model->get_edges();
      auto& other_edges = other->get_edges();

      for(auto flvr_itr=other_edges.begin(); flvr_itr!=other_edges.end(); flvr_itr++)      
	{
	  auto& edge_coll = flvr_itr->second;

	  for(auto edge_itr=edge_coll.begin(); edge_itr!=edge_coll.end(); edge_itr++)
	    {
	      these_edges.insert(*edge_itr, enforce_max_size);
	    }
	}	  
    }

  }

}

#endif
