//-*-C++-*-

#ifndef ANDROMEDA_UTILS_REGEX_PCRE2_ITEM_H_
#define ANDROMEDA_UTILS_REGEX_PCRE2_ITEM_H_

namespace andromeda
{
  struct pcre2_group
  {
  public:
    
    typedef uint64_t                index_type;
    typedef std::array<uint64_t, 2> range_type;
    
  public:
    
    nlohmann::json to_json();
    void from_json(nlohmann::json& item);

  public:

    range_type rng;
    index_type group_index;

    std::string text, group_name;
  };

  nlohmann::json pcre2_group::to_json()
  {
    nlohmann::json item = nlohmann::json::object({});
    {
      item["text"] = text;

      item["group-name"] = group_name;
      item["group-index"] = group_index;
    }
    
    return item;
  }
  
  void pcre2_group::from_json(nlohmann::json& item)
  {
    text = item.value("text", text);

    rng = item.value("range", rng);
    
    group_name = item.value("group-name", group_name);
    group_index = item.value("group-index", group_index);
  }
  
  struct pcre2_item
  {
    typedef typename pcre2_group::index_type index_type;
    typedef typename pcre2_group::range_type range_type;
    
  public:
    
    nlohmann::json to_json();
    void from_json(nlohmann::json& item);

  public:
    
    range_type  rng;
    std::string text, type, subtype;
    
    std::vector<pcre2_group> groups;
  };

  nlohmann::json pcre2_item::to_json()
  {
    nlohmann::json item = nlohmann::json::object({});
    {
      item["text"] = text;
      item["range"] = rng;
      
      item["type"] = type;
      item["subtype"] = subtype;

      item["groups"] = nlohmann::json::array();
      for(auto& group:groups)
	{
	  item["groups"].push_back(group.to_json());
	}
    }
    
    return item;
  }
  
  void pcre2_item::from_json(nlohmann::json& item)
  {
    text = item.value("text", text);
    rng = item.value("range", rng);

    type = item.value("type", type);
    subtype = item.value("subtype", subtype);

    groups.clear();
    for(auto& _:item["groups"])
      {
	pcre2_group group;
	group.from_json(_);
	
	groups.push_back(group);
      }    
  }

}

#endif
