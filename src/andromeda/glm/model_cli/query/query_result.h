//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_ALGOS_QUERY_RESULT_H
#define ANDROMEDA_MODELS_GLM_ALGOS_QUERY_RESULT_H

namespace andromeda
{
  namespace glm
  {
    template<typename model_type>
    class query_result
    {
    public:

      typedef typename model_type::flvr_type flvr_type;
      typedef typename model_type::hash_type hash_type;

      typedef typename model_type::cnt_type cnt_type;
      typedef typename model_type::ind_type ind_type;
      typedef typename model_type::val_type val_type;

      typedef typename model_type::node_type glm_node_type;
      typedef typename model_type::edge_type glm_edge_type;

      typedef query_node qry_node_type;
      typedef query_edge qry_edge_type;

      typedef typename std::vector<qry_node_type>::iterator node_itr_type;
      typedef typename std::vector<qry_edge_type>::iterator edge_itr_type;

      const static inline std::vector<std::string> node_headers = { "flavor", "name",
                                                                    "hash", "weight", "prob", "cumul",
                                                                    "text", "count"};

      const static inline std::vector<std::string> edge_headers = { "flavor", "name",
                                                                    "hash", "hash_i", "hash_j", "weight",
                                                                    "prob"};

    public:

      query_result(std::shared_ptr<model_type> model);

      nlohmann::json to_json();

      nlohmann::json to_json(cnt_type num_nodes,
                             cnt_type num_edges,
                             cnt_type ind_nodes,
                             cnt_type ind_edges);

      void show(std::size_t max=16);

      std::string get_name() { return name; }
      std::string get_description() { return description; }

      void set_name(std::string name) { this->name=name; }
      void set_description(std::string description) { this->description=description; }

      val_type get_sum() { return sum; }

      val_type get_prob_avg() { return prob_avg; }
      val_type get_prob_std() { return prob_std; }
      val_type get_prob_ent() { return prob_ent; }

      std::size_t size() { return query_nodes.size(); }

      std::size_t get_num_nodes() { return query_nodes.size(); }
      std::size_t get_num_edges() { return query_edges.size(); }

      node_itr_type begin() { return query_nodes.begin(); }
      node_itr_type end() { return query_nodes.end(); }

      node_itr_type find(hash_type hash);

      void to_nodes(std::vector<glm_node_type>& nodes);
      void to_edges(std::vector<glm_edge_type>& edges);

      bool has_node(hash_type hash);
      bool has_edge(hash_type hash);

      void erase(std::vector<ind_type>& hashes);

      void set(hash_type hash, ind_type cnt, val_type weight);
      void set(qry_node_type& node);

      void add(hash_type hash, ind_type cnt, val_type weight);

      void add(qry_node_type& node);
      void add(qry_edge_type& edge);

      void clear();

      void normalise(bool check=false);
      void sort();

      void accumulate(query_result<model_type>& other);
      void intersect(query_result<model_type>& other);

    private:

      std::shared_ptr<model_type> model;

      std::string name, description;

      bool normalised, sorted;
      val_type sum;

      val_type prob_avg, prob_std, prob_ent;

      std::unordered_map<hash_type, ind_type> node_index;
      std::unordered_map<hash_type, ind_type> edge_index;

      std::vector<qry_node_type> query_nodes;
      std::vector<qry_edge_type> query_edges;
    };

    template<typename model_type>
    query_result<model_type>::query_result(std::shared_ptr<model_type> model):
      model(model),

      normalised(false),
      sorted(false),

      sum(0),

      prob_avg(0),
      prob_std(0),
      prob_ent(0),

      node_index({}),
      edge_index({}),

      query_nodes({}),
      query_edges({})
    {}

    template<typename model_type>
    nlohmann::json query_result<model_type>::to_json()
    {
      return to_json(256, 256, 0, 0);
    }

    template<typename model_type>
    nlohmann::json query_result<model_type>::to_json(cnt_type num_nodes,
                                                     cnt_type num_edges,
                                                     cnt_type ind_nodes,
                                                     cnt_type ind_edges)
    {
      this->normalise(true);

      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();

      nlohmann::json result = nlohmann::json::object();

      result["name"] = name;

      auto& rnodes = result["nodes"];
      auto& redges = result["edges"];

      rnodes = nlohmann::json::object();
      redges = nlohmann::json::object();

      {
        rnodes["headers"] = node_headers;
        rnodes["data"] = nlohmann::json::array();
      }

      {
        redges["headers"] = edge_headers;
        redges["data"] = nlohmann::json::array();
      }

      if(num_nodes>0)
        {
          cnt_type cnt=0, tot=0;

          base_node node;
          for(auto& qnode:query_nodes)
            {
              if(ind_nodes<=cnt and tot<num_nodes)
                {
                  auto row = nlohmann::json::array();

                  if(nodes.get(qnode.hash, node))
                    {
                      row.push_back(node.get_flvr());
                      row.push_back(node.get_name());
                      row.push_back(node.get_hash());

                      row.push_back(qnode.weight);
                      row.push_back(qnode.prob);
                      row.push_back(qnode.cumul);

                      row.push_back(node.get_text(nodes, false));
                      row.push_back(node.get_word_cnt());

                      rnodes["data"].push_back(row);
                    }
                  else
                    {
                      LOG_S(WARNING) << "could not find hash " << qnode.hash;
                    }

                  tot += 1;
                }

              cnt += 1;

              if(tot>=num_nodes)
                {
                  break;
                }
            }
        }

      if(num_edges>0)
        {
          cnt_type cnt=0, tot=0;

          base_edge edge;
          for(auto& qedge:query_edges)
            {
              if(ind_nodes<=cnt and tot<num_nodes)
                {
                  auto row = nlohmann::json::array();

                  if(edges.get(qedge.hash, edge))
                    {
                      row.push_back(edge.get_flvr());
                      row.push_back(edge.get_name());

                      row.push_back(edge.get_hash());
                      row.push_back(edge.get_hash_i());
                      row.push_back(edge.get_hash_j());
                      row.push_back(edge.get_prob());

                      row.push_back(qedge.prob);

                      redges["data"].push_back(row);
                    }
                  else
                    {
                      LOG_S(WARNING) << "could not find hash " << qedge.hash;
                    }

                  tot += 1;
                }

              cnt += 1;

              if(tot>=num_nodes)
                {
                  break;
                }
            }
        }

      return result;
    }

    template<typename model_type>
    void query_result<model_type>::show(std::size_t max)
    {
      this->normalise(true);

      auto& nodes = model->get_nodes();

      std::stringstream capt;
      capt << "name: " << name << ", " << "size: " << query_nodes.size();

      std::vector<std::string> header = { "type", "flavor",
                                          "count", "weight", "prob", "cumul",
                                          "text",
                                          "#-word", "#-sent", "#-text", "#-table", "#-doc"};

      std::vector<std::vector<std::string> > data={};

      base_node node;
      for(auto& qnode:query_nodes)
        {
          if(nodes.get(qnode.hash, node))
            {
              std::vector<std::string> row = { "node", node_names::to_name(node.get_flvr()),
                                               std::to_string(qnode.count),
                                               std::to_string(utils::round_value(qnode.weight)),
                                               std::to_string(utils::round_value(qnode.prob)),
                                               std::to_string(utils::round_value(qnode.cumul)),
                                               node.get_text(nodes, false),
                                               std::to_string(node.get_word_cnt()),
                                               std::to_string(node.get_sent_cnt()),
                                               std::to_string(node.get_text_cnt()),
					       std::to_string(node.get_tabl_cnt()),
					       std::to_string(node.get_fdoc_cnt()) };

              assert(row.size()==header.size());
              data.push_back(row);
            }
          else
            {
              LOG_S(WARNING) << "could not find hash " << qnode.hash;
            }

          if(data.size()>=max)
            {
              break;
            }
        }

      LOG_S(INFO) << utils::to_string(capt.str(), header, data);
    }

    template<typename model_type>
    typename query_result<model_type>::node_itr_type query_result<model_type>::find(hash_type hash)
    {
      auto itr_i = node_index.find(hash);
      if(itr_i==node_index.end())
        {
          return query_nodes.end();
        }

      ind_type ind = itr_i->second;
      node_itr_type itr_j = query_nodes.begin()+ind;

      assert(itr_j->hash==hash);
      return itr_j;
    }

    template<typename model_type>
    void query_result<model_type>::to_nodes(std::vector<glm_node_type>& new_nodes)
    {
      auto& model_nodes = model->get_nodes();

      new_nodes.clear();

      for(auto& _:this->query_nodes)
        {
          glm_node_type node;
          if(model_nodes.get(_.hash, node))
            {
              new_nodes.push_back(node);
            }
          else
            {
              LOG_S(ERROR) << "hash: " << _.hash << " undefined ... ";
            }
        }
    }

    template<typename model_type>
    bool query_result<model_type>::has_node(hash_type hash)
    {
      return (node_index.count(hash)>0);
    }

    template<typename model_type>
    bool query_result<model_type>::has_edge(hash_type hash)
    {
      return (edge_index.count(hash)>0);
    }

    template<typename model_type>
    void query_result<model_type>::erase(std::vector<ind_type>& hashes)
    {
      std::set<ind_type> indices={};
      for(auto hash:hashes)
        {
          auto itr_i = node_index.find(hash);

          if(itr_i!=node_index.end())
            {
              indices.insert(itr_i->second);
            }
        }

      // go backwards in the indices so you dont invalidate
      // the next indices after the first erase ...
      for(auto itr=indices.rbegin(); itr!=indices.rend(); itr++)
        {
          auto ind = *itr;
          query_nodes.erase(query_nodes.begin()+ind);
        }

      // recreate the node_index ...

      node_index.clear();
      for(ind_type ind=0; ind<query_nodes.size(); ind++)
        {
          auto hash = query_nodes.at(ind).hash;
          node_index[hash] = ind;
        }
    }

    template<typename model_type>
    void query_result<model_type>::set(hash_type hash, ind_type cnt, val_type weight)
    {
      qry_node_type node(hash, cnt, weight);
      this->set(node);
    }

    template<typename model_type>
    void query_result<model_type>::add(hash_type hash, ind_type cnt, val_type weight)
    {
      qry_node_type node(hash, cnt, weight);
      this->add(node);
    }

    template<typename model_type>
    void query_result<model_type>::set(qry_node_type& node)
    {
      auto itr_i = node_index.find(node.hash);

      if(itr_i==node_index.end())
        {
          node_index[node.hash] = query_nodes.size();
          query_nodes.push_back(node);
        }
      else
        {
          auto ind = node_index.at(node.hash);

          query_nodes.at(ind).count = node.count;
          query_nodes.at(ind).weight = node.weight;
        }
    }

    template<typename model_type>
    void query_result<model_type>::add(qry_node_type& node)
    {
      auto itr_i = node_index.find(node.hash);

      if(itr_i==node_index.end())
        {
          node_index[node.hash] = query_nodes.size();
          query_nodes.push_back(node);
        }
      else
        {
          auto ind = node_index.at(node.hash);

          query_nodes.at(ind).count += node.count;
          query_nodes.at(ind).weight += node.weight;
        }
    }

    template<typename model_type>
    void query_result<model_type>::add(qry_edge_type& edge)
    {
      auto itr_i = edge_index.find(edge.hash);

      if(itr_i==edge_index.end())
        {
          edge_index[edge.hash] = query_edges.size();
          query_edges.push_back(edge);
        }
      else
        {
          auto ind = edge_index.at(edge.hash);

          query_edges.at(ind).weight += edge.weight;
        }
    }

    template<typename model_type>
    void query_result<model_type>::clear()
    {
      normalised=false;
      sorted=false;

      sum=0;

      prob_avg=0;
      prob_std=0;
      prob_ent=0;

      node_index.clear();
      edge_index.clear();

      query_nodes.clear();
      query_edges.clear();
    }

    template<typename model_type>
    void query_result<model_type>::normalise(bool check)
    {
      if(check and normalised)
        {
          return;
        }

      val_type Z_inv=1.e-12, cumul=1.e-12;

      sum=0;
      for(auto& node:query_nodes)
        {
          sum += node.weight;
        }
      Z_inv=1.0/sum;

      for(auto& node:query_nodes)
        {
          node.prob = node.weight*Z_inv;
        }

      std::sort(query_nodes.begin(), query_nodes.end(),
                [](const qry_node_type& lhs,
                   const qry_node_type& rhs)
                {
                  return lhs.prob>rhs.prob;
                });

      while(query_nodes.size()>0 and
            query_nodes.back().prob<1.e-6)
        {
          query_nodes.pop_back();
        }

      if(query_nodes.size()==0)
        {
          return;
        }

      sum=0;
      for(auto& node:query_nodes)
        {
          sum += node.weight;
        }
      Z_inv=1.0/sum;

      cumul=0.0;
      for(auto& node:query_nodes)
        {
          node.prob = node.weight*Z_inv;

          cumul += node.prob;
          node.cumul = cumul;
        }

      prob_avg=1.0/(query_nodes.size());

      prob_std=0;
      prob_ent=0;

      if(query_nodes.size()>1)
        {
          for(auto& node:query_nodes)
            {
              prob_ent -= node.prob*std::log(node.prob);
              prob_std += (node.prob-prob_avg)*(node.prob-prob_avg);
            }

          prob_std = std::sqrt(prob_std)/(query_nodes.size()-1.0);
        }

      node_index.clear();
      for(auto& node:query_nodes)
        {
          std::size_t ind = node_index.size();
          node_index[node.hash] = ind;
        }

      normalised=true;
    }

    template<typename model_type>
    void query_result<model_type>::accumulate(query_result<model_type>& other)
    {
      //LOG_S(INFO) << __FUNCTION__;

      for(auto itr_i=other.begin(); itr_i!=other.end(); itr_i++)
        {
          auto itr_j = node_index.find(itr_i->hash);

          if(itr_j==node_index.end())
            {
              node_index[itr_i->hash] = query_nodes.size();
              query_nodes.push_back(*itr_i);
            }
          else
            {
              query_nodes.at(itr_j->second).weight += itr_i->weight;
            }
        }

      normalised=false;
    }

    template<typename model_type>
    void query_result<model_type>::intersect(query_result<model_type>& other)
    {
      for(auto itr_i=begin(); itr_i!=end(); itr_i++)
        {
          auto itr_j = other.find(itr_i->hash);

          if(itr_j==other.end())
            {
              node_index.erase(itr_i->hash);
              itr_i = query_nodes.erase(itr_i);
            }
          else
            {
              itr_i->count += itr_j->count;
            }
        }

      normalised=false;
    }

  }

}

#endif
