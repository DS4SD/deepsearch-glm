//-*-C++-*-

#ifndef ANDROMEDA_PRODUCERS_DOCUMENT_H_
#define ANDROMEDA_PRODUCERS_DOCUMENT_H_

#include <iostream>
#include <fstream>

namespace andromeda
{

  template<>
  class producer<DOCUMENT>: public base_producer
  {
  public:

    const static inline std::string keep_text_lbl = "keep-text";
    const static inline std::string keep_table_lbl = "keep-tables";
    
    typedef doc_type subject_type;
    
  public:

    producer();
    producer(std::vector<model_ptr_type> models);
    producer(nlohmann::json config, std::vector<model_ptr_type> models);

    producer(const producer<DOCUMENT>& other);

    ~producer();

    nlohmann::json to_json();

    virtual subject_name get_subject_name() { return DOCUMENT; }
    
    virtual bool initialise(nlohmann::json& config);
    virtual bool reset_pointer();

    virtual bool set_ofs(std::filesystem::path path);
    
    /* next */
    
    virtual bool next(std::string& text, std::size_t& cnt) { return false; };

    virtual bool next(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool next(paragraph_type& subj, std::size_t& cnt) { return false; };

    //virtual bool next(webdoc_type& subj, std::size_t& cnt) { return false; };
    virtual bool next(doc_type& subj, std::size_t& cnt);

    /* read */
    
    virtual bool read(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool read(paragraph_type& subj, std::size_t& cnt) { return false; };

    //virtual bool read(webdoc_type& subj, std::size_t& cnt) { return false; };
    virtual bool read(doc_type& subj, std::size_t& cnt);

    /* write */
    
    virtual bool write(table_type& subj) { return false; };
    virtual bool write(paragraph_type& subj) { return false; };

    //virtual bool write(webdoc_type& subj) { return false; };
    virtual bool write(doc_type& subj);    

    /* apply */
    
    virtual bool apply(table_type& subj) { return false; };
    virtual bool apply(paragraph_type& subj) { return false; };

    //virtual bool apply(webdoc_type& subj) { return false; };
    virtual bool apply(doc_type& subj);

  private:

    std::size_t curr_docs;
    
    bool keep_text, keep_tables;
  };

  producer<DOCUMENT>::producer():
    base_producer(),

    curr_docs(0),
    
    keep_text(true),
    keep_tables(true)
  {}

  producer<DOCUMENT>::producer(std::vector<model_ptr_type> models):
    base_producer(models),

    curr_docs(0),
    
    keep_text(true),
    keep_tables(true)
  {}
  
  producer<DOCUMENT>::producer(nlohmann::json config,
			     std::vector<model_ptr_type> models):
    base_producer(models),

    curr_docs(0),
    
    keep_text(true),
    keep_tables(true)
  {
    initialise(config);
  }

  producer<DOCUMENT>::producer(const producer<DOCUMENT>& other)
  {}

  producer<DOCUMENT>::~producer()
  {}

  nlohmann::json producer<DOCUMENT>::to_json()
  {
    nlohmann::json configs = nlohmann::json::array({});

    {
      nlohmann::json config = nlohmann::json::object({});
      config[base_producer::subject_lbl] = to_string(DOCUMENT);

      config[maxnum_docs_lbl] = "<optional:int>";
      
      config[iformat_lbl] = "json";
      config[oformat_lbl] = "annot.json";

      std::vector<std::string> ipaths = { "<path-to-directory-of-json-files>" };
      config[ipaths_lbl] = ipaths;

      config[write_output_lbl] = true;
      config[opath_lbl] = "<optional:output-directory-of-json-files>";
      
      config[keep_text_lbl] = keep_text;
      config[keep_table_lbl] = keep_tables;
      
      configs.push_back(config);
    }

    return configs;
  }
  
  bool producer<DOCUMENT>::initialise(nlohmann::json& config)
  {
    base_producer::initialise(config);
    base_producer::find_filepaths();
    
    keep_text = this->configuration.value(keep_text_lbl, keep_text);
    keep_tables = this->configuration.value(keep_table_lbl, keep_tables);

    return reset_pointer();
  }

  bool producer<DOCUMENT>::reset_pointer()
  {
    curr_docs = 0;

    path_itr = paths.begin();
    path_end = paths.end();

    return true;
  }

  bool producer<DOCUMENT>::set_ofs(std::filesystem::path path)
  {
    base_producer::opath = path;
    return true;
  }
  
  bool producer<DOCUMENT>::next(doc_type& subject, std::size_t& cnt)
  {    
    if(read(subject, cnt))
      {
        return apply(subject);
      }

    return false;
  }

  bool producer<DOCUMENT>::read(doc_type& subject, std::size_t& count)
  {
    bool valid=false, success=false;

    while((not valid) and (path_itr!=path_end))
      {
	//LOG_S(INFO) << "reading: " << path_itr->c_str();

	std::ifstream ifs(path_itr->c_str());
	if(ifs)
	  {
	    nlohmann::json data;
	    ifs >> data;

	    valid = subject.set_text(*path_itr, data);
	  }

	success = ((valid) and (path_itr!=path_end));
	
	path_itr++;
      }

    if(success)
      {
	count += 1;
	curr_docs += 1;
      }
    
    return success;
  }

  bool producer<DOCUMENT>::apply(doc_type& subject)
  {
    subject.set_tokens(char_normaliser, text_normaliser);

    for(auto& model:models)
      {
        model->apply(subject);
      }

    return true;
  }

  bool producer<DOCUMENT>::write(doc_type& subj)
  {
    std::filesystem::path ofile;
    if(not get_output_file(ofile))
      {
	return false;
      }

    std::ofstream ofs;
    ofs.open(ofile.c_str(), std::ofstream::out);
    
    if(ofs.good())
      {
	nlohmann::json data = subj.to_json();

	if(opath.ends_with("json"))
	  {
	    ofs << std::setw(4) << data;
	  }
	else
	  {
	    ofs << data << "\n";
	  }
	
	ofs.close();
	return true;
      }
    
    return false;
  }
  
}

#endif
