//-*-C++-*-

#ifndef ANDROMEDA_EXPORTERS_PARAGRAPH_H_
#define ANDROMEDA_EXPORTERS_PARAGRAPH_H_

#include <iostream>
#include <fstream>

namespace andromeda
{

  template<>
  class exporter<PARAGRAPH>
  {
    typedef subject<PARAGRAPH> subject_type;
    typedef base_nlp_model base_model_type;

  public:

    exporter(std::vector<std::string> types,
             std::filesystem::path root);
    exporter(std::string line);

    ~exporter();

    bool initialised();

    void execute(subject_type& subj);

  private:

    bool initialise();

    template<typename entity_type>
    void execute_entity(entity_type& ent, std::ofstream& of);

    template<typename entity_type>
    void execute_sentence(subject_type& subj, entity_type& ent, std::ofstream& of);

  private:

    std::vector<std::string> types;

    std::filesystem::path root;
    std::map<std::string, std::ofstream> ofs;
  };

  exporter<PARAGRAPH>::exporter(std::vector<std::string> types,
                                std::filesystem::path root):
    types(types),
    root(root),
    ofs()
  {
    initialise();
  }

  exporter<PARAGRAPH>::exporter(std::string line)
  {
    if(line=="null")
      {
        ofs.clear();
        return;
      }

    auto parts = utils::split(line, ":");

    root = parts.at(0);
    types = utils::split(parts.at(1), ";");

    initialise();
  }

  exporter<PARAGRAPH>::~exporter()
  {
    for(auto itr=ofs.begin(); itr!=ofs.end(); itr++)
      {
        (itr->second).close();
      }
  }

  bool exporter<PARAGRAPH>::initialise()
  {
    if(not std::filesystem::exists(root))
      {
	std::filesystem::create_directory(root);
      }
    
    for(auto type:types)
      {
	std::string fname = "output."+type+".jsonl";
	
	std::filesystem::path path = root;
	path /= fname;

	try
	  {
	    ofs.emplace(std::make_pair(type, path.c_str()));
	    LOG_S(INFO) << "exporting " << type << " to " << path.string();
	  }
	catch(std::exception& exc)
	  {
	    LOG_S(ERROR) << "failed exporting " << type
			 << " with error: " << exc.what();
	  }
      }

    return true;
  }
    
  bool exporter<PARAGRAPH>::initialised()
  {
    return (ofs.size()>0);
  }

  void exporter<PARAGRAPH>::execute(subject_type& subj)
  {
    auto& ents = subj.entities;
	
    for(auto& ent:ents)
      {
	std::string key = to_key(ent.type);
	
	if(ofs.count(key)==0)
	  {
	    continue;
	  }

	auto& of = ofs.at(key);	
	
	if(ent.type==SENTENCE)
	  {
	    execute_sentence(subj, ent, of);
	  }
	else
	  {
	    execute_entity(ent, of);
	  }
      }
  }

  template<typename entity_type>
  void exporter<PARAGRAPH>::execute_entity(entity_type& ent, std::ofstream& of)
  {
    of << std::setw(12) << to_key(ent.type)
       << std::setw(20) << ent.subtype
       << std::setw(48) << ent.name
       << "\n";
  }
  
  template<typename entity_type>
  void exporter<PARAGRAPH>::execute_sentence(subject_type& subj, entity_type& ent, std::ofstream& of)
  {
    std::string& text = subj.text;
    std::array<std::size_t, 2> range = ent.char_range;
    
    std::string sent = text.substr(range[0], range[1]-range[0]);
    of << sent << "\n";
  }
  
}

#endif
