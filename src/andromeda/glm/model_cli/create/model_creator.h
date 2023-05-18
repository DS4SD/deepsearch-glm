//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_CREATOR_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_CREATOR_H_

namespace andromeda
{
  namespace glm
  {
    class model_creator: public base_types,
                         public model_types
    {
      typedef model model_type;

    public:

      model_creator(std::shared_ptr<model_type> model);

      void update(subject<PARAGRAPH>& subj, std::set<hash_type>& docs_cnt);
      void update(subject<TABLE>& subj, std::set<hash_type>& docs_cnt);

      void update(subject<DOCUMENT>& subj, std::set<hash_type>& docs_cnt);

    private:

      void contract_tokens(subject<PARAGRAPH>& subj);

      void contract_tokens(subject<TABLE>& subj);

      void contract_tokens(subject<DOCUMENT>& subj);

      void update_tokens(std::vector<word_token>& tokens,
                         std::vector<base_entity>& entities);

      void insert_nodes(nodes_type& nodes,
                        std::vector<word_token>& tokens,
                        std::vector<hash_type>& tok_hashes,
                        std::vector<hash_type>& pos_hashes);

      void update_counters(subject_name name, nodes_type& nodes,
                           std::vector<base_entity>& entities,
                           std::vector<hash_type>& hashes,
			   std::set<hash_type>& text_cnt,
			   std::set<hash_type>& tabl_cnt,
                           std::set<hash_type>& docs_cnt);

      void update_counters(
			   nodes_type& nodes,
                           std::vector<base_entity>& entities,
                           std::map<range_type, hash_type>& ent_rngs,

                           std::set<hash_type>& docs_cnt);

      void insert_edges(std::vector<hash_type>& tok_hashes,
                        std::vector<hash_type>& pos_hashes,
                        edges_type& edges);

      void insert_edges(int padding, edges_type& edges,
                        std::vector<hash_type>& hashes);

      void insert_edges(std::vector<base_entity>& ents,
                        edges_type& edges,
                        std::vector<hash_type>& hashes);

      void insert_begin_and_end_of_paths(std::vector<word_token>& tokens,
                                         std::vector<base_entity>& ents,
                                         std::vector<base_relation>& rels,
                                         nodes_type& nodes, edges_type& edges,
                                         std::vector<hash_type>& tok_hashes);

      void insert_concatenation_paths(std::vector<word_token>& tokens,
                                      std::vector<base_entity>& entities,
                                      std::vector<base_relation>& relations,
                                      nodes_type& nodes,
                                      edges_type& edges,
                                      std::vector<hash_type>& tok_hashes,
                                      std::map<range_type, hash_type>& path_hashes);

      void insert_conn_paths(std::vector<word_token>& tokens,
                             std::vector<base_entity>& entities,
                             std::vector<base_relation>& relations,
                             nodes_type& nodes, edges_type& edges,
                             std::vector<hash_type>& tok_hashes,
                             std::map<range_type, hash_type>& rng_to_conn);

      void insert_term_paths(std::vector<word_token>& tokens,
                             std::vector<base_entity>& entities,
                             std::vector<base_relation>& relations,
                             nodes_type& nodes, edges_type& edges,
                             std::vector<hash_type>& tok_hashes,
                             std::map<range_type, hash_type>& rng_to_term,
			     std::map<hash_type, hash_type>& ihash_to_node);

      void insert_verb_paths(std::vector<word_token>& tokens,
                             std::vector<base_entity>& entities,
                             std::vector<base_relation>& relations,
                             nodes_type& nodes, edges_type& edges,
                             std::vector<hash_type>& tok_hashes,
                             std::map<range_type, hash_type>& rng_to_verb,
			     std::map<hash_type, hash_type>& ihash_to_node);

      void insert_padding_for_conn_verb_term(int padding,
                                             std::vector<base_entity>& entities,
                                             nodes_type& nodes, edges_type& edges,
                                             std::vector<hash_type>& tok_hashes,
                                             std::vector<hash_type>& sent_hashes,
                                             std::map<range_type, hash_type>& rng_to_conn,
                                             std::map<range_type, hash_type>& rng_to_term,
                                             std::map<range_type, hash_type>& rng_to_verb);

      void insert_triplets(nodes_type& nodes, edges_type& edges,
                           std::map<range_type, hash_type>& rng_to_conn,
                           std::map<range_type, hash_type>& rng_to_term,
                           std::map<range_type, hash_type>& rng_to_verb);

      void insert_sentences(std::vector<base_entity>& entities,
                            nodes_type& nodes, edges_type& edges,
                            std::vector<hash_type>& tok_hashes,
                            std::vector<hash_type>& sent_hashes);

      void insert_sentences(std::vector<base_entity>& entities,
                            nodes_type& nodes, edges_type& edges,
                            std::vector<hash_type>& tok_hashes,
                            std::vector<hash_type>& sent_hashes,
                            std::map<range_type, hash_type>& rng_to_conn,
                            std::map<range_type, hash_type>& rng_to_term,
                            std::map<range_type, hash_type>& rng_to_verb);

      void insert_texts(std::vector<base_entity>& entities,
                        nodes_type& nodes, edges_type& edges,
                        std::vector<hash_type>& sent_hashes);

      void insert_tables(std::vector<base_entity>& entities,
			 nodes_type& nodes, edges_type& edges,
			 std::vector<hash_type>& sent_hashes);      

      void insert_relations(std::vector<base_entity>& entities,
			    std::vector<base_relation>& relations,
			    nodes_type& nodes, edges_type& edges,
			    std::map<hash_type, hash_type>& ihash_to_node);
      
    private:

      std::shared_ptr<model_type> model;

      std::size_t beg_term_hash, end_term_hash,
        beg_sent_hash, end_sent_hash,
        beg_text_hash, end_text_hash,
        undef_pos_hash;
    };

    model_creator::model_creator(std::shared_ptr<model_type> model):
      model(model),

      beg_term_hash(node_names::DEFAULT_HASH),
      end_term_hash(node_names::DEFAULT_HASH),

      beg_sent_hash(node_names::DEFAULT_HASH),
      end_sent_hash(node_names::DEFAULT_HASH),

      beg_text_hash(node_names::DEFAULT_HASH),
      end_text_hash(node_names::DEFAULT_HASH),

      undef_pos_hash(node_names::DEFAULT_HASH)
    {
      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();

      nodes.initialise();
      edges.initialise();

      beg_term_hash = node_names::to_hash.at(node_names::BEG_TERM);
      end_term_hash = node_names::to_hash.at(node_names::END_TERM);

      beg_sent_hash = node_names::to_hash.at(node_names::BEG_SENT);
      end_sent_hash = node_names::to_hash.at(node_names::END_SENT);

      beg_text_hash = node_names::to_hash.at(node_names::BEG_TEXT);
      end_text_hash = node_names::to_hash.at(node_names::END_TEXT);

      undef_pos_hash = node_names::to_hash.at(node_names::UNDEFINED_POS);
    }

    void model_creator::update(subject<PARAGRAPH>& subj,
			       std::set<hash_type>& docs_cnt) // hashes of nodes already in doc
    {
      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();

      auto& parameters = model->get_parameters();

      base_node text_node(node_names::TEXT, subj.get_hash());      
      if(parameters.keep_texts)
	{
	  text_node = nodes.insert(text_node, false);
	}
      
      std::vector<word_token>& tokens = subj.word_tokens;

      std::vector<base_entity>& entities = subj.entities;
      std::vector<base_relation>& relations = subj.relations;

      if(tokens.size()==0)
        {
           return;
        }

      update_tokens(tokens, entities);

      std::vector<hash_type> tok_hashes={}, pos_hashes={};
      std::set<hash_type> text_hashes={}, table_hashes={};
      
      insert_nodes(nodes, tokens, tok_hashes, pos_hashes);

      update_counters(PARAGRAPH, nodes, entities, tok_hashes, text_hashes, table_hashes, docs_cnt);
      update_counters(PARAGRAPH, nodes, entities, pos_hashes, text_hashes, table_hashes, docs_cnt);

      insert_edges(tok_hashes, pos_hashes, edges);

      insert_edges(parameters.padding, edges, tok_hashes);
      insert_edges(parameters.padding, edges, pos_hashes);

      insert_begin_and_end_of_paths(tokens, entities, relations,
                                    nodes, edges, tok_hashes);

      std::map<hash_type, hash_type> ihash_to_node={};
      
      std::map<range_type, hash_type> rng_to_conc={};
      std::map<range_type, hash_type> rng_to_conn={};
      std::map<range_type, hash_type> rng_to_term={};
      std::map<range_type, hash_type> rng_to_verb={};
      
      if(parameters.keep_concs)
        {
          insert_concatenation_paths(tokens, entities, relations,
                                     nodes, edges, 
                                     tok_hashes, rng_to_conc);

          update_counters(nodes, entities, rng_to_conc, docs_cnt);
        }

      if(parameters.keep_conns)
        {
          insert_conn_paths(tokens, entities, relations,
                            nodes, edges, 
                            tok_hashes, rng_to_conn);

          update_counters(nodes, entities, rng_to_conn, docs_cnt);
        }

      if(parameters.keep_terms)
        {
          insert_term_paths(tokens, entities, relations,
                            nodes, edges, 
                            tok_hashes, rng_to_term,
			    ihash_to_node);

          update_counters(nodes, entities, rng_to_term, docs_cnt);
        }

      if(parameters.keep_verbs)
        {
          insert_verb_paths(tokens, entities, relations,
                            nodes, edges, 
                            tok_hashes, rng_to_verb,
			    ihash_to_node);

          update_counters(nodes, entities, rng_to_verb, docs_cnt);
        }

      std::vector<hash_type> sent_hashes={};
      
      if(true)
        {
          insert_padding_for_conn_verb_term(parameters.padding,
                                            entities,
                                            nodes, edges,
                                            tok_hashes, sent_hashes,
                                            rng_to_conn, rng_to_term, rng_to_verb);
        }

      if(parameters.keep_sents)
        {
          insert_sentences(entities, nodes, edges,
                           tok_hashes, sent_hashes,
                           rng_to_conn, rng_to_term, rng_to_verb);
        }

      if(sent_hashes.size()>=2 and
         parameters.keep_sents and
         parameters.keep_texts)
        {
          insert_texts(entities, nodes, edges, sent_hashes);
        }

      if(true)
	{
	  insert_relations(entities, relations, nodes, edges, ihash_to_node);
	}
    }

    void model_creator::update(subject<TABLE>& subj,
			       std::set<hash_type>& docs_cnt) // hashes of nodes already in doc`
    {      
      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();

      auto& parameters = model->get_parameters();

      base_node table_node(node_names::TABL, subj.get_hash());      
      if(parameters.keep_tabls)
	{
	  //LOG_S(INFO) << "table-cnt(" << table_node.get_hash() << "): " << table_node.get_tabl_cnt();
	  table_node = nodes.insert(table_node, false);
	  //LOG_S(INFO) << " -> " << table_node.get_tabl_cnt();
	}
      
      subj.sort();
      //subj.show(true, true, false);
      
      std::vector<base_entity>& entities = subj.entities;
      //std::vector<base_relation>& relations = subj.relations;

      std::set<hash_type> text_cnt={}, tabl_cnt={};
      
      for(uint64_t i=0; i<subj.num_rows(); i++)
	{
	  for(uint64_t j=0; j<subj.num_cols(); j++)
	    {
	      if(subj(i,j).skip())
		{
		  continue;
		}
	      std::vector<word_token>& tokens = subj(i,j).word_tokens;

	      //LOG_S(INFO) << "(i, j): " << i << ", " << j;
	      //LOG_S(INFO) << andromeda::tabulate(tokens, subj(i,j).text);
	      
	      std::vector<hash_type> tok_hashes={}, pos_hashes={};//, sent_hashes={};
	      insert_nodes(nodes, tokens, tok_hashes, pos_hashes);

	      update_counters(TABLE, nodes, entities, tok_hashes, text_cnt, tabl_cnt, docs_cnt);
	      update_counters(TABLE, nodes, entities, pos_hashes, text_cnt, tabl_cnt, docs_cnt);

	      insert_edges(tok_hashes, pos_hashes, edges);
      
	      std::vector<hash_type> node_term_hashes={};
      
	      for(auto itr=subj.ents_beg({i,j}); itr!=subj.ents_end({i,j}); itr++)
		{
		  assert(i==(itr->coor)[0]);
		  assert(j==(itr->coor)[1]);
		  
		  const base_entity& ent = *itr;
		  //LOG_S(INFO) << "ent: " << ent.to_json().dump();
		  
		  auto rng = ent.wtok_range;

		  if(ent.model_type==andromeda::TERM and
		     ent.model_subtype=="single-term")
		    {
		      std::vector<hash_type> term_hashes={};
		      for(std::size_t i=rng[0]; i<rng[1]; i++)
			{
			  term_hashes.push_back(tok_hashes.at(i));
			}
		      
		      base_node tmp(node_names::TERM, term_hashes);
		      base_node& term_i = nodes.insert(tmp, false);
		      
		      term_i.incr_word_cnt();
		      
		      auto tabl_ins = tabl_cnt.insert(term_i.get_hash());
		      term_i.incr_tabl_cnt(tabl_ins.second);

		      auto docs_ins = docs_cnt.insert(term_i.get_hash());
		      term_i.incr_fdoc_cnt(docs_ins.second);

		      if(term_i.get_text(nodes, false)=="Income")
			{
			  LOG_S(INFO) << table_node.get_hash() << ", "<< term_i.get_hash() << "\t"
				      << tabl_ins.second << ": " << term_i.get_tabl_cnt();
			  
			  LOG_S(INFO) << "\n counters: " << term_i.to_json(nodes)["counters"].dump();
			}
		      
		      if(term_hashes.size()==1)
			{
			  edges.insert(edge_names::from_token, term_hashes.at(0), term_i.get_hash(), false);
			  edges.insert(edge_names::to_token, term_i.get_hash(), term_hashes.at(0), false);
			}

		      if(parameters.keep_tabls)
			{
			  edges.insert(edge_names::from_table, table_node.get_hash(), term_i.get_hash(), false);
			  edges.insert(edge_names::to_table, term_i.get_hash(), table_node.get_hash(), false);
			}
		    }
		}
	    }
	}
    }
    
    void model_creator::update(subject<DOCUMENT>& subj, std::set<hash_type>& doc_ents)
    {
      auto& nodes = model->get_nodes();
      //auto& edges = model->get_edges();

      auto& parameters = model->get_parameters();
      
      doc_ents.clear();

      for(auto& paragraph:subj.paragraphs)
        {
          this->update(paragraph, doc_ents);
        }
      
      for(auto& table:subj.tables)
        {
          this->update(table, doc_ents);
        }
      
      base_node fdoc_node(node_names::FDOC, subj.get_hash());      
      if(parameters.keep_fdocs)
	{
	  fdoc_node = nodes.insert(fdoc_node, false);
	}      
    }









    
    void model_creator::contract_tokens(subject<PARAGRAPH>& subj)
    {
      subj.contract_wtokens_from_entities(LINK);
      subj.contract_wtokens_from_entities(CITE);
      subj.contract_wtokens_from_entities(NAME);
    }

    void model_creator::contract_tokens(subject<TABLE>& subj)
    {

    }

    void model_creator::contract_tokens(subject<DOCUMENT>& subj)
    {
      for(auto item:subj.paragraphs)
        {
          contract_tokens(item);
        }

      for(auto item:subj.tables)
        {
          contract_tokens(item);
        }
    }

    void model_creator::update_tokens(std::vector<word_token>& tokens,
                                      std::vector<base_entity>& entities)
    {
      for(auto& ent:entities)
        {
          auto rng = ent.wtok_range;

          std::string subtype = ent.model_subtype;

          if(ent.model_type==andromeda::NUMVAL and (rng[1]-rng[0])==1)
            {
              tokens.at(rng[0]).set_word("__"+subtype+"__");
            }
          else if(ent.model_type==andromeda::LINK and (rng[1]-rng[0])==1)
            {
              tokens.at(rng[0]).set_word("__"+subtype+"__");
            }
          else if(ent.model_type==andromeda::CITE and (rng[1]-rng[0])==1)
            {
              tokens.at(rng[0]).set_word("__"+subtype+"__");
            }
          else if(ent.model_type==andromeda::PARENTHESIS)
            {
              /*
                auto rng = ent.word_range;

                std::cout << " -> parenthesis\n";
                for(std::size_t i=rng[0]; i<rng[1]; i++)
                {
                std::cout << std::setw(8) << tokens.at(i).pos << std::setw(24) << tokens.at(i).word << "\n";
                }
              */
            }
          else
            {}
        }
    }

    void model_creator::insert_nodes(nodes_type& nodes,
                                     std::vector<word_token>& tokens,
                                     std::vector<hash_type>& tok_hashes,
                                     std::vector<hash_type>& pos_hashes)
    {
      for(word_token& token:tokens)
        {
          std::string text = token.get_word();
          std::string pos  = token.get_pos();

          {
            auto& node = nodes.insert(node_names::TOKEN, text);
            tok_hashes.push_back(node.get_hash());
          }

          {
            auto& node = nodes.insert(node_names::SYNTX, pos);
            pos_hashes.push_back(node.get_hash());
          }
        }
    }

    void model_creator::update_counters(subject_name name, nodes_type& nodes,
                                        std::vector<base_entity>& entities,
                                        std::vector<hash_type>& hashes,
                                        std::set<hash_type>& text_cnt,
					std::set<hash_type>& tabl_cnt,
					std::set<hash_type>& fdoc_cnt)
    {      
      std::set<hash_type> sent_beg={};

      if(name==PARAGRAPH)
	{
	  for(auto& ent:entities)
	    {
	      if(ent.model_type==andromeda::SENTENCE)
		{
		  auto rng = ent.wtok_range;
		  sent_beg.insert(rng[0]);
		  sent_beg.insert(rng[1]);
		}
	    }
	}

      std::set<hash_type> sent_cnt={};

      for(hash_type l=0; l<hashes.size(); l++)
        {
          if(sent_beg.count(l))
            {
              sent_cnt={};
            }

          auto& hash = hashes.at(l);
          auto& node = nodes.get(hash);

          auto sent_ins = sent_cnt.insert(hash);
          auto text_ins = text_cnt.insert(hash);
	  auto tabl_ins = tabl_cnt.insert(hash);
          auto docs_ins = fdoc_cnt.insert(hash);

          node.incr_word_cnt();// += 1;

	  if(name==PARAGRAPH)
	    {
	      node.incr_sent_cnt(sent_ins.second);
	      node.incr_text_cnt(text_ins.second);
	    }
	  else if(name==TABLE)	    
	    {
	      node.incr_tabl_cnt(tabl_ins.second);
	    }
	  else
	    {}
	  
          node.incr_fdoc_cnt(docs_ins.second);
        }
    }

    void model_creator::update_counters(nodes_type& nodes,
                                        std::vector<base_entity>& entities,
                                        std::map<range_type, hash_type>& ent_rngs,
                                        std::set<hash_type>& fdoc_cnt)
    {
      std::set<hash_type> text_cnt={};
      for(auto& ent_rng:ent_rngs)
        {
          hash_type hash = ent_rng.second;

          auto& node = nodes.get(hash);
          node.incr_word_cnt();

          auto text_ins = text_cnt.insert(hash);
          node.incr_text_cnt(text_ins.second);

          auto docs_ins = fdoc_cnt.insert(hash);
          node.incr_fdoc_cnt(docs_ins.second);
        }

      std::set<range_type> sent_rngs={};
      for(auto& ent:entities)
        {
          if(ent.model_type==andromeda::SENTENCE)
            {
              auto rng = ent.wtok_range;
              sent_rngs.insert(rng);
            }
        }

      std::set<hash_type> sent_cnt={};
      for(auto& sent_rng:sent_rngs)
        {
          sent_cnt={};

          for(auto& ent_rng:ent_rngs)
            {
              range_type rng = ent_rng.first;
              hash_type hash = ent_rng.second;

              if(sent_rng.at(0)<=rng.at(0) and rng.at(1)<=sent_rng.at(1))
                {
                  auto sent_ins = sent_cnt.insert(hash);

                  auto& node = nodes.get(hash);
                  node.incr_sent_cnt(sent_ins.second);
                }
            }
        }
    }

    void model_creator::insert_edges(std::vector<hash_type>& tok_hashes,
                                     std::vector<hash_type>& pos_hashes,
                                     edges_type& edges)
    {
      for(std::size_t l=0; l<tok_hashes.size(); l++)
        {
          if(pos_hashes.at(l)==undef_pos_hash)
            {
              continue;
            }
          else
            {
              edges.insert(edge_names::to_pos , tok_hashes.at(l), pos_hashes.at(l), false);
              edges.insert(edge_names::from_pos, pos_hashes.at(l), tok_hashes.at(l), false);
            }
        }
    }

    void model_creator::insert_edges(int padding, edges_type& edges,
                                     std::vector<hash_type>& hashes)
    {
      if(padding>0)
        {
          edges.insert(edge_names::next, beg_text_hash, hashes.front(), false);
          edges.insert(edge_names::next, hashes.back(), end_text_hash, false);

          edges.insert(edge_names::prev, end_text_hash, hashes.back(), false);
        }

      for(int i=0; i<hashes.size(); i++)
        {
          for(int d=1; d<=padding; d++)
            {
              if(i+d<hashes.size())
                {
                  edges.insert(d, hashes.at(i), hashes.at(i+d), false);
                }

              if(0<=i-d)
                {
                  edges.insert(-d, hashes.at(i), hashes.at(i-d), false);
                }
            }
        }
    }

    void model_creator::insert_begin_and_end_of_paths(std::vector<word_token>& tokens,
                                                      std::vector<base_entity>& entities,
                                                      std::vector<base_relation>& relations,
                                                      nodes_type& nodes, edges_type& edges,
                                                      std::vector<hash_type>& tok_hashes)
    {
      for(auto& ent:entities)
        {
          if(ent.model_type==andromeda::TERM)
            {
              nodes.get(beg_term_hash).incr_word_cnt();// += 1;
              nodes.get(end_term_hash).incr_word_cnt();// += 1;

              auto rng = ent.wtok_range;

              edges.insert(edge_names::to_label, tok_hashes.at(rng[0]  ), beg_term_hash, false);
              edges.insert(edge_names::to_label, tok_hashes.at(rng[1]-1), end_term_hash, false);

              edges.insert(edge_names::from_label, beg_term_hash, tok_hashes.at(rng[0]), false);
              edges.insert(edge_names::from_label, end_term_hash, tok_hashes.at(rng[1]-1), false);
	      
              edges.insert(edge_names::tax_up, end_term_hash, tok_hashes.at(rng[1]-1), false);
            }

          if(ent.model_type==andromeda::SENTENCE)
            {
              nodes.get(beg_sent_hash).incr_word_cnt();// += 1;
              nodes.get(end_sent_hash).incr_word_cnt();// += 1;

              auto rng = ent.wtok_range;

              edges.insert(edge_names::to_label, tok_hashes.at(rng[0]  ), beg_sent_hash, false);
              edges.insert(edge_names::to_label, tok_hashes.at(rng[1]-1), end_sent_hash, false);

              edges.insert(edge_names::from_label, beg_sent_hash, tok_hashes.at(rng[0]), false);
              edges.insert(edge_names::from_label, end_sent_hash, tok_hashes.at(rng[1]-1), false);
            }
        }

      if(tok_hashes.size()>0)
        {
          nodes.get(beg_text_hash).incr_word_cnt();// += 1;
          nodes.get(end_text_hash).incr_word_cnt();// += 1;

          edges.insert(edge_names::to_label, tok_hashes.front(), beg_text_hash, false);
          edges.insert(edge_names::to_label, tok_hashes.back(), end_text_hash, false);

          edges.insert(edge_names::from_label, beg_text_hash, tok_hashes.front(), false);
          edges.insert(edge_names::from_label, end_text_hash, tok_hashes.back(), false);
        }
    }

    void model_creator::insert_concatenation_paths(std::vector<word_token>& tokens,
                                                   std::vector<base_entity>& entities,
                                                   std::vector<base_relation>& relations,
                                                   nodes_type& nodes,
                                                   edges_type& edges,
                                                   std::vector<hash_type>& tok_hashes,
                                                   std::map<range_type, hash_type>& rng_to_hash)
    {
      for(auto& ent:entities)
        {
          if(ent.model_type==andromeda::EXPRESSION and
             (ent.model_subtype=="name-concatenation" or
              ent.model_subtype=="word-concatenation" or
              ent.model_subtype=="latex-concatenation") and
             ent.name.find("-")!=std::string::npos and
             ent.name.find(" ")==std::string::npos and
             (ent.wtok_range[1]-ent.wtok_range[0])==1)
            {
              auto rng = ent.wtok_range;

              hash_type hash = tok_hashes.at(rng[0]);
              auto& node = nodes.get(hash);

              std::string text = node.get_text();

              std::vector<std::string> parts = utils::split(text, "-");

              std::vector<hash_type> cont_hashes={};
              for(std::string& part:parts)
                {
                  auto& node = nodes.insert(node_names::TOKEN, part);
                  cont_hashes.push_back(node.get_hash());
                }

              if(cont_hashes.size()>=2)
                {
                  base_node path(node_names::CONT, cont_hashes);
                  nodes.insert(path, false);

                  rng_to_hash.emplace(ent.wtok_range, path.get_hash());

                  for(std::size_t i=0; i<cont_hashes.size()-1; i++)
                    {
                      edges.insert(edge_names::tax_dn, cont_hashes.at(i), cont_hashes.at(i+1), false);
                    }

                  for(std::size_t i=1; i<cont_hashes.size(); i++)
                    {
                      edges.insert(edge_names::tax_up, cont_hashes.at(i), cont_hashes.at(i-1), false);
                    }

                  for(std::size_t i=0; i<cont_hashes.size(); i++)
                    {
                      edges.insert(edge_names::from_token, cont_hashes.at(i), path.get_hash(), false);
                      edges.insert(edge_names::to_token  , path.get_hash(), cont_hashes.at(i), false);
                    }
                }
            }
        }
    }

    void model_creator::insert_conn_paths(std::vector<word_token>& tokens,
                                          std::vector<base_entity>& entities,
                                          std::vector<base_relation>& relations,
                                          nodes_type& nodes, edges_type& edges,
                                          std::vector<hash_type>& tok_hashes,
                                          std::map<range_type, hash_type>& rng_to_conn)
    {
      for(auto& ent:entities)
        {
          if(ent.model_type==andromeda::CONN)
            {
              auto rng = ent.wtok_range;

              std::vector<hash_type> hashes={};
              for(std::size_t i=rng[0]; i<rng[1]; i++)
                {
                  hashes.push_back(tok_hashes.at(i));
                }

              base_node path(node_names::CONN, hashes);
              nodes.insert(path, false);

              rng_to_conn.emplace(ent.wtok_range, path.get_hash());
            }
        }
    }

    void model_creator::insert_term_paths(std::vector<word_token>& tokens,
                                          std::vector<base_entity>& entities,
                                          std::vector<base_relation>& relations,
                                          nodes_type& nodes, edges_type& edges,
                                          std::vector<hash_type>& tok_hashes,
                                          std::map<range_type, hash_type>& rng_to_term,
					  std::map<hash_type, hash_type>& ihash_to_node)
    {
      for(auto& ent:entities)
        {
          if(ent.model_type==andromeda::TERM and
             ent.model_subtype=="single-term")
            {
              auto rng = ent.wtok_range;

              std::vector<hash_type> term_hashes={};
              for(std::size_t i=rng[0]; i<rng[1]; i++)
                {
                  term_hashes.push_back(tok_hashes.at(i));
                }

	      /*
              edges.insert(edge_names::to_label, term_hashes.front(), beg_term_hash, false);
              edges.insert(edge_names::to_label, term_hashes.back(), end_term_hash, false);

              edges.insert(edge_names::from_label, beg_term_hash, term_hashes.front(), false);
              edges.insert(edge_names::from_label, end_term_hash, term_hashes.back(), false);

              for(std::size_t i=0; i<term_hashes.size()-1; i++)
                {
                  edges.insert(edge_names::tax_dn, term_hashes.at(i), term_hashes.at(i+1), false);
                }

              for(std::size_t i=1; i<term_hashes.size(); i++)
                {
                  edges.insert(edge_names::tax_up, term_hashes.at(i), term_hashes.at(i-1), false);
                }
	      */
	      
              base_node term_i(node_names::TERM, term_hashes);
              nodes.insert(term_i, false);

              rng_to_term.emplace(ent.wtok_range, term_i.get_hash());

	      if(term_hashes.size()==1)
		{
		  edges.insert(edge_names::from_token, term_hashes.at(0), term_i.get_hash(), false);
		  edges.insert(edge_names::to_token, term_i.get_hash(), term_hashes.at(0), false);		  
		}
	      
	      /*
              if(term_hashes.size()>1)
                {
                  edges.insert(edge_names::from_root_to_path, term_hashes.back(), path.get_hash(), false);
                  edges.insert(edge_names::from_path_to_root, path.get_hash(), term_hashes.back(), false);
                }
	      */
	      
              /*
                if(term_hashes.size()>=1)
                {
                base_node fpath(node_names::TERM, term_hashes);
                nodes.insert(fpath, false);

                rng_to_term.emplace(ent.wtok_range, path.get_hash());

                edges.insert(edge_names::from_root_to_path, term_hashes.back(), path.get_hash(), false);
                edges.insert(edge_names::from_path_to_root, path.get_hash(), term_hashes.back(), false);


                edges.insert(edge_names::from_root_to_path, term_hashes.back(), path.get_hash(), false);
                edges.insert(edge_names::from_path_to_root, path.get_hash(), term_hashes.back(), false);

                for(std::size_t i=0; i<term_hashes.size(); i++)
                {
                edges.insert(edge_names::to_path  , term_hashes.at(i), path.get_hash(), false);
                edges.insert(edge_names::from_path, path.get_hash(), term_hashes.at(i), false);
                }

                for(std::size_t i=0; i<term_hashes.size(); i++)
                {
                if(i+1==term_hashes.size())
                {
                edges.insert(edge_names::from_root_to_path, term_hashes.at(i), path.get_hash(), false);
                edges.insert(edge_names::from_path_to_root, path.get_hash(), term_hashes.at(i), false);
                }
                else
                {
                edges.insert(edge_names::from_desc_to_path, term_hashes.at(i), path.get_hash(), false);
                edges.insert(edge_names::from_path_to_desc, path.get_hash(), term_hashes.at(i), false);
                }
                }

                }
              */
            }
        }
    }

    void model_creator::insert_verb_paths(std::vector<word_token>& tokens,
                                          std::vector<base_entity>& entities,
                                          std::vector<base_relation>& relations,
                                          nodes_type& nodes,
                                          edges_type& edges,
                                          std::vector<hash_type>& tok_hashes,
                                          std::map<range_type, hash_type>& rng_to_verb,
					  std::map<hash_type, hash_type>& ihash_to_node)
    {
      for(auto& ent:entities)
        {
          if(ent.model_type==andromeda::VERB)
            {
              auto rng = ent.wtok_range;

              std::vector<hash_type> verb_hashes={};
              std::vector<std::string> pos={};

              for(std::size_t i=rng[0]; i<rng[1]; i++)
                {
                  verb_hashes.push_back(tok_hashes.at(i));
                  pos.push_back(tokens.at(i).get_pos());
                }

              if(verb_hashes.size()>=1)
                {
                  base_node path(node_names::VERB, verb_hashes);
                  nodes.insert(path, false);

                  rng_to_verb.emplace(ent.wtok_range, path.get_hash());

                  for(std::size_t i=0; i<verb_hashes.size(); i++)
                    {
                      if(pos.at(i).starts_with("V"))
                        {
                          edges.insert(edge_names::from_token, verb_hashes.at(i), path.get_hash(), false);
                          edges.insert(edge_names::to_token, path.get_hash(), verb_hashes.at(i), false);
                        }
                    }
                }
            }
        }
    }

    /*
      void model_creator::insert_triplets(nodes_type& nodes, edges_type& edges, paths_type& paths,
      std::map<range_type, hash_type>& rng_to_conn,
      std::map<range_type, hash_type>& rng_to_term,
      std::map<range_type, hash_type>& rng_to_verb)
      {
      for(auto itr_i=rng_to_conn.begin(); itr_i!=rng_to_conn.end(); itr_i++)
      {
      auto rng_i = itr_i->first;
      hash_type conn = itr_i->second, term_0=-1, term_1=-1;

      for(auto itr_j=rng_to_term.begin(); itr_j!=rng_to_term.end(); itr_j++)
      {
      auto rng_j = itr_j->first;

      if(rng_j[1]==rng_i[0])
      {
      term_0 = itr_j->second;
      }

      if(rng_j[0]==rng_i[1])
      {
      term_1 = itr_j->second;
      }
      }

      if(term_0!=-1 and term_1!=-1)
      {
      glm_path path(path_names::TRIPLET, {term_0, conn, term_1});
      paths.insert(path, false);

      edges.insert(edge_names::to_path, term_0, path.get_hash(), false);
      edges.insert(edge_names::to_path, term_1, path.get_hash(), false);

      edges.insert(edge_names::from_path, path.get_hash(), term_0, false);
      edges.insert(edge_names::from_path, path.get_hash(), term_1, false);
      }
      }
      }
    */

    /*
      void model_creator::insert_sentences(std::vector<base_entity>& entities,
      nodes_type& nodes, edges_type& edges, paths_type& paths,
      std::vector<hash_type>& tok_hashes,
      std::vector<hash_type>& sent_hashes)
      {
      sent_hashes.clear();

      for(auto& ent:entities)
      {
      if(ent.model_type==andromeda::SENTENCE)
      {
      std::vector<hash_type> path_hashes={};

      auto rng = ent.wtok_range;
      for(std::size_t l=rng[0]; l<rng[1]; l++)
      {
      path_hashes.push_back(tok_hashes.at(l));
      }

      glm_path path(path_names::SENT, path_hashes);
      paths.insert(path, false);

      sent_hashes.push_back(path.get_hash());

      for(hash_type hash:path_hashes)
      {
      if(terms.count(hash)==1 or verbs.count(hash)==1)
      {
      //edges.insert(edge_names::to_sent, hash, path.get_hash(), false);
      //edges.insert(edge_names::from_sent, path.get_hash(), hash, false);
      }
      else
      {}
      }
      }
      }
      }
    */

    void model_creator::insert_padding_for_conn_verb_term(int padding,
                                                          std::vector<base_entity>& entities,
                                                          nodes_type& nodes, edges_type& edges,
                                                          std::vector<hash_type>& tok_hashes,
                                                          std::vector<hash_type>& sent_hashes,
                                                          std::map<range_type, hash_type>& rng_to_conn,
                                                          std::map<range_type, hash_type>& rng_to_term,
                                                          std::map<range_type, hash_type>& rng_to_verb)
    {
      std::vector<hash_type> hashes = tok_hashes;

      std::set<hash_type> terms={};
      std::set<hash_type> verbs={};

      for(auto itr=rng_to_conn.begin(); itr!=rng_to_conn.end(); itr++)
        {
          for(index_type l=(itr->first)[0]; l<(itr->first)[1]; l++)
            {
              hashes.at(l) = itr->second;
            }
        }

      for(auto itr=rng_to_term.begin(); itr!=rng_to_term.end(); itr++)
        {
          for(index_type l=(itr->first)[0]; l<(itr->first)[1]; l++)
            {
              hashes.at(l) = itr->second;
              terms.insert(itr->second);
            }
        }

      for(auto itr=rng_to_verb.begin(); itr!=rng_to_verb.end(); itr++)
        {
          for(index_type l=(itr->first)[0]; l<(itr->first)[1]; l++)
            {
              hashes.at(l) = itr->second;
              verbs.insert(itr->second);
            }
        }

      {
        auto itr = hashes.begin();

        hash_type prev = *itr;
        while(itr!=hashes.end())
          {
            if(itr==hashes.begin())
              {
                prev = *itr;
                itr++;
              }
            else if(prev==*itr)
              {
                itr = hashes.erase(itr);
              }
            else
              {
                prev = *itr;
                itr++;
              }
          }
      }

      for(int i=0; i<hashes.size(); i++)
        {
          for(int d=1; d<=padding; d++)
            {
              if(i+d<hashes.size())
                {
                  edges.insert(d, hashes.at(i), hashes.at(i+d), false);
                }

              if(0<=i-d)
                {
                  edges.insert(-d, hashes.at(i), hashes.at(i-d), false);
                }
            }
        }
    }

    void model_creator::insert_sentences(std::vector<base_entity>& entities,
                                         nodes_type& nodes, edges_type& edges,
                                         std::vector<hash_type>& tok_hashes,
                                         std::vector<hash_type>& sent_hashes,
                                         std::map<range_type, hash_type>& rng_to_conn,
                                         std::map<range_type, hash_type>& rng_to_term,
                                         std::map<range_type, hash_type>& rng_to_verb)
    {
      sent_hashes.clear();

      std::vector<hash_type> hashes = tok_hashes;

      std::set<hash_type> terms={};
      std::set<hash_type> verbs={};

      for(auto itr=rng_to_conn.begin(); itr!=rng_to_conn.end(); itr++)
        {
          for(index_type l=(itr->first)[0]; l<(itr->first)[1]; l++)
            {
              hashes.at(l) = itr->second;
            }
        }

      for(auto itr=rng_to_term.begin(); itr!=rng_to_term.end(); itr++)
        {
          for(index_type l=(itr->first)[0]; l<(itr->first)[1]; l++)
            {
              hashes.at(l) = itr->second;
              terms.insert(itr->second);
            }
        }

      for(auto itr=rng_to_verb.begin(); itr!=rng_to_verb.end(); itr++)
        {
          for(index_type l=(itr->first)[0]; l<(itr->first)[1]; l++)
            {
              hashes.at(l) = itr->second;
              verbs.insert(itr->second);
            }
        }

      for(auto& ent:entities)
        {
          if(ent.model_type==andromeda::SENTENCE)
            {
              std::vector<hash_type> path_hashes={};

              auto rng = ent.wtok_range;
              for(index_type l=rng[0]; l<rng[1]; l++)
                {
                  if(path_hashes.size()==0)
                    {
                      path_hashes.push_back(hashes.at(l));
                    }
                  else if(path_hashes.back()!=hashes.at(l))
                    {
                      path_hashes.push_back(hashes.at(l));
                    }
                  else // repeating hash to be skipped
                    {}
                }

              base_node path(node_names::SENT, path_hashes);
              {
                path.incr_word_cnt();
                path.incr_sent_cnt();
                path.incr_text_cnt();
                path.incr_fdoc_cnt();
              }

              nodes.insert(path, false);

              sent_hashes.push_back(path.get_hash());

              for(hash_type hash:path_hashes)
                {
                  if(terms.count(hash)==1 or
                     verbs.count(hash)==1)
                    {
                      //edges.insert(edge_names::to_sent, hash, path.get_hash(), false);
                      //edges.insert(edge_names::from_sent, path.get_hash(), hash, false);
                    }
                  else
                    {}
                }
            }
        }
    }

    void model_creator::insert_texts(std::vector<base_entity>& entities,
                                     nodes_type& nodes, edges_type& edges,
                                     std::vector<hash_type>& sent_hashes)
    {
      base_node path(node_names::TEXT, sent_hashes);
      {
        path.incr_word_cnt();
        path.incr_sent_cnt();
        path.incr_text_cnt();
        path.incr_fdoc_cnt();
      }

      nodes.insert(path, false);
    }

    void model_creator::insert_relations(std::vector<base_entity>& entities,
					 std::vector<base_relation>& relations,
					 nodes_type& nodes, edges_type& edges,
					 std::map<hash_type, hash_type>& ihash_to_node)
    {
      for(auto& rel:relations)
	{
	  edge_names::update_flvr(rel.get_name());
	}
      
      for(auto& rel:relations)
	{
	  flvr_type flvr = edge_names::to_flvr(rel.get_name());
	  
	  hash_type ihash_i = rel.get_ihash_i();
	  hash_type ihash_j = rel.get_ihash_j();
	  
	  if(ihash_to_node.count(ihash_i) and
	     ihash_to_node.count(ihash_j))
	    {
	      edges.insert(flvr, ihash_to_node.at(ihash_i), ihash_to_node.at(ihash_j), 1, false);
	    }
	}
    }
    
  }

}

#endif
