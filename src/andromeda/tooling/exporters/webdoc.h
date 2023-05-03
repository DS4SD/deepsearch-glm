//-*-C++-*-

#ifndef ANDROMEDA_EXPORTERS_WEBDOC_H_
#define ANDROMEDA_EXPORTERS_WEBDOC_H_

#include <iostream>
#include <fstream>

namespace andromeda
{

  template<>
  class exporter<WEBDOC>
  {
    typedef subject<WEBDOC> subject_type;
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
    void execute_entity(subject_type& subj, entity_type& ent, std::ofstream& of);
    
    template<typename entity_type>
    void execute_sentence(subject_type& subj, entity_type& ent, std::ofstream& of);
    
  private:

    std::vector<std::string> types;

    std::filesystem::path root;
    std::map<std::string, std::ofstream> ofs;
  };

  exporter<WEBDOC>::exporter(std::vector<std::string> types,
			     std::filesystem::path root):
    types(types),
    root(root),
    ofs()
  {
    initialise();
  }

  exporter<WEBDOC>::exporter(std::string line)
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
  
  exporter<WEBDOC>::~exporter()
  {
    for(auto itr=ofs.begin(); itr!=ofs.end(); itr++)
      {
	(itr->second).close();
      }
  }

  bool exporter<WEBDOC>::initialise()
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
	catch(std::exception e)
	  {
	    LOG_S(ERROR) << "failed exporting " << type
			 << " with error: " << e.what();
	  }
      }

    return true;
  }
    
  bool exporter<WEBDOC>::initialised()
  {
    return (ofs.size()>0);
  }
  
  void exporter<WEBDOC>::execute(subject_type& subj)
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
	
	if(key=="sentence")
	  {
	    execute_sentence(subj, ent, of);
	  }
	else
	  {
	    execute_entity(subj, ent, of);
	  }
      }
  }

  template<typename entity_type>
  void exporter<WEBDOC>::execute_entity(subject_type& subj, entity_type& ent, std::ofstream& of)
  {
    if(ent.element_type==PARAGRAPH)
      {
	of << std::setw(12) << to_key(ent.type)
	   << std::setw(20) << ent.subtype
	   << std::setw(48) << ent.name
	   << "\n";
      }
  }
  
  template<typename entity_type>
  void exporter<WEBDOC>::execute_sentence(subject_type& subj, entity_type& ent, std::ofstream& of)
  {
    if(ent.element_type==PARAGRAPH)
      {
	std::string& text = subj.paragraphs.at(ent.element_index).text;
	std::array<std::size_t, 2> range = ent.char_range;
	
	std::string sent = text.substr(range[0], range[1]-range[0]);
	of << sent << "\n";
      }
  }
  
}

#endif
