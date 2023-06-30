//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_TAXTREE_AUGMENTER_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_TAXTREE_AUGMENTER_H_

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class taxtree_augmenter: public base_types
    {
    private:

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef typename nodes_type::node_type node_type;
      typedef typename edges_type::edge_type edge_type;

    public:

      taxtree_augmenter(std::shared_ptr<model_type> model);

      void augment();

    private:

      void add_taxupdn();

      void add_related();

    private:

      std::shared_ptr<model_type> model_ptr;
    };

    template<typename model_type>
    taxtree_augmenter<model_type>::taxtree_augmenter(std::shared_ptr<model_type> model_ptr):
      model_ptr(model_ptr)
    {}

    template<typename model_type>
    void taxtree_augmenter<model_type>::augment()
    {
      add_taxupdn();
    }

    template<typename model_type>
    void taxtree_augmenter<model_type>::add_taxupdn()
    {
      LOG_S(INFO) << __FUNCTION__;

      auto& nodes = model_ptr->get_nodes();
      auto& edges = model_ptr->get_edges();
      
      for(auto& term_i:nodes.at(node_names::TERM))
        {
	  auto cnt = term_i.count();
          auto term_hashes_i = term_i.get_nodes();

          if(term_hashes_i.size()>1)
            {
              std::vector<hash_type> root_hashes = {term_hashes_i.back()};
              base_node root(node_names::TERM, root_hashes);
	      
              if(nodes.has(root.get_hash()))
                {
                  edges.insert(edge_names::from_root, root.get_hash(), term_i.get_hash(), cnt, false);
                  edges.insert(edge_names::to_root, term_i.get_hash(), root.get_hash(), cnt, false);
                }

              for(std::size_t i=0; i<term_hashes_i.size()-1; i++)
                {
                  edges.insert(edge_names::tax_dn, term_hashes_i.at(i), term_hashes_i.at(i+1), cnt, false);
		  edges.insert(edge_names::to_root, term_hashes_i.at(i), root.get_hash(), cnt, false);
                }

              for(std::size_t i=1; i<term_hashes_i.size(); i++)
                {
                  edges.insert(edge_names::tax_up, term_hashes_i.at(i), term_hashes_i.at(i-1), cnt, false);
		  edges.insert(edge_names::from_root, root.get_hash(), term_hashes_i.at(i), cnt, false);
                }
	      	      
	      edges.insert(edge_names::from_root, root.get_hash(), term_i.get_hash(), cnt, false);	      
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
                  edges.insert(edge_names::tax_dn, term_i.get_hash(), term_j.get_hash(), cnt, false);
                  edges.insert(edge_names::tax_up, term_j.get_hash(), term_i.get_hash(), cnt, false);

                  break;
                }
              else
                {}
            }
        }
    }

    template<typename model_type>
    void taxtree_augmenter<model_type>::add_related()
    {
      /*
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
      */
    }

  }

}

#endif
