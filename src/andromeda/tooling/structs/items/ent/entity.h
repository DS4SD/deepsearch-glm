//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_ENTITY_H_
#define ANDROMEDA_STRUCTS_ITEMS_ENTITY_H_

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>

namespace andromeda
{
  class base_entity: public base_types
  {
  public:

    const static inline std::vector<std::string> HEADERS =
      {"type", "subtype", "name", "entity_kind", "hash", "count", "parent", "parent_hash"};

    base_entity() = default;

    base_entity(std::string type,
                std::string subtype,
                std::string name,
                std::string entity_kind,
                cnt_type count,
                std::string parent=""):
      type(std::move(type)),
      subtype(std::move(subtype)),
      name(std::move(name)),
      entity_kind(std::move(entity_kind)),
      count(count),
      parent(std::move(parent))
    {
      initialise_hashes();
    }

    static std::vector<std::string> headers() { return HEADERS; }

    const std::string& get_type() const { return type; }
    const std::string& get_subtype() const { return subtype; }
    const std::string& get_name() const { return name; }
    const std::string& get_entity_kind() const { return entity_kind; }
    hash_type get_hash() const { return hash; }
    cnt_type get_count() const { return count; }
    const std::string& get_parent() const { return parent; }
    hash_type get_parent_hash() const { return parent_hash; }

    nlohmann::json to_json_row() const
    {
      return nlohmann::json::array({
          type, subtype, name, entity_kind, hash, count, parent, parent_hash});
    }

    std::vector<std::string> to_row() const
    {
      return {
        type,
        subtype,
        name,
        entity_kind,
        std::to_string(hash),
        std::to_string(count),
        parent,
        std::to_string(parent_hash)
      };
    }

    bool from_json_row(const nlohmann::json& row)
    {
      if((not row.is_array()) or row.size()!=HEADERS.size())
        {
          return false;
        }

      type = row.at(0).get<std::string>();
      subtype = row.at(1).get<std::string>();
      name = row.at(2).get<std::string>();
      entity_kind = row.at(3).get<std::string>();
      hash = row.at(4).get<hash_type>();
      count = row.at(5).get<cnt_type>();
      parent = row.at(6).get<std::string>();
      parent_hash = row.at(7).get<hash_type>();
      return true;
    }

  private:

    void initialise_hashes()
    {
      hash = utils::to_reproducible_hash(name);
      parent_hash = parent.empty()? 0:utils::to_reproducible_hash(parent);
    }

  private:

    std::string type;
    std::string subtype;
    std::string name;
    std::string entity_kind;
    hash_type hash = 0;
    cnt_type count = 0;
    std::string parent;
    hash_type parent_hash = 0;
  };

  inline std::vector<std::string> split_entity_tokens(const std::string& text)
  {
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string token;
    while(ss >> token)
      {
        tokens.push_back(token);
      }

    return tokens;
  }

  inline std::string join_entity_tokens(const std::vector<std::string>& tokens,
                                        std::size_t begin)
  {
    std::ostringstream oss;
    for(std::size_t i=begin; i<tokens.size(); i++)
      {
        if(i>begin)
          {
            oss << " ";
          }

        oss << tokens.at(i);
      }

    return oss.str();
  }

  inline std::string entity_parent_name(const std::string& name)
  {
    const auto tokens = split_entity_tokens(name);
    if(tokens.size()<2)
      {
        return "";
      }

    return join_entity_tokens(tokens, 1);
  }

  inline std::vector<base_entity> compute_entities_from_instances(
    const std::vector<base_instance>& instances)
  {
    using key_type = std::tuple<std::string, std::string, std::string>;

    std::map<key_type, base_types::cnt_type> counts;
    std::set<key_type> exact_keys;

    for(const auto& inst:instances)
      {
        const auto tokens = split_entity_tokens(inst.get_name());
        if(tokens.empty())
          {
            continue;
          }

        for(std::size_t i=0; i<tokens.size(); i++)
          {
            counts[{inst.get_type(), inst.get_subtype(), join_entity_tokens(tokens, i)}] += 1;
          }

        exact_keys.insert({inst.get_type(), inst.get_subtype(), inst.get_name()});
      }

    std::vector<base_entity> result;
    result.reserve(counts.size());

    for(const auto& entry:counts)
      {
        const auto& key = entry.first;
        const auto& count = entry.second;
        const std::string type = std::get<0>(key);
        const std::string subtype = std::get<1>(key);
        const std::string name = std::get<2>(key);
        const std::string kind = exact_keys.count(key)>0? "exact":"derived";

        std::string parent = entity_parent_name(name);
        if(parent.empty() or counts.count({type, subtype, parent})==0)
          {
            parent = "";
          }

        result.emplace_back(type, subtype, name, kind, count, parent);
      }

    std::sort(result.begin(), result.end(), [](const base_entity& lhs,
                                               const base_entity& rhs)
    {
      if(lhs.get_type()!=rhs.get_type())
        {
          return lhs.get_type()<rhs.get_type();
        }
      if(lhs.get_subtype()!=rhs.get_subtype())
        {
          return lhs.get_subtype()<rhs.get_subtype();
        }
      if(lhs.get_name()!=rhs.get_name())
        {
          return lhs.get_name()<rhs.get_name();
        }
      return lhs.get_entity_kind()<rhs.get_entity_kind();
    });

    return result;
  }

}

#endif
