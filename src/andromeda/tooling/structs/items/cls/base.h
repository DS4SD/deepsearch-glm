//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_CLS_BASE_H_
#define ANDROMEDA_STRUCTS_ITEMS_CLS_BASE_H_

namespace andromeda
{
  class base_property: public base_types
  {
  public:

    const static inline std::string UNDEF = "__undef__";
    const static inline std::vector<std::string> HEADERS = { "type", "label", "confidence"};
    
  public:

    base_property();

    base_property(std::string type,
		  std::string name,
		  val_type    conf);

    std::string get_type() { return this->type; }
    std::string get_name() { return this->name; }

    float get_conf() { return this->conf; }

    void set_name(const std::string& name) { this->name = name; }
    void set_conf(const float& conf) { this->conf = conf; }
    
    std::vector<std::string> to_row();
    
    nlohmann::json to_json();

    nlohmann::json to_json_row();
    bool from_json_row(const nlohmann::json& row);

    friend bool operator<(const base_property& lhs, const base_property& rhs);
    
  private:

    std::string type;
    std::string name;
    val_type    conf;
  };
  
  base_property::base_property():
    type(UNDEF),
    name(UNDEF),
    conf(0.0)
  {}
    
  base_property::base_property(std::string type,
			       std::string name,
			       val_type    conf):
    type(type),
    name(name),
    conf(conf)
  {}

  std::vector<std::string> base_property::to_row()
  {
    std::vector<std::string> row = { type, name, std::to_string(conf) };
    assert(row.size()==HEADERS.size());
    
    return row;
  }
  
  nlohmann::json base_property::to_json()
  {
    nlohmann::json result = nlohmann::json::object();
    {
      result["type"] = type;
      result["name"] = name;
      result["confidence"] = conf;
    }
    
    return result;
  }

  nlohmann::json base_property::to_json_row()
  {
    nlohmann::json row = nlohmann::json::array({ type, name, conf});
    assert(row.size()==HEADERS.size());
    
    return row;
  }
  
  bool base_property::from_json_row(const nlohmann::json& row)
  {
    if(row.size()>=HEADERS.size())
      {
	type = row[0].get<std::string>();
	name = row[1].get<std::string>();
	conf = row[2].get<float>();
	
	return true;
      }
    
    return false;
  }    

  bool operator<(const base_property& lhs, const base_property& rhs)
  {
    if(lhs.type==rhs.type)
      {
	return lhs.conf>rhs.conf;
      }
    else
      {
	return (lhs.type<rhs.type);
      }
  }
  
}

#endif
