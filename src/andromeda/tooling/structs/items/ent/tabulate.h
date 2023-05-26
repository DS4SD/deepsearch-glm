//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_ENT_TABULATE_H_
#define ANDROMEDA_STRUCTS_ITEMS_ENT_TABULATE_H_

namespace andromeda
{

  void to_json(std::vector<base_entity>& entities, nlohmann::json& ents)
  {
    ents = nlohmann::json::object({});

    ents["headers"] = base_entity::HEADERS;

    nlohmann::json& data = ents["data"];      
    data = nlohmann::json::array({});
    
    for(std::size_t l=0; l<entities.size(); l++)
      {
	data.push_back(entities.at(l).to_json_row());
      }
  }

  bool from_json(std::vector<base_entity>& entities, const nlohmann::json& ents)
  {
    auto& data = ents["data"];      

    bool success=true;
    
    base_entity ent;
    for(auto& row:data)
      {
	if(ent.from_json_row(row))
	  {
	    entities.push_back(ent);
	  }
	else
	  {
	    success=false;
	  }
      }

    return success;
  }
  
  template<typename index_type>
  void create_hash_to_name(std::vector<base_entity>& entities,
                           std::map<index_type, std::string>& hash_to_name)
  {
    hash_to_name.clear();
    for(const auto& ent:entities)
      {
        hash_to_name.insert({ent.ehash, ent.name});
      }
  }

  std::string tabulate(std::vector<base_entity>& entities, bool sort=true)
  {
    if(sort)
      {
	std::sort(entities.begin(), entities.end());
      }
    
    std::stringstream ss;

    std::vector<std::string> headers={};

    if(entities.size()==0)
      {
        ss << "\nentities: " << entities.size() << "\n";
        return ss.str();
      }
    else if(entities.at(0).subj_name==PARAGRAPH)
      {
        headers = base_entity::short_text_headers();
      }
    else if(entities.at(0).subj_name==TABLE)
      {
        headers = base_entity::short_table_headers();
      }
    else
      {
	LOG_S(ERROR) << "not supported subject-type";
	return "";
      }

    std::vector<std::vector<std::string> > data={};

    std::size_t col_width=32;
    for(auto& ent:entities)
      {
        auto row = ent.to_row(col_width);
        if(row.size()==headers.size())
          {
            data.push_back(ent.to_row(col_width));
          }
	else
	  {
	    LOG_S(WARNING) << "inconsistent sizes: "
			   << headers.size() << " versus " << row.size();
	  }
      }

    ss << "\nentities: " << entities.size() << "\n"
       << utils::to_string(headers, data);

    return ss.str();
  }

  std::string tabulate(std::string text, std::vector<base_entity>& entities)
  {
    std::sort(entities.begin(), entities.end(),
              [](const base_entity& lhs, const base_entity& rhs)
              {
                if(lhs.char_range[0]==rhs.char_range[0])
                  {
                    return lhs.char_range[1]>rhs.char_range[1];
                  }

                return lhs.char_range[0]<rhs.char_range[0];
              });

    std::vector<std::string> header = base_entity::short_text_headers();
    std::vector<std::vector<std::string> > data={};

    //std::size_t col_width=64;
    std::size_t name_width = 32;
    std::size_t orig_width = 48;

    for(auto& ent:entities)
      {
        data.push_back(ent.to_row(text, name_width, orig_width));
      }

    std::stringstream ss;
    if(entities.size()==0)
      {
        ss << "\nentities: " << entities.size() << "\n";
      }
    else
      {
        ss << "\nentities: " << entities.size() << "\n"
           << utils::to_string(header, data);
      }

    return ss.str();
  }

}

#endif
