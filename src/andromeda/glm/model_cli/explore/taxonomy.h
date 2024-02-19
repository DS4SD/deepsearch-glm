//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_TAXONOMY_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_EXPLORE_TAXONOMY_H_

namespace andromeda
{
  namespace glm
  {

    template<typename model_type>
    class taxonomy: public base_explorer<model_type>
    {
      typedef base_explorer<model_type> base_type;

      typedef typename base_type::glm_node_type glm_node_type;
      typedef typename base_type::qry_node_type qry_node_type;

    public:

      taxonomy(std::shared_ptr<model_type> model);
      ~taxonomy();

      void explore_word();

      void explore_taxonomy();
    };

    template<typename model_type>
    taxonomy<model_type>::taxonomy(std::shared_ptr<model_type> model):
      base_type(model)
    {}

    template<typename model_type>
    taxonomy<model_type>::~taxonomy()
    {}

    template<typename model_type>
    void taxonomy<model_type>::explore_word()
    {
      std::string word;

      while(true)
        {
          std::cout << "exploring word: ";
          std::getline(std::cin, word);

          if(word=="quit")
            {
              break;
            }

	  {
	    query_flow<model_type> flow(base_type::model);
	    
	    auto op_0 = flow.add_select(word);
	    op_0->get_nodeset()->set_name("search");

	    auto op_1 = flow.add_traverse(edge_names::to_pos, op_0->get_flid());
	    op_1->get_nodeset()->set_name("to-POS");
	    
	    auto op_2 = flow.add_traverse(edge_names::prev, op_0->get_flid());
	    op_2->get_nodeset()->set_name("prev");

	    auto op_3 = flow.add_traverse(edge_names::next, op_0->get_flid());
	    op_3->get_nodeset()->set_name("next");
	    
	    flow.execute();
	    flow.show();
	    
	    auto result_0 = op_0->get_nodeset();
	    result_0->show(32);

	    auto result_1 = op_1->get_nodeset();
	    result_1->show(32);

	    auto result_2 = op_2->get_nodeset();
	    result_2->show(32);

	    auto result_3 = op_3->get_nodeset();
	    result_3->show(32);	    
	  }
        }
    }

    template<typename model_type>
    void taxonomy<model_type>::explore_taxonomy()
    {
      std::string line;

      while(true)
        {
          std::cout << "exploring related terms to [word]: ";
          std::getline(std::cin, line);

          if(line=="quit")
            {
              break;
            }

	  std::vector<std::string> words = utils::split(line, " ");
	  std::vector<std::vector<std::string> > path = {words};
	  {
	    query_flow<model_type> flow(base_type::model);
	    
	    auto op_0a = flow.add_select(words);
	    op_0a->get_nodeset()->set_name("search");

	    auto op_1a = flow.add_traverse(edge_names::from_token, op_0a->get_flid());
	    op_1a->get_nodeset()->set_name("from-token");
	    
	    auto op_0b = flow.add_select(words.back());
	    op_0b->get_nodeset()->set_name("search-root");
	    
	    auto op_0c = flow.add_select(path);
	    op_0c->get_nodeset()->set_name("search-path");

	    auto op_0d = flow.add_traverse(edge_names::from_token, op_0c->get_flid());
	    op_0d->get_nodeset()->set_name("from-token");
	    
	    //auto op_2 = flow.add_traverse(edge_names::from_root_to_path, op_0b->get_flid());
	    //op_2->get_nodeset()->set_name("from-root-to-path");

	    auto op_3 = flow.add_traverse(edge_names::tax_up, op_0c->get_flid());
	    op_3->get_nodeset()->set_name("tax-up");

	    auto op_4a = flow.add_filter({node_names::WORD_TOKEN}, {op_3->get_flid()});
	    op_4a->get_nodeset()->set_name("tax-up tokens");

	    auto op_4b = flow.add_filter({node_names::TERM}, {op_3->get_flid()});
	    op_4b->get_nodeset()->set_name("tax-up terms");


	    auto op_5a = flow.add_traverse(edge_names::to_root, op_0d->get_flid());
	    op_5a->get_nodeset()->set_name("to-root");

	    auto op_5b = flow.add_traverse(edge_names::from_root, op_5a->get_flid());
	    op_5b->get_nodeset()->set_name("from-root");

	    auto op_5c = flow.add_filter({node_names::TERM}, {op_0d->get_flid()});
	    op_5c->get_nodeset()->set_name("terms containing `from-token`");

	    
	    
	    /*
	    auto op_4a = flow.add_traverse(edge_names::to_verb, op_2->get_id());
	    op_4a->get_nodeset()->set_name("to-verb");

	    auto op_4b = flow.add_traverse(edge_names::to_term, op_2->get_id());
	    op_4b->get_nodeset()->set_name("to-term");

	    auto op_4c = flow.add_traverse(edge_names::from_verb, op_2->get_id());
	    op_4c->get_nodeset()->set_name("from-root-to-path -> from-verb");

	    auto op_4d = flow.add_traverse(edge_names::from_term, op_2->get_id());
	    op_4d->get_nodeset()->set_name("from-term");

	    auto op_5 = flow.add_traverse(edge_names::from_verb, op_0c->get_id());
	    op_5->get_nodeset()->set_name("search-path -> from-verb");
	    */
	    
	    flow.execute();
	    flow.show();
	    
	    auto result_0a = op_0a->get_nodeset();
	    result_0a->show(32);

	    auto result_1a = op_1a->get_nodeset();
	    result_1a->show(32);
	    
	    auto result_0b = op_0b->get_nodeset();
	    result_0b->show(32);

	    auto result_0c = op_0c->get_nodeset();
	    result_0c->show(32);

	    auto result_0d = op_0d->get_nodeset();
	    result_0d->show(32);
	    
	    //auto result_1b = op_1b->get_nodeset();
	    //result_1b->show(32);

	    //auto result_2 = op_2->get_nodeset();
	    //result_2->show(32);

	    auto result_3 = op_3->get_nodeset();
	    result_3->show(32);

	    
	    auto result_4a = op_4a->get_nodeset();
	    result_4a->show(32);

	    auto result_4b = op_4b->get_nodeset();
	    result_4b->show(32);

	    /*
	    auto result_4c = op_4c->get_nodeset();
	    result_4c->show(32);

	    auto result_4d = op_4d->get_nodeset();
	    result_4d->show(32);

	    auto result_5 = op_5->get_nodeset();
	    result_5->show(32);	    	    
	    */

	    auto result_5a = op_5a->get_nodeset();
	    result_5a->show(32);

	    auto result_5b = op_5b->get_nodeset();
	    result_5b->show(32);

	    auto result_5c = op_5c->get_nodeset();
	    result_5c->show(32);	    	    
	  }	  
	}
    }

  }

}

#endif
