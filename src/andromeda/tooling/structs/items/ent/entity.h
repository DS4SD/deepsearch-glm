//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_ENTITY_H_
#define ANDROMEDA_STRUCTS_ITEMS_ENTITY_H_

namespace andromeda
{
  class base_entity: public base_types
  {
  public:

  public:

    static std::vector<std::string> headers();

    base_entity();

    base_entity(hash_type ehash,
		std::string name,
		hash_type instance);

    nlohmann::json to_json_row() const;
    bool from_json_row(const nlohmann::json& row);

    nlohmann::json to_json_row(subject_name subj) const;

    std::vector<std::string> to_row(std::size_t col_width);

    std::vector<std::string> to_row(std::string& text,
                                    std::size_t name_width);

    void add_instance(const hash_type& instance);
    
  private:

    void initialise_hashes();
    
  private:

    hash_type ehash;
    std::string name;

    std::vector<hash_type> instances;
  };

}

#endif
