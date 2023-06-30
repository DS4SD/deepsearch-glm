//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_BASE_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_BASE_H_

namespace andromeda
{
  namespace glm
  {

    template<typename model_type>
    class base_explorer
    {
    public:
      
      typedef typename model_type::index_type index_type;

      typedef typename model_type::node_type glm_node_type;
      typedef typename model_type::edge_type glm_edge_type;
      
      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

      typedef query_node qry_node_type;

      typedef                 query_flow<model_type>     qry_flow_type;
      typedef std::shared_ptr<query_result<model_type> > qry_result_type;
      
      typedef typename qry_flow_type::flow_id_type flow_id_type;
      
    public:

      base_explorer(std::shared_ptr<model_type> model);
      ~base_explorer();

      std::string to_string(std::vector<qry_node_type>& nodes);

      void filter(short flavor, std::vector<qry_node_type>& qry_nodes);
      void filter(short flavor, std::string pos,
		  std::vector<qry_node_type>& qry_nodes);

      void reweight_by_text_count(std::vector<qry_node_type>& qry_nodes);
      
      bool to_node(std::string& word, glm_node_type& node_j);

      bool to_nodes(std::string& word, std::vector<qry_node_type>& nodes);
      bool to_nodes(std::string& word, std::vector<glm_node_type>& nodes);

      bool to_nodes(short edge_type, index_type hash, std::vector<qry_node_type>& nodes);

      bool to_concepts(std::string& word, std::vector<qry_node_type>& nodes);

      bool to_concepts(std::vector<std::string>& words,
		       std::vector<qry_node_type>& query_nodes);
      
    private:
      
      bool to_nodes(glm_node_type& node_i, qry_result_type& result);

      bool to_nodes(short edge_type, index_type hash, qry_result_type& nodes);
      bool to_nodes(short edge_type, glm_node_type& node, qry_result_type& nodes);
      
    protected:

      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    base_explorer<model_type>::base_explorer(std::shared_ptr<model_type> model):
      model(model)
    {}

    template<typename model_type>
    base_explorer<model_type>::~base_explorer()
    {}

    template<typename model_type>
    std::string base_explorer<model_type>::to_string(std::vector<qry_node_type>& qry_nodes)
    {
      auto& model_nodes = model->get_nodes();
      
      std::vector<std::string> header={"hash", "flavor",					     
				       "weight", "prob", "cum",
				       "pos", "word",
				       "word-count", "sent-count", "text-count"};
      
      std::vector<std::vector<std::string> > data={};
      for(auto& qry_node:qry_nodes)
	{
	  glm_node_type node;
	  
	  if(model_nodes.get(qry_node.hash, node)) 
	    {
	      std::vector<std::string> row={std::to_string(node.hash),
					    node_names::to_name(node.flavor),
					    
					    std::to_string(qry_node.weight),
					    std::to_string(qry_node.prob),
					    std::to_string(qry_node.cumul), 		  
					    
					    node.pos,
					    node.word,					      
					    
					    std::to_string(node.word_cnt),
					    std::to_string(node.sent_cnt),
					    std::to_string(node.text_cnt)};
	      data.push_back(row);
	    }

	  if(data.size()>=32)
	    {
	      break;
	    }
	}

      return utils::to_string(header, data);
    }

    template<typename model_type>
    void base_explorer<model_type>::filter(short flavor, std::vector<qry_node_type>& qry_nodes)
    {
      auto& nodes = model->get_nodes();
      
      auto itr = qry_nodes.begin();
      while(itr!=qry_nodes.end())
	{
	  auto& glm_node = nodes.get(itr->hash);
	  
	  if(glm_node.flavor==flavor)
	    {
	      itr++;
	    }
	  else
	    {
	      itr = qry_nodes.erase(itr);
	    }
	}
    }

    template<typename model_type>
    void base_explorer<model_type>::filter(short flavor, std::string pos,
					   std::vector<qry_node_type>& qry_nodes)
    {
      auto& nodes = model->get_nodes();
      
      auto itr = qry_nodes.begin();
      while(itr!=qry_nodes.end())
	{
	  auto& glm_node = nodes.get(itr->hash);
	  
	  if(glm_node.flavor==flavor and glm_node.pos.starts_with(pos))
	    {
	      itr++;
	    }
	  else
	    {
	      itr = qry_nodes.erase(itr);
	    }
	}
    }    

    template<typename model_type>
    void base_explorer<model_type>::reweight_by_text_count(std::vector<qry_node_type>& qry_nodes)
    {
      auto& nodes = model->get_nodes();
      auto& paths = model->get_paths();

      glm_node_type glm_node;
      //glm_path_type glm_path;
      
      std::size_t total=0;
      for(auto& qry_node:qry_nodes)
	{
	  if(nodes.get(qry_node.hash, glm_node))
	    {
	      qry_node.weight = glm_node.text_cnt;	  	      
	    }
	  /*
	  else if(paths.get(qry_node.hash, glm_path))
	    {
	      qry_node.weight = glm_path.count;	      
	    }
	  */
	  else
	    {
	      LOG_S(WARNING) << "query-item not found ...";
	    }

	  total += qry_node.weight;
	}

      //double prob=0;
      for(auto& qry_node:qry_nodes)
	{
	  qry_node.prob = double(qry_node.weight)/double(1.e-6+total);
	}

      std::sort(qry_nodes.begin(), qry_nodes.end(),
		[](const qry_node_type& lhs,
		   const qry_node_type& rhs)
		{ return lhs.weight>rhs.weight; });

      double cumul=0;
      for(auto& qry_node:qry_nodes)
	{
	  cumul += qry_node.prob;
      	  qry_node.cumul = cumul;	  
	}
    }
    
    template<typename model_type>
    bool base_explorer<model_type>::to_node(std::string& word, glm_node_type& node_j)
    {
      std::vector<glm_node_type> nodes;
      to_nodes(word, nodes);

      if(nodes.size()>0)
	{
	  node_j = nodes.at(0);
	}
      else
	{
	  return false;	  
	}
      
      return true;
    }

    template<typename model_type>
    bool base_explorer<model_type>::to_nodes(short edge_flavor, index_type hash,
					     qry_result_type& result)
    {
      auto& model_nodes = model->get_nodes();

      glm_node_type node_i;
      if(not model_nodes.get(hash, node_i))
	{
	  return false;
	}
      
      return to_nodes(edge_flavor, node_i, result);
    }

    template<typename model_type>
    bool base_explorer<model_type>::to_nodes(short edge_flavor, glm_node_type& node,
					     qry_result_type& result)
    {
      query_flow<model_type> flow(model);

      auto op_i = flow.add_select(node);
      auto op_j = flow.add_traverse(edge_flavor, op_i->get_id());

      bool success = flow.execute();

      result = op_j->get_nodeset();
      result->normalise();
      
      return success;            
    }

    /*
    template<typename model_type>
    bool base_explorer<model_type>::to_nodes(glm_node_type& node_i, qry_result_type& result)      
    {
      query_flow<model_type> flow(model);

      auto op_i = flow.add_select(node_i);
      auto op_j = flow.add_traverse(edge_names::to_node, op_i->get_id());

      bool success = flow.execute();

      result = op_j->get_nodeset();
      result->normalise();
      
      return success;
    }
    */
    
    template<typename model_type>
    bool base_explorer<model_type>::to_nodes(std::string& word, std::vector<qry_node_type>& query_nodes)
    {
      query_nodes.clear();
      
      auto& nodes = model->get_nodes();
      
      word_token token(word, word_token::DEFAULT_POS);
      auto node_i = nodes.get(token);

      if(node_i.word==node_names::UNKNOWN)
        {
          LOG_S(WARNING) << "word `" << word << "` not found ...";
          return false;
        }
      else if(node_i.word==node_names::MASK)
        {	  
	  query_nodes.emplace_back(node_i.hash, 1, 1);
          return true;
        }
      else
	{
	  qry_result_type result;
      
	  bool success = to_nodes(node_i, result);
	  
	  query_nodes.clear();
	  for(auto itr=result->begin(); itr!=result->end(); itr++)
	    {
	      query_nodes.push_back(*itr);
	    }
      
	  return success;
	}
    }
    
    template<typename model_type>
    bool base_explorer<model_type>::to_nodes(std::string& word, std::vector<glm_node_type>& query_nodes)
    {
      auto& nodes = model->get_nodes();
      
      word_token token(word, word_token::DEFAULT_POS);
      auto node_i = nodes.get(token);

      if(node_i.word==node_names::UNKNOWN)
        {
          LOG_S(WARNING) << "word `" << word << "` not found ...";
	  query_nodes.clear();
          return false;
        }
      else if(node_i.word==node_names::MASK)
        {
	  query_nodes.push_back(node_i);
          return true;
        }
      else
	{
	  qry_result_type result;
      
	  bool success = to_nodes(node_i, result);
	  result->to_nodes(query_nodes);
	  
	  return success;
	}
    }

    template<typename model_type>
    bool base_explorer<model_type>::to_nodes(short edge_flavor, index_type hash,
					     std::vector<qry_node_type>& nodes)
    {
      qry_result_type result;
      
      bool success = to_nodes(edge_flavor, hash, result);

      nodes.clear();

      if(success)
	{
	  for(auto itr=result->begin(); itr!=result->end(); itr++)
	    {
	      nodes.push_back(*itr);
	    }
	}
      
      return success;
    }

    template<typename model_type>
    bool base_explorer<model_type>::to_concepts(std::string& word, std::vector<qry_node_type>& query_nodes)
    {
      query_nodes.clear();
      
      auto& nodes = model->get_nodes();
      
      word_token token(word, word_token::DEFAULT_POS);
      auto node_i = nodes.get(token);

      if(node_i.word==node_names::UNKNOWN)
        {
          LOG_S(WARNING) << "word `" << word << "` not found ...";
          return false;
        }
      else if(node_i.word==node_names::MASK)
        {	  
	  query_nodes.emplace_back(node_i.hash, 1, 1);
          return true;
        }
      else
	{
	  qry_result_type result;

	  query_flow<model_type> flow(model);

	  auto op_0 = flow.add_select(node_i);

	  std::set<short> edge_flavors =
	    {
	     //edge_names::to_word, edge_names::to_node,
	     //edge_names::to_lower, edge_names::to_upper,
	     edge_names::to_singular, edge_names::to_plural,
	     //edge_names::to_concept, edge_names::to_instance
	    };

	  std::set<typename query_flow<model_type>::flow_id_type> flow_ids={};
	  
	  auto op_y = op_0;

	  for(int itr=0; itr<6; itr++)
	    {
	      flow_ids.insert(op_0->get_id());
	      
	      for(auto edge_flavor:edge_flavors)
		{
		  auto op_x = flow.add_traverse(edge_flavor, op_y->get_id());
		  flow_ids.insert(op_x->get_id());
		}
	      
	      op_y = flow.add_join(flow_ids);
	    }
	  
	  bool success = flow.execute();

	  result = op_y->get_nodeset();
	  result->normalise();
	  
	  query_nodes.clear();
	  for(auto itr=result->begin(); itr!=result->end(); itr++)
	    {
	      auto& node = nodes.get(itr->hash);
	      if(not node.pos.starts_with("__"))
		{
		  query_nodes.push_back(*itr);
		}
	    }
      
	  return success;
	}
    }

    /*
    template<typename model_type>
    bool base_explorer<model_type>::to_concepts(std::vector<std::string>& words,
						std::vector<qry_node_type>& query_nodes)
    {
      bool success = false;
      query_nodes.clear();
      
      auto& nodes = model->get_nodes();
      auto& paths = model->get_paths();

      std::vector<index_type> hashes={};
      for(std::string word:words)
	{
	  word_token token(word, word_token::DEFAULT_POS);

	  glm_node_type node;
	  if(nodes.get(token, node))
	    {
	      hashes.push_back(node.hash);
	    }
	  else
	    {
	      LOG_S(WARNING) << "can not find word " << word << " in GLM ...";
	      return false;
	    }
	}
      
      glm_path path(path_names::TERM, hashes);
      if(paths.has(path))
	{
	  std::shared_ptr<query_nodeset<model_type> > result;

	  query_flow<model_type> flow(model);
	  auto op_0 = flow.add_select(path);
	  auto op_1 = flow.add_traverse(edge_names::to_node, op_0->get_id());

	  std::set<flow_id_type> ids_1 = { op_1->get_id()}; 
	  auto op_2 = flow.add_join(ids_1);

	  auto op_3a = flow.add_traverse(edge_names::to_singular, op_2->get_id());
	  auto op_3b = flow.add_traverse(edge_names::to_plural  , op_2->get_id());

	  std::set<flow_id_type> ids_3 = {op_3a->get_id(), op_3b->get_id()}; 
	  auto op_4 = flow.add_join(ids_3);

	  auto op_5 = flow.set_uniform_weight(op_4->get_id());
	  
	  success = flow.execute();

	  result = op_5->get_nodeset();
	  result->normalise();
	  
	  query_nodes.clear();
	  for(auto itr=result->begin(); itr!=result->end(); itr++)
	    {
	      query_nodes.push_back(*itr);
	    }	  
	}
      else
	{
	  LOG_S(WARNING) << "no path found ...";
	}

      return success;
    }
*/

  }

}

#endif
