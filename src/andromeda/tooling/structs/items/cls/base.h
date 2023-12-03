//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_CLS_BASE_H_
#define ANDROMEDA_STRUCTS_ITEMS_CLS_BASE_H_

namespace andromeda
{
  class base_property: public base_types
  {
  public:

    const static inline std::string UNDEF = "__undef__";

    const static inline std::vector<std::string> HEADERS
    = { "type",
        "subj_hash", "subj_name", "subj_path",
        "label", "confidence"};

  public:

    base_property();

    base_property(hash_type subj_hash, // hash of the subject from which the entity comes
                  subject_name subj_name,
                  std::string subj_path,
                  model_name model,
                  std::string label,
                  val_type conf);

    hash_type get_subj_hash() const { return subj_hash; }
    subject_name get_subj_name() const { return subj_name; }
    std::string get_subj_path() const { return subj_path; }

    bool is_type(const std::string name) const { return (name==to_key(model)); }
    bool is_label(const std::string label) const { return (label==this->label); }

    bool is_model(const model_name name) const { return (name==model); }

    model_name get_model() const { return this->model; }
    std::string get_type() const { return to_key(this->model); }

    std::string get_label() const { return this->label; }
    float get_conf() const { return this->conf; }

    void set_label(const std::string label) { this->label=label; }
    void set_conf(const float conf) { this->conf = conf; }

    std::vector<std::string> to_row();

    nlohmann::json to_json();

    nlohmann::json to_json_row();
    bool from_json_row(const nlohmann::json& row);

    friend bool operator==(const base_property& lhs, const base_property& rhs);
    friend bool operator<(const base_property& lhs, const base_property& rhs);

  private:

    hash_type subj_hash; // hash of the subject from which the entity comes
    subject_name subj_name;
    std::string subj_path;

    model_name model;
    std::string label;
    val_type conf;
  };

  base_property::base_property():
    subj_hash(-1),
    subj_name(TEXT),
    subj_path("#"),

    model(NULL_MODEL),
    label("UNDEF"),
    conf(0.0)
  {}

  base_property::base_property(hash_type subj_hash,
                               subject_name subj_name,
                               std::string subj_path,
                               model_name model,
                               std::string label,
                               val_type conf):
    subj_hash(subj_hash),
    subj_name(subj_name),
    subj_path(subj_path),

    model(model),
    label(label),
    conf(conf)
  {}

  std::vector<std::string> base_property::to_row()
  {
    std::vector<std::string> row = { to_key(model),
                                     std::to_string(subj_hash), to_string(subj_name), subj_path,
                                     label, std::to_string(utils::round_conf(conf)) };
    assert(row.size()==HEADERS.size());

    return row;
  }

  nlohmann::json base_property::to_json()
  {
    nlohmann::json result = nlohmann::json::object();
    {
      result["type"] = to_key(model);

      result["subj_hash"] = subj_hash;
      result["subj_name"] = to_string(subj_name);
      result["subj_path"] = subj_path;

      result["label"] = label;
      result["confidence"] = utils::round_conf(conf);
    }

    return result;
  }

  nlohmann::json base_property::to_json_row()
  {
    nlohmann::json row = nlohmann::json::array({
        to_key(model),
        subj_hash, to_string(subj_name), subj_path,
        label, utils::round_conf(conf)});
    assert(row.size()==HEADERS.size());

    return row;
  }

  bool base_property::from_json_row(const nlohmann::json& row)
  {
    if(row.size()>=HEADERS.size())
      {
        model = to_modelname(row[0].get<std::string>());

        subj_hash = row[1].get<hash_type>();
        subj_name = to_subject_name(row[2].get<std::string>());
        subj_path = row[3].get<std::string>();

        label = row[4].get<std::string>();
        conf = row[5].get<float>();

        return true;
      }

    return false;
  }

  bool operator==(const base_property& lhs,
                  const base_property& rhs)
  {
    return ((lhs.subj_name==rhs.subj_name) and
            (lhs.subj_path==rhs.subj_path) and
            (lhs.model==rhs.model));
  }

  bool operator<(const base_property& lhs,
                 const base_property& rhs)
  {
    if(lhs.subj_name==rhs.subj_name)
      {
        if(lhs.subj_path==rhs.subj_path)
          {
            if(lhs.model==rhs.model)
              {
                return lhs.conf>rhs.conf;
              }
            else
              {
                return (lhs.model<rhs.model);
              }
          }
        else
          {
            /*
              auto lhs_parts = utils::split(lhs.subj_path, "/");
              auto rhs_parts = utils::split(rhs.subj_path, "/");

              if(lhs_parts.size()==rhs_parts.size())
              {
              if(lhs_parts.size()==3)
              {
              return std::stoi(lhs_parts.at(2))<std::stoi(rhs_parts.at(2));
              }
              else
              {
              return (lhs.subj_path<rhs.subj_path);
              }
              }
              else
              {
              return lhs_parts.size()<rhs_parts.size();
              }
            */

            return utils::compare_paths(lhs.subj_path, rhs.subj_path);
          }
      }
    else
      {
        return (lhs.subj_name<rhs.subj_name);
      }
  }

}

#endif
