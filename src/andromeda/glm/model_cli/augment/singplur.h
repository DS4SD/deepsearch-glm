//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_SINGPLUR_AUGMENTER_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_SINGPLUR_AUGMENTER_H_

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class singplur_augmenter: public base_types
    {
    private:

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef typename nodes_type::node_type node_type;
      typedef typename edges_type::edge_type edge_type;

    public:

      singplur_augmenter(std::shared_ptr<model_type> model);

      void augment(std::size_t max_len);

    private:
      
      void add_singplur(std::size_t max_len);
      
    private:

      std::shared_ptr<model_type> model_ptr;
    };

    template<typename model_type>
    singplur_augmenter<model_type>::singplur_augmenter(std::shared_ptr<model_type> model_ptr):
      model_ptr(model_ptr)
    {}

    template<typename model_type>
    void singplur_augmenter<model_type>::augment(std::size_t max_len)
    {
      add_singplur(max_len);
    }

    template<typename model_type>
    void singplur_augmenter<model_type>::add_singplur(std::size_t max_len)
    {
      LOG_S(INFO) << "keeping terms up to length: " << max_len;
      
      std::map<std::string, std::string> endings = {{"sses", "ss"},
                                                    {"ies", "y"},
						    {"us", "i"},
                                                    {"s", ""}};


      auto& nodes = model_ptr->get_nodes();
      auto& edges = model_ptr->get_edges();
      
      std::vector<node_type> new_nodes={};
      std::vector<edge_type> new_edges={};
      
      std::vector<hash_type> tokens_i={}, tokens_j={};
      for(auto& term_i:nodes.at(node_names::TERM))
	{
	  std::size_t len_i = term_i.get_token_path(nodes, tokens_i);
	  
	  if(len_i>max_len or len_i==0)
	    {
	      if(len_i==0)
		{
		  LOG_S(WARNING) << "term: " << " -> " << len_i;
		}
	      continue;
	    }

	  std::string text_i = term_i.get_token_text(nodes, tokens_i);
	  //LOG_S(INFO) << "term text: " << text_i;
	  
	  std::string root_text_i = nodes.get(tokens_i.back()).get_text();
	  //LOG_S(INFO) << "root token: " << root_text_i;
	  
	  for(auto itr=endings.begin(); itr!=endings.end(); itr++)
	    {
	      std::string ending = itr->first;
	      if(root_text_i.ends_with(ending))
		{
		  std::string root_text_j = root_text_i.substr(0, root_text_i.size()-ending.size());
		  root_text_j += itr->second;

		  node_type token_j(node_names::WORD_TOKEN, root_text_j);
		  if(not nodes.has(token_j.get_hash()))
		    {
		      //LOG_S(WARNING) << " --> missing root token: " << root_text_i << " => " << root_text_j;    
		      //new_nodes.push_back(token_j);
		      continue;
		    }
		  //LOG_S(INFO) << " --> root token: " << root_text_i << " => " << root_text_j;    
		  
		  tokens_j = tokens_i;
		  tokens_j.back() = token_j.get_hash();
		  
		  node_type term_j(node_names::TERM, tokens_j);
		  if(not nodes.has(term_j.get_hash()))
		    {
		      new_nodes.push_back(term_j);

		      if(tokens_j.size()==1)
			{
			  edges.insert(edge_names::from_token, tokens_j.at(0), term_j.get_hash(), false);
			  edges.insert(edge_names::to_token, term_j.get_hash(), tokens_j.at(0), false);	  
			}
		    }
		  
		  edges.insert(edge_names::to_singular, term_j.get_hash(), term_j.get_hash(),
			       1, false);
		  edges.insert(edge_names::to_plural  , term_i.get_hash(), term_i.get_hash(),
			       1, false);
		  
		  edges.insert(edge_names::to_singular, term_i.get_hash(), term_j.get_hash(),
			       1, false);
		  edges.insert(edge_names::to_plural  , term_j.get_hash(), term_i.get_hash(),
			       1, false);

		  break;
		}
	    }
	}

      LOG_S(INFO) << __FUNCTION__ << " --> new nodes: " << new_nodes.size();
      for(auto& new_node:new_nodes)
	{
	  nodes.insert(new_node, false);
	}

      {
	//nodes.sort(node_names::TOKEN);
	nodes.sort(node_names::TERM);	

	edges.sort(edge_names::to_singular);
	edges.sort(edge_names::to_plural);
      }      
    }

  }

}

#endif
