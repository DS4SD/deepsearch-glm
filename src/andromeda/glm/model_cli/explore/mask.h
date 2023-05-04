//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_MASK_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_MASK_H_

namespace andromeda
{
  namespace glm
  {

    template<typename model_type>
    class text_masker: public base_types
    {
      //typedef typename model_type::index_type index_type;
      
      typedef typename model_type::node_type node_type;
      typedef typename model_type::edge_type edge_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

    public:

      text_masker(std::shared_ptr<model_type> model);
      ~text_masker();

      void interactive();

    private:

      bool get_nodes_and_masks(std::string line,
			       std::vector<node_type>& nodes,
			       std::vector<short>& masks);

      bool to_node(std::string& word, node_type& node);
      
    private:

      std::shared_ptr<model_type> model;
    };

    template<typename model_type>
    text_masker<model_type>::text_masker(std::shared_ptr<model_type> model):
      model(model)
    {}

    template<typename model_type>
    text_masker<model_type>::~text_masker()
    {}

    template<typename model_type>
    void text_masker<model_type>::interactive()
    {
      LOG_S(INFO) << __FUNCTION__;

      auto& edges = model->get_edges();

      std::string line;

      std::vector<node_type> nodes;
      std::vector<flvr_type> masks;
      
      while(true)
        {
          std::cout << "exploring line: ";
          std::getline(std::cin, line);

	  if(not get_nodes_and_masks(line, nodes, masks))
	    {
	      break;
	    }	  

	  /*
	  for(auto& node:nodes)
	    {
	      LOG_S(INFO) << std::setw( 8) << node.pos  << " | "
			  << std::setw(24) << node.word << " |";
	    }
	  */
	  
          std::set<std::size_t> set_i={}, set_k={};

          query_flow<model_type> flow(model);

	  flvr_type len = nodes.size();
          for(flvr_type l=0; l<len; l++)
            {
              for(auto k:masks)
                {
                  flvr_type delta = k-l;

                  if(edges.has(delta))
                    {
                      auto op_i = flow.add_select(nodes.at(l));

                      auto op_j = flow.add_traverse(delta, op_i->get_id());

                      set_i.insert(op_j->get_id());

                      auto op_l = flow.add_traverse(edge_names::to_pos, op_i->get_id());

                      auto op_k = flow.add_traverse(delta, op_l->get_id());
                      op_k->get_nodeset()->set_name("traverse pos");

                      set_k.insert(op_k->get_id());
                    }
                }
            }

          auto op_0 = flow.add_join(set_i);
          auto op_1 = flow.add_intersect(set_i);

          auto op_2 = flow.add_join(set_k);
          auto op_3 = flow.add_intersect(set_k);

          auto op_4 = flow.add_posrank(op_3->get_id(), op_1->get_id());

          op_0->get_nodeset()->set_name("join words");
          op_1->get_nodeset()->set_name("intersect words");

          op_2->get_nodeset()->set_name("join pos");
          op_3->get_nodeset()->set_name("intersect pos");

          op_4->get_nodeset()->set_name("posranked words");

          flow.execute();

          flow.show();

          LOG_S(INFO) << "time to query [sec]: " << flow.time();
        }      
    }

    template<typename model_type>    
    bool text_masker<model_type>::get_nodes_and_masks(std::string line,
						      std::vector<node_type>& nodes,
						      std::vector<short>& masks)
    {
      nodes.clear();
      masks.clear();
      
      if(line=="quit")
	{
	  return false;
	}

      line = utils::replace(line, "___", node_names::MASK);
      line = utils::replace(line, "???", node_names::MASK);

      if(not utils::contains(line, node_names::MASK))
	{
	  line += " ";
	  line += node_names::MASK;
	}
      LOG_S(INFO) << "input-line: `" << line << "`";
      
      std::vector<std::string> words = utils::split(line, ' ');
      nodes.resize(words.size());

      for(std::size_t l=0; l<words.size(); l++)
	{
	  to_node(words.at(l), nodes.at(l));

	  LOG_S(INFO) << std::setw( 8) << nodes.at(l).pos  << " | "
		      << std::setw(24) << nodes.at(l).word << " |";	  
	}
      
      for(short l=0; l<nodes.size(); l++)
	{
	  if(nodes.at(l).word==node_names::MASK)
	    {
	      masks.push_back(l);
	    }
	}
      
      return true;
    }

    /*
    template<typename model_type>    
    bool text_masker<model_type>::to_node(std::string& word, node_type& node_j)
    {
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
	  node_j = node_i;
          return true;
        }

      query_flow<model_type> flow(model);

      auto op_i = flow.add_select(node_i);
      auto op_j = flow.add_traverse(edge_names::to_node, op_i->get_id());

      op_j->get_nodeset()->set_name("traverse nodes");
      
      flow.execute();
      //flow.show();

      {
	auto result = op_j->get_nodeset();

	std::vector<node_type> query_nodes;
	result->to_nodes(query_nodes);

	if(query_nodes.size()>0)
	  {
	    node_j = query_nodes.at(0);
	  }
	else
	  {
	    return false;
	  }
      }

      return true;
    }
    */    
  }

}

#endif
