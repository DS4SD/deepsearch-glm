//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_AUGMENTER_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_AUGMENTER_H_

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class model_augmenter
    {
    private:

      typedef typename model_type::index_type index_type;

      typedef typename model_type::flvr_type flvr_type;
      typedef typename model_type::hash_type hash_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef typename nodes_type::node_type node_type;
      typedef typename edges_type::edge_type edge_type;

    public:

      model_augmenter(std::shared_ptr<model_type> model);

      void augment();

    private:
      
      void add_singplur();

      void add_taxupdn();

    private:

      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    model_augmenter<model_type>::model_augmenter(std::shared_ptr<model_type> model):
      model(model)
    {}

    template<typename model_type>
    void model_augmenter<model_type>::augment()
    {
      add_singplur();

      add_taxupdn();

      model->finalise(true);
    }

    template<typename model_type>
    void model_augmenter<model_type>::add_singplur()
    {
      LOG_S(INFO) << __FUNCTION__;

      model->finalise(false);

      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();

      std::map<std::string, std::string> endings = {{"sses", "ss"},
                                                    {"ies", "y"},
                                                    {"s", ""}};


      for(auto& node_i:nodes.at(node_names::TOKEN))
	{
	  std::string text_i = node_i.get_text();

	  for(auto itr=endings.begin(); itr!=endings.end(); itr++)
	    {
	      std::string ending = itr->first;
	      if(text_i.ends_with(ending))
		{
		  std::string text_j = text_i.substr(0, text_i.size()-ending.size());
		  text_j += itr->second;
		  
		  node_type node_j(node_names::TOKEN, text_j);
		  if(nodes.get(node_j.get_hash(), node_j))
		    {
		      // self edges		      
		      edges.insert(edge_names::to_singular, node_j.get_hash(), node_j.get_hash(),
				   1, false);
		      edges.insert(edge_names::to_plural  , node_i.get_hash(), node_i.get_hash(),
				   1, false);

		      
		      edges.insert(edge_names::to_singular, node_i.get_hash(), node_j.get_hash(),
				   1, false);
		      edges.insert(edge_names::to_plural  , node_j.get_hash(), node_i.get_hash(),
				   1, false);
		    }
		}
	    }
	}

      LOG_S(INFO) << __FUNCTION__;
      edges.sort(edge_names::to_singular);

      LOG_S(INFO) << __FUNCTION__;
      edges.sort(edge_names::to_plural);

      std::vector<edge_type> new_edges={};

      LOG_S(INFO) << __FUNCTION__;
      for(auto& term_i:nodes.at(node_names::TERM))
	{
	  std::vector<hash_type> nodes_i = term_i.get_nodes();

	  if(nodes_i.size()==0)
	    {
	      LOG_S(WARNING) << "no nodes for term: " << term_i.get_hash();
	      continue;	      
	    }

	  if(not nodes.has(nodes_i.back()))
	    {
	      LOG_S(WARNING) << "no nodes known for: " << nodes_i.back();
	      continue;	      
	    }
	  
	  if(nodes.get_flvr(nodes_i.back())!=node_names::TOKEN)
	    {
	      continue;
	    }
	  
	  std::vector<edge_type> term_edges={};
	  edges.traverse(edge_names::to_singular, nodes_i.back(), term_edges, true);

	  for(auto& term_edge:term_edges)
	    {
	      if(not nodes.has(term_edge.get_hash_j()))
		{
		  LOG_S(WARNING) << "no nodes known for: " << term_edge.get_hash_j();
		  continue;	      
		}

	      if(not nodes.has(term_edge.get_hash_j()))
		{
		  LOG_S(WARNING) << "no nodes known for: " << term_edge.get_hash_j();
		  continue;	      
		}
	      
	      if(nodes.get_flvr(term_edge.get_hash_j())!=node_names::TOKEN)
		{
		  continue;
		}
	      
	      std::vector<hash_type> nodes_j = nodes_i;
	      nodes_j.back() = term_edge.get_hash_j();

	      /*
	      node_type term_j(node_names::TERM, nodes_j);	      
	      if(nodes.get(term_j.get_hash(), term_j))
		{
		}
	      else 
		{
		  nodes.insert(term_j, false);		  
		}
	      */

	      node_type term_j = nodes.insert(node_names::TERM, nodes_j);
	      
	      new_edges.emplace_back(edge_names::to_singular, term_j.get_hash(), term_j.get_hash(), 1);
	      new_edges.emplace_back(edge_names::to_plural  , term_i.get_hash(), term_i.get_hash(), 1);
	      
	      new_edges.emplace_back(edge_names::to_singular, term_i.get_hash(), term_j.get_hash(), 1);
	      new_edges.emplace_back(edge_names::to_plural  , term_j.get_hash(), term_i.get_hash(), 1);
	    }
	}

      LOG_S(INFO) << __FUNCTION__;
      for(auto& new_edge:new_edges)
	{
	  edges.insert(new_edge, false);
	}

      LOG_S(INFO) << __FUNCTION__;
      edges.sort(edge_names::to_singular);

      LOG_S(INFO) << __FUNCTION__;
      edges.sort(edge_names::to_plural);      
    }
    
    template<typename model_type>
    void model_augmenter<model_type>::add_taxupdn()
    {
      LOG_S(INFO) << __FUNCTION__;

      glm_nodes& nodes = model->get_nodes();
      glm_edges& edges = model->get_edges();

      auto& terms = nodes.at(node_names::TERM);
      
      for(auto& term_i:terms)
        {
          std::vector<hash_type> term_hashes_i = term_i.get_nodes();

	  if(term_hashes_i.size()>1)
	    {
	      std::vector<hash_type> root_hashes = {term_hashes_i.back()};
	      base_node root(node_names::TERM, root_hashes);

	      if(nodes.has(root.get_hash()))
		{
		  edges.insert(edge_names::from_root_to_path, root.get_hash(), term_i.get_hash(),
			       term_i.count(), false);
		  edges.insert(edge_names::from_path_to_root, term_i.get_hash(), root.get_hash(),
			       term_i.count(), false);
		}
	    }		  
				       
          for(std::size_t i=0; i<term_hashes_i.size(); i++)
            {
              std::vector<hash_type> term_hashes_j={};
              for(std::size_t j=i+1; j<term_hashes_i.size(); j++)
                {
                  term_hashes_j.push_back(term_hashes_i.at(j));
                }

              if(term_hashes_j.size()==0) // skip
                {
                  continue;
                }

              base_node term_j(node_names::TERM, term_hashes_j);
              if(nodes.get(term_j.get_hash(), term_j))
                {
                  edges.insert(edge_names::tax_dn, term_i.get_hash(), term_j.get_hash(), term_i.count(), false);
                  edges.insert(edge_names::tax_up, term_j.get_hash(), term_i.get_hash(), term_i.count(), false);

                  break;
                }
              else
                {}
            }
        }
    }

    /*
      template<typename model_type>
      void model_augmenter<model_type>::add_related_to_relation()
      {
      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();
      auto& paths = model->get_paths();

      for(std::size_t i=0; i<paths.size(); i++)
      {
      path_type& path_i = paths.at(i);

      if(path_i.flavor!=path_names::TERM and path_i.count>=2)
      {
      continue;
      }

      std::string pos = path_i.to_pos(nodes);

      if(pos!="NN CC NN" and
      pos!="NNS CC NNS")
      {
      continue;
      }

      LOG_S(INFO) << "related: " << path_i.to_text(nodes);

      hash_type hash_i = path_i.path.front();
      hash_type hash_j = path_i.path.back();

      edges.insert(edge_names::related, hash_i, hash_j, path_i.count, false);
      edges.insert(edge_names::related, hash_j, hash_i, path_i.count, false);
      }
      }
    */
  }

}

#endif
