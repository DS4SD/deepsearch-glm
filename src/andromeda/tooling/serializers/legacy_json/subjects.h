//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_SERIALIZERS_LEGACY_JSON_SUBJECTS_H_
#define ANDROMEDA_TOOLING_SERIALIZERS_LEGACY_JSON_SUBJECTS_H_

#include <filesystem>
#include <set>

#include <andromeda/utils.h>
#include <andromeda/enums.h>
#include <andromeda/tooling/base_types.h>
#include <andromeda/tooling/structs.h>

namespace andromeda::serializers::legacy_json
{

  class subjects
  {
  public:

    static nlohmann::json to_json(subject<TEXT>& value,
                                  const std::set<std::string>& filters = {});

    static nlohmann::json to_json(subject<TABLE>& value,
                                  const std::set<std::string>& filters = {});

    static nlohmann::json to_json(subject<FIGURE>& value,
                                  const std::set<std::string>& filters = {});

    static nlohmann::json to_json(subject<DOCUMENT>& value,
                                  const std::set<std::string>& filters = {});

    static bool from_json(const nlohmann::json& data, subject<TEXT>& value);
    static bool from_json(const nlohmann::json& data, subject<TABLE>& value);
    static bool from_json(const nlohmann::json& data, subject<FIGURE>& value);
    static bool from_json(const nlohmann::json& data, subject<DOCUMENT>& value);

    static bool set_document_data(subject<DOCUMENT>& value,
                                  nlohmann::json& data,
                                  bool order_maintext);

    static bool set_document_data(subject<DOCUMENT>& value,
                                  const std::filesystem::path& filepath,
                                  nlohmann::json& data,
                                  bool order_maintext);
  };

  nlohmann::json subjects::to_json(subject<TEXT>& value,
                                   const std::set<std::string>& filters)
  {
    return value.to_json(filters);
  }

  nlohmann::json subjects::to_json(subject<TABLE>& value,
                                   const std::set<std::string>& filters)
  {
    return value.to_json(filters);
  }

  nlohmann::json subjects::to_json(subject<FIGURE>& value,
                                   const std::set<std::string>& filters)
  {
    return value.to_json(filters);
  }

  nlohmann::json subjects::to_json(subject<DOCUMENT>& value,
                                   const std::set<std::string>& filters)
  {
    return value.to_json(filters);
  }

  bool subjects::from_json(const nlohmann::json& data, subject<TEXT>& value)
  {
    return value.from_json(data);
  }

  bool subjects::from_json(const nlohmann::json& data, subject<TABLE>& value)
  {
    return value.from_json(data);
  }

  bool subjects::from_json(const nlohmann::json& data, subject<FIGURE>& value)
  {
    return value.from_json(data);
  }

  bool subjects::from_json(const nlohmann::json& data, subject<DOCUMENT>& value)
  {
    return value.from_json(data);
  }

  bool subjects::set_document_data(subject<DOCUMENT>& value,
                                   nlohmann::json& data,
                                   bool order_maintext)
  {
    return value.set_data(data, order_maintext);
  }

  bool subjects::set_document_data(subject<DOCUMENT>& value,
                                   const std::filesystem::path& filepath,
                                   nlohmann::json& data,
                                   bool order_maintext)
  {
    return value.set_data(filepath, data, order_maintext);
  }

}

#endif
