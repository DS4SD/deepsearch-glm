//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_TOPOLOGY_H_
#define ANDROMEDA_MODELS_GLM_TOPOLOGY_H_

namespace andromeda
{

  namespace glm
  {

    class glm_topology: public base_types
    {
    public:

      typedef std::pair<short, std::size_t> key_type;

    public:

      glm_topology();
      ~glm_topology();

      void to_shell();

      std::ofstream& to_txt(std::ofstream& ofs);

      nlohmann::json to_json();

      bool from_json(nlohmann::json& config);

      void clear();

      void initialise();

      template<typename model_type>
      void compute(model_type& model);

    private:

      template<typename tmp_type>
      std::string to_string(tmp_type val);

      template<typename model_type>
      std::size_t compute_nodes_statistics(model_type& model);

      template<typename model_type>
      std::size_t compute_edges_statistics(model_type& model);

      void initialise(short type, std::map<key_type, std::size_t>& stats);

      nlohmann::json to_json(std::map<short, std::string>& data);

      nlohmann::json to_json(std::map<short, std::size_t>& data,
                             std::map<short, std::string>& types);

      nlohmann::json to_json(std::map<key_type, std::size_t>& data,
                             std::map<short, std::string>& types);

      void from_json(nlohmann::json& obj, std::map<short, std::string>& data);
      void from_json(nlohmann::json& obj, std::map<short, std::size_t>& data);
      void from_json(nlohmann::json& obj, std::map<key_type, std::size_t>& data);

      void update_statistics(short type, std::size_t value,
                             std::map<key_type, std::size_t>& stats);

    private:

      const static inline std::size_t MAX_EXP=20;
      std::vector<std::size_t> lower_bound;

      std::map<short, std::string> node_flavors;
      std::map<short, std::string> edge_flavors;

      std::map<short, std::size_t> node_counts;
      std::map<short, std::size_t> edge_counts;

      std::map<key_type, std::size_t> node_word_stats;
      std::map<key_type, std::size_t> node_sent_stats;
      std::map<key_type, std::size_t> node_text_stats;

      std::map<key_type, std::size_t> edge_count_stats;

      std::vector<std::string> nodes_header, edges_header;//, paths_header;
      std::vector<std::vector<std::string> > nodes_table, edges_table;//, paths_table;
    };

    glm_topology::glm_topology()
    {
      {
        lower_bound = {0};
        for(std::size_t e=0; e<=MAX_EXP; e++)
          {
            std::size_t val = std::pow(2,e);
            lower_bound.push_back(val);
          }
      }

      initialise();
    }

    glm_topology::~glm_topology()
    {}

    template<typename tmp_type>
    std::string glm_topology::to_string(tmp_type val)
    {
      std::stringstream ss;
      ss << std::scientific << std::setprecision(1);

      if(val==0)
        {
          ss << val;
        }
      else
        {
          ss << double(val);
        }

      return ss.str();
    }

    void glm_topology::to_shell()
    {
      LOG_S(INFO);

      {
        nodes_header = {"flavor", "node-name", "count"};

        for(auto lb:lower_bound)
          {
            nodes_header.push_back(std::to_string(lb));
          }

        std::vector<std::size_t> total(nodes_header.size(), 0);

        nodes_table={};
        for(auto itr=node_counts.begin(); itr!=node_counts.end(); itr++)
          {
            std::size_t ind=2;
            total.at(ind++) += itr->second;

            std::vector<std::string> row = { std::to_string(itr->first),
                                             node_flavors.at(itr->first),
                                             std::to_string(itr->second)};

            for(auto lb:lower_bound)
              {
                key_type key(itr->first, lb);
		
                auto itr = node_text_stats.find(key);
                row.push_back(to_string(itr->second));

                total.at(ind++) += itr->second;
              }

            nodes_table.push_back(row);
          }

        {
          std::vector<std::string>
            row_0(nodes_header.size(), ""),
            row_1(nodes_header.size(), ""),
            cum_0(nodes_header.size(), ""),
            cum_1(nodes_header.size(), "");

          row_0.at(0) = "total (cnt)";
          row_1.at(0) = "total (%)";
          cum_0.at(0) = "cumul (cnt)";
          cum_1.at(0) = "cumul (%)";

          row_0.at(2) = to_string(total.at(2));
          row_1.at(2) = to_string(1.0);
          cum_0.at(2) = to_string(total.at(2));
          cum_1.at(2) = to_string(1.0);

          std::size_t cumul=0;
          for(std::size_t i=3; i<total.size(); i++)
            {
              cumul += total.at(i);

              double cnt_percent = double(total.at(i))/double(total.at(2));
              double cum_percent = double(cumul)/double(total.at(2));

              row_0.at(i) = to_string(total.at(i));
              row_1.at(i) = to_string(cnt_percent);
              cum_0.at(i) = to_string(cumul);
              cum_1.at(i) = to_string(cum_percent);
            }

          nodes_table.push_back(row_0);
          nodes_table.push_back(row_1);
          nodes_table.push_back(cum_1);
        }

        LOG_S(INFO) << "node table: \n" << utils::to_string(nodes_header, nodes_table, 21);
      }

      {
        edges_header = {"flavor", "edge-name", "count"};

        for(auto lb:lower_bound)
          {
            edges_header.push_back(std::to_string(lb));
          }

        std::vector<std::size_t> total(edges_header.size(), 0);

        edges_table={};
        for(auto itr=edge_counts.begin(); itr!=edge_counts.end(); itr++)
          {
            std::size_t ind=2;
            total.at(ind++) += itr->second;

            std::vector<std::string> row = { std::to_string(itr->first),
                                             edge_flavors.at(itr->first),
                                             to_string(itr->second)};

            for(auto lb:lower_bound)
              {
                key_type key(itr->first, lb);

                auto itr = edge_count_stats.find(key);
                row.push_back(to_string(itr->second));

                total.at(ind++) += itr->second;
              }

            edges_table.push_back(row);
          }

        {
          std::vector<std::string>
            row_0(edges_header.size(), ""),
            row_1(edges_header.size(), ""),
            cum_0(edges_header.size(), ""),
            cum_1(edges_header.size(), "");

          row_0.at(0) = "total (cnt)";
          row_1.at(0) = "total (%)";
          cum_0.at(0) = "cumul (cnt)";
          cum_1.at(0) = "cumul (%)";

          row_0.at(2) = to_string(total.at(2));
          row_1.at(2) = to_string(1.0);
          cum_0.at(2) = to_string(total.at(2));
          cum_1.at(2) = to_string(1.0);

          std::size_t cumul=0;
          for(std::size_t i=3; i<total.size(); i++)
            {
              cumul += total.at(i);

              double cnt_percent = double(total.at(i))/double(total.at(2));
              double cum_percent = double(cumul)/double(total.at(2));

              row_0.at(i) = to_string(total.at(i));
              row_1.at(i) = to_string(cnt_percent);
              cum_0.at(i) = to_string(cumul);
              cum_1.at(i) = to_string(cum_percent);
            }

          edges_table.push_back(row_0);
          edges_table.push_back(row_1);
          //edges_table.push_back(cum_0);
          edges_table.push_back(cum_1);
        }

        LOG_S(INFO) << "edge table: \n" << utils::to_string(edges_header, edges_table, 21);
      }
    }

    std::ofstream& glm_topology::to_txt(std::ofstream& ofs)
    {
      ofs << "node-count table: \n" << utils::to_string(nodes_header, nodes_table);
      ofs << "edge-count table: \n" << utils::to_string(edges_header, edges_table);

      return ofs;
    }

    nlohmann::json glm_topology::to_json()
    {
      nlohmann::json result;

      result["node-flavors"] = to_json(node_flavors);
      result["edge-flavors"] = to_json(edge_flavors);

      result["node-count"] = to_json(node_counts, node_flavors);
      result["edge-count"] = to_json(edge_counts, edge_flavors);

      result["node-word-stats"] = to_json(node_word_stats, node_flavors);
      result["node-sent-stats"] = to_json(node_sent_stats, node_flavors);
      result["node-text-stats"] = to_json(node_text_stats, node_flavors);

      result["edge-count-stats"] = to_json(edge_count_stats, edge_flavors);

      return result;
    }

    bool glm_topology::from_json(nlohmann::json& config)
    {
      //LOG_S(INFO);// << config.dump(2);

      from_json(config["node-flavors"], node_flavors);
      from_json(config["edge-flavors"], edge_flavors);

      from_json(config["node-count"], node_counts);
      from_json(config["edge-count"], edge_counts);

      from_json(config["node-word-stats"], node_word_stats);
      from_json(config["node-sent-stats"], node_sent_stats);
      from_json(config["node-text-stats"], node_text_stats);

      from_json(config["edge-count-stats"], edge_count_stats);

      //LOG_S(INFO) << "node_counts: " << node_counts.size();
      //LOG_S(INFO) << "edge_counts: " << edge_counts.size();
      //to_shell();

      return true;
    }

    nlohmann::json glm_topology::to_json(std::map<short, std::string>& data)
    {
      nlohmann::json result;

      std::vector<std::string> header={"flavor", "name"};
      result["header"] = header;

      auto& rows = result["data"];
      for(auto itr=data.begin(); itr!=data.end(); itr++)
        {
          nlohmann::json row;
          row.push_back(itr->first);
          row.push_back(itr->second);

          rows.push_back(row);
        }

      return result;
    }

    void glm_topology::from_json(nlohmann::json& obj, std::map<short, std::string>& data)
    {
      auto& rows = obj["data"];
      for(auto& row:rows)
        {
          short       flavor = row[0].get<short>();
          std::string name = row[1].get<std::string>();

          data[flavor] = name;
        }
    }

    nlohmann::json glm_topology::to_json(std::map<short, std::size_t>& data,
                                         std::map<short, std::string>& flavors)
    {
      nlohmann::json result;

      std::vector<std::string> header={"flavor", "name", "count"};
      result["header"] = header;

      auto& rows = result["data"];
      for(auto itr=data.begin(); itr!=data.end(); itr++)
        {
          nlohmann::json row;
          row.push_back(itr->first);
          row.push_back(flavors.at(itr->first));
          row.push_back(itr->second);

          rows.push_back(row);
        }

      return result;
    }

    void glm_topology::from_json(nlohmann::json& obj, std::map<short, std::size_t>& data)
    {
      auto& rows = obj["data"];
      for(auto& row:rows)
        {
          short       flavor  = row[0].get<short>();
          std::size_t count = row[2].get<std::size_t>();

          data[flavor] = count;
        }
    }

    nlohmann::json glm_topology::to_json(std::map<key_type, std::size_t>& data,
                                         std::map<short, std::string>& flavors)
    {
      nlohmann::json result;

      std::vector<std::string> header={"flavor", "name", "lower-bound", "count"};
      result["header"] = header;

      auto& rows = result["data"];
      for(auto itr=data.begin(); itr!=data.end(); itr++)
        {
          nlohmann::json row;

          row.push_back((itr->first).first);
          row.push_back(flavors.at((itr->first).first));
          row.push_back((itr->first).second);
          row.push_back(itr->second);

          rows.push_back(row);
        }

      return result;
    }

    void glm_topology::from_json(nlohmann::json& obj, std::map<key_type, std::size_t>& data)
    {
      auto& rows = obj["data"];
      for(auto& row:rows)
        {
          short flavor = row[0].get<short>();
          std::size_t lowb = row[2].get<std::size_t>();
          std::size_t count = row[3].get<std::size_t>();

          key_type key(flavor, lowb);
          data[key] = count;
        }
    }

    void glm_topology::clear()
    {
      node_flavors.clear();
      edge_flavors.clear();

      node_counts.clear();
      edge_counts.clear();

      node_word_stats.clear();
      node_sent_stats.clear();
      node_text_stats.clear();

      edge_count_stats.clear();
    }

    void glm_topology::initialise()
    {
      clear();

      {
        //auto& names = node_names::to_name;
        for(auto itr=node_names::begin(); itr!=node_names::end(); itr++)
          {
            node_flavors[itr->first] = itr->second;
            node_counts[itr->first] = 0;

            initialise(itr->first, node_word_stats);
            initialise(itr->first, node_sent_stats);
            initialise(itr->first, node_text_stats);
          }
      }

      {
        //auto& names = edge_names::flvr_to_name_map;
        for(auto itr=edge_names::begin(); itr!=edge_names::end(); itr++)
          {
            edge_flavors[itr->first] = itr->second;
            edge_counts[itr->first] = 0;

            initialise(itr->first, edge_count_stats);
          }
      }
    }

    void glm_topology::initialise(short flavor, std::map<key_type, std::size_t>& stats)
    {
      for(auto lb:lower_bound)
        {
          key_type key(flavor, lb);
          stats[key] = 0;
        }
    }

    template<typename model_type>
    void glm_topology::compute(model_type& model)
    {
      LOG_S(INFO) << "computing topology ...";

      initialise();

      compute_nodes_statistics(model);

      //std::size_t max_text_cnt = compute_nodes_statistics(model);
      //LOG_S(INFO) << "max text-count: " << max_text_cnt;

      compute_edges_statistics(model);

      to_shell();
    }

    template<typename model_type>
    std::size_t glm_topology::compute_nodes_statistics(model_type& model)
    {
      LOG_S(INFO) << "computing nodes-topology ...";

      cnt_type max_text_cnt=0;

      auto& nodes = model.get_nodes();
      for(auto& flvr_coll:nodes)
        {
          for(auto& node:flvr_coll.second)
            {
	      cnt_type cnt = node.get_text_cnt()+node.get_tabl_cnt()+node.get_fdoc_cnt();	      

              max_text_cnt = std::max(max_text_cnt, cnt);

              {
                node_counts.at(node.get_flvr()) += 1;
              }

              {
                update_statistics(node.get_flvr(), node.get_word_cnt(), node_word_stats);
                update_statistics(node.get_flvr(), node.get_sent_cnt(), node_sent_stats);
                //update_statistics(node.get_flvr(), node.get_text_cnt(), node_text_stats);
		update_statistics(node.get_flvr(), cnt, node_text_stats);
              }
            }
        }

      return max_text_cnt;
    }

    template<typename model_type>
    std::size_t glm_topology::compute_edges_statistics(model_type& model)
    {
      LOG_S(INFO) << "computing edges-topology ...";

      cnt_type max_cnt=0;

      auto& edges = model.get_edges();
      for(auto& flvr_coll:edges)
        {
          for(auto& edge:flvr_coll.second)
            {
              max_cnt = std::max(max_cnt, edge.get_count());

              if(edge_counts.count(edge.get_flvr())==1)
                {
                  edge_counts.at(edge.get_flvr()) += 1;
                }
              else
                {
                  LOG_S(WARNING) << "new edge-flavor: " << edge.get_flvr();

                  edge_counts[edge.get_flvr()] = 1;
                  initialise(edge.get_flvr(), edge_count_stats);
                }

              update_statistics(edge.get_flvr(), edge.get_count(), edge_count_stats);
            }
        }

      return max_cnt;
    }

    /*
      template<typename model_type>
      std::size_t glm_topology::compute_paths_statistics(model_type& model)
      {
      LOG_S(INFO) << "computing paths-topology ...";

      cnt_type max_cnt=0;

      auto& paths = model.get_nodes();
      for(auto& path:paths)
      {
      auto& node = path.second;
      max_cnt = std::max(max_cnt, node.count());

      if(path_counts.count(node.get_flvr())==1)
      {
      path_counts.at(node.get_flvr()) += 1;
      }
      else
      {
      LOG_S(WARNING) << "new path-flavor: " << node.get_flvr();

      path_counts[node.get_flvr()] = 1;
      initialise(node.get_flvr(), path_count_stats);
      }

      update_statistics(node.get_flvr(), node.count(), path_count_stats);
      }

      return max_cnt;
      }
    */

    void glm_topology::update_statistics(short flavor, std::size_t value,
                                         std::map<key_type, std::size_t>& stats)
    {
      key_type key(flavor, value);

      auto itr = stats.lower_bound(key);
      auto prv = itr; prv--;

      if(itr==stats.end() and prv!=stats.end() and
         (prv->first).first == flavor and
         (prv->first).second <= value)
        {
          prv->second += 1;
        }
      else if((itr->first).first == flavor and
              (itr->first).second == value)
        {
          itr->second += 1;
        }
      else if((prv->first).first == flavor and
              (itr->first).first == flavor)
        {
          if((prv->first).second <= value and
             value < (itr->first).second)
            {
              itr->second += 1;
            }
          else
            {
              LOG_S(WARNING) << (prv->first).second << "\t"
                             << value << "\t"
                             << (itr->first).second << "\t";
            }
        }
      else if((prv->first).first == flavor and
              (prv->first).second <= value)
        {
          prv->second += 1;
        }
      else
        {
          LOG_S(ERROR) << "not the right flavor (node): "
                       << flavor << ", " << value << " => (itr): "
                       << (itr->first).first << ", "
                       << (itr->first).second;
        }
    }

  }

}

#endif
