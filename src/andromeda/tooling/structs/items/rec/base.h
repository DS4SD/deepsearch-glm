//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_RECORD_H_
#define ANDROMEDA_STRUCTS_ITEMS_RECORD_H_

namespace andromeda
{
  class base_record: public base_types
  {
  public:

    static std::vector<std::string> headers();

    base_record();

    nlohmann::json to_json() const;
    bool from_json(const nlohmann::json& row);

  private:

  };

  base_record::base_record()
  {}

  nlohmann::json base_record::to_json() const
  {
    nlohmann::json result;
    return result;
  }

  bool base_record::from_json(const nlohmann::json& row)
  {
    return false;
  }

}

#endif
