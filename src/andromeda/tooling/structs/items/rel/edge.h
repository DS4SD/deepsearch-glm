//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_REL_EDGE_H_
#define ANDROMEDA_STRUCTS_ITEMS_REL_EDGE_H_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace andromeda
{
  class base_graph_edge: public base_types
  {
  public:

    const static inline std::vector<std::string> HEADERS =
      {"hash", "flvr", "name", "hash_i", "hash_j", "count", "probability"};

    base_graph_edge() = default;

    base_graph_edge(std::string name,
                    hash_type hash_i,
                    hash_type hash_j,
                    cnt_type count=1,
                    val_type probability=0.0):
      flvr(to_flvr(name)),
      name(std::move(name)),
      hash_i(hash_i),
      hash_j(hash_j),
      count(count),
      probability(probability)
    {
      initialise_hash();
    }

    static std::vector<std::string> headers() { return HEADERS; }

    static bool update(flvr_type flvr, const std::string& edge_name)
    {
      auto itr = name_to_flvr.find(edge_name);
      if(itr!=name_to_flvr.end())
        {
          return itr->second==flvr;
        }

      std::scoped_lock lock(mtx);
      name_to_flvr.insert({edge_name, flvr});
      flvr_to_name.insert({flvr, edge_name});
      return true;
    }

    static flvr_type to_flvr(const std::string& edge_name)
    {
      auto itr = name_to_flvr.find(edge_name);
      if(itr!=name_to_flvr.end())
        {
          return itr->second;
        }

      std::scoped_lock lock(mtx);
      flvr_type flvr = next_custom_flvr++;
      name_to_flvr.insert({edge_name, flvr});
      flvr_to_name.insert({flvr, edge_name});
      return flvr;
    }

    static std::string to_name(flvr_type flvr)
    {
      auto itr = flvr_to_name.find(flvr);
      if(itr!=flvr_to_name.end())
        {
          return itr->second;
        }
      return "unknown";
    }

    hash_type get_hash() const { return hash; }
    flvr_type get_flvr() const { return flvr; }
    const std::string& get_name() const { return name; }
    hash_type get_hash_i() const { return hash_i; }
    hash_type get_hash_j() const { return hash_j; }
    cnt_type get_count() const { return count; }
    val_type get_probability() const { return probability; }

    nlohmann::json to_json_row() const
    {
      return nlohmann::json::array({
          hash, flvr, name, hash_i, hash_j, count, probability});
    }

    std::vector<std::string> to_row() const
    {
      return {
        std::to_string(hash),
        std::to_string(flvr),
        name,
        std::to_string(hash_i),
        std::to_string(hash_j),
        std::to_string(count),
        std::to_string(probability)
      };
    }

    bool from_json_row(const nlohmann::json& row)
    {
      if((not row.is_array()) or row.size()!=HEADERS.size())
        {
          return false;
        }

      hash = row.at(0).get<hash_type>();
      flvr = row.at(1).get<flvr_type>();
      name = row.at(2).get<std::string>();
      hash_i = row.at(3).get<hash_type>();
      hash_j = row.at(4).get<hash_type>();
      count = row.at(5).get<cnt_type>();
      probability = row.at(6).get<val_type>();

      update(flvr, name);
      return true;
    }

  private:

    void initialise_hash()
    {
      hash = flvr;
      hash = utils::murmerhash3(hash);
      hash = utils::combine_hash(hash, hash_i);
      hash = utils::combine_hash(hash, hash_j);
    }

  private:

    hash_type hash = 0;
    flvr_type flvr = 0;
    std::string name;
    hash_type hash_i = 0;
    hash_type hash_j = 0;
    cnt_type count = 0;
    val_type probability = 0.0;

    inline static std::mutex mtx;
    inline static flvr_type next_custom_flvr = 256;
    inline static std::unordered_map<std::string, flvr_type> name_to_flvr = {
      {"tax-dn", 32},
      {"tax-up", 33},
      {"to-instances", 136},
      {"to-entities", 137}
    };
    inline static std::unordered_map<flvr_type, std::string> flvr_to_name = {
      {32, "tax-dn"},
      {33, "tax-up"},
      {136, "to-instances"},
      {137, "to-entities"}
    };
  };
}

#endif
