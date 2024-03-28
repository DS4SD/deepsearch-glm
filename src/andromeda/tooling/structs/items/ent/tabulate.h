//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_ITEMS_ENT_TABULATE_H_
#define ANDROMEDA_STRUCTS_ITEMS_ENT_TABULATE_H_

namespace andromeda
{

  void to_json(std::vector<base_instance>& instances, nlohmann::json& insts)
  {
    insts = nlohmann::json::object({});

    insts["headers"] = base_instance::HEADERS;

    nlohmann::json& data = insts["data"];      
    data = nlohmann::json::array({});
    
    for(std::size_t l=0; l<instances.size(); l++)
      {
	data.push_back(instances.at(l).to_json_row());
      }
  }

  bool from_json(std::vector<base_instance>& instances, const nlohmann::json& insts)
  {
    auto& data = insts["data"];      

    bool success=true;
    
    base_instance ent;
    for(auto& row:data)
      {
	if(ent.from_json_row(row))
	  {
	    instances.push_back(ent);
	  }
	else
	  {
	    success=false;
	  }
      }

    return success;
  }
  
  template<typename index_type>
  void create_hash_to_name(std::vector<base_instance>& instances,
                           std::map<index_type, std::string>& hash_to_name)
  {
    hash_to_name.clear();
    for(const auto& ent:instances)
      {
        hash_to_name.insert({ent.get_ehash(), ent.get_name()});
      }
  }

  std::string tabulate(std::vector<base_instance>& instances, bool sort=true)
  {
    if(sort)
      {
	std::sort(instances.begin(), instances.end());
      }
    
    std::stringstream ss;

    std::vector<std::string> headers={};

    if(instances.size()==0)
      {
        ss << "\ninstances: " << instances.size() << "\n";
        return ss.str();
      }
    else if(instances.at(0).is_in(TEXT) or instances.at(0).is_in(DOCUMENT))
      {
        headers = base_instance::short_text_headers();
      }
    else if(instances.at(0).is_in(TABLE))
      {
        headers = base_instance::short_table_headers();
      }
    else
      {
	LOG_S(ERROR) << "not supported subject-type";
	return "";
      }

    std::vector<std::vector<std::string> > data={};

    std::size_t col_width=32;
    for(auto& ent:instances)
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

    ss << "\ninstances: " << instances.size() << "\n"
       << utils::to_string(headers, data);

    return ss.str();
  }

  std::string tabulate(std::string text, std::vector<base_instance>& instances)
  {
    std::sort(instances.begin(), instances.end(),
              [](const base_instance& lhs, const base_instance& rhs)
              {
		/*
                if(lhs.char_range[0]==rhs.char_range[0])
                  {
                    return lhs.char_range[1]>rhs.char_range[1];
                  }

                return lhs.char_range[0]<rhs.char_range[0];
		*/
		
                if(lhs.get_char_range(0)==rhs.get_char_range(0))
                  {
                    return lhs.get_char_range(1)>rhs.get_char_range(1);
                  }
                return lhs.get_char_range(0)<rhs.get_char_range(0);
              });

    std::vector<std::string> header = base_instance::short_text_headers();
    std::vector<std::vector<std::string> > data={};

    std::size_t name_width = 32;
    std::size_t orig_width = 48;

    for(auto& ent:instances)
      {
        data.push_back(ent.to_row(text, name_width, orig_width));
      }

    std::stringstream ss;
    if(instances.size()==0)
      {
        ss << "\ninstances: " << instances.size() << "\n";
      }
    else
      {
        ss << "\ninstances: " << instances.size() << "\n"
           << utils::to_string(header, data);
      }

    return ss.str();
  }

}

#endif
