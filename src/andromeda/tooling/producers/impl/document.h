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

    const static inline std::string order_text_lbl = "order-text";
    
    const static inline std::string keep_text_lbl = "keep-text";
    const static inline std::string keep_table_lbl = "keep-tables";
    const static inline std::string keep_figure_lbl = "keep-figures";
    
    typedef doc_type subject_type;
    
  public:

    producer();
    producer(std::vector<model_ptr_type> models);
    producer(nlohmann::json config, std::vector<model_ptr_type> models);

    ~producer();

    nlohmann::json to_json();

    virtual subject_name get_subject_name() { return DOCUMENT; }
    
    virtual bool initialise(nlohmann::json& config);
    virtual bool reset_pointer();

    virtual bool set_ofs(std::filesystem::path odir);
    
    /* next */
    
    virtual bool next(std::string& text, std::size_t& cnt) { return false; };

    virtual bool next(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool next(paragraph_type& subj, std::size_t& cnt) { return false; };

    virtual bool next(doc_type& subj, std::size_t& cnt);

    /* read */
    
    virtual bool read(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool read(paragraph_type& subj, std::size_t& cnt) { return false; };

    //virtual bool read(webdoc_type& subj, std::size_t& cnt) { return false; };
    virtual bool read(doc_type& subj, std::size_t& cnt);

    /* write */
    
    virtual bool write(table_type& subj) { return false; };
    virtual bool write(paragraph_type& subj) { return false; };

    virtual bool write(doc_type& subj);    

    /* apply */
    
    virtual bool apply(table_type& subj) { return false; };
    virtual bool apply(paragraph_type& subj) { return false; };

    virtual bool apply(doc_type& subj);

  private:

    std::size_t curr_docs;

    bool order_text;
    
    bool keep_text, keep_tables, keep_figures;
  };

  producer<DOCUMENT>::producer():
    base_producer(),

    curr_docs(0),

    order_text(true),
    
    keep_text(true),
    keep_tables(true),
    keep_figures(true)
  {}

  producer<DOCUMENT>::producer(std::vector<model_ptr_type> models):
    base_producer(models),

    curr_docs(0),

    order_text(true),
    
    keep_text(true),
    keep_tables(true),
    keep_figures(true)
  {}
  
  producer<DOCUMENT>::producer(nlohmann::json config,
			       std::vector<model_ptr_type> models):
    base_producer(models),

    curr_docs(0),

    order_text(true),
    
    keep_text(true),
    keep_tables(true),
    keep_figures(true)
  {
    initialise(config);
  }

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

      config[order_text_lbl] = order_text;
      
      config[keep_text_lbl] = keep_text;
      config[keep_table_lbl] = keep_tables;
      config[keep_figure_lbl] = keep_figures;
      
      configs.push_back(config);
    }

    return configs;
  }
  
  bool producer<DOCUMENT>::initialise(nlohmann::json& config)
  {
    base_producer::initialise(config);

    base_producer::find_filepaths();

    order_text = this->configuration.value(order_text_lbl, order_text);
    
    keep_text = this->configuration.value(keep_text_lbl, keep_text);
    keep_tables = this->configuration.value(keep_table_lbl, keep_tables);
    keep_figures = this->configuration.value(keep_figure_lbl, keep_figures);

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
    // base_producer::opath = path;
    base_producer::opath = path.string();
    base_producer::write_output = true;
    
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
    if(curr_docs>=maxnum_docs)
      {
	static bool show=true;
	if(show)
	  {
	    show=false;
	    LOG_S(WARNING) << "count is exceeding max-count: " << curr_docs
			   << " versus " << maxnum_docs;
	  }
	
        return false;
      }
    
    bool valid=false, success=false;

    while((not valid) and (path_itr!=path_end))
      {
	// LOG_S(INFO) << "reading: " << path_itr->c_str();
  LOG_S(INFO) << "reading: " << path_itr->string();

	// std::ifstream ifs(path_itr->c_str());
  std::ifstream ifs(path_itr->string());

	if(ifs)
	  {
	    nlohmann::json data;
	    ifs >> data;

	    valid = subject.set_data(*path_itr, data, order_text);
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

    //LOG_S(INFO) << "#-models: " << models.size();
    for(auto& model:models)
      {
        model->apply(subject);
      }
    
    subject.finalise();
    
    return true;
  }

  bool producer<DOCUMENT>::write(doc_type& subj)
  {
    std::filesystem::path filepath = subj.get_filepath();

    std::filesystem::path filename = filepath.filename();
    std::filesystem::path filedir = filepath.parent_path();

    //LOG_S(WARNING) << "filepath: " << filepath.c_str();
    //LOG_S(WARNING) << "filename: " << filename.c_str();
    //LOG_S(WARNING) << "filedir: " << filedir.c_str();
    
    std::filesystem::path outfile;// = filedir;
    if(not get_output_file(outfile, filedir, filename))
      {
	// LOG_S(ERROR) << "can not write: " << outfile.c_str();
  LOG_S(ERROR) << "can not write: " << outfile.string();
	return false;
      }
    
    // LOG_S(INFO) << "writing: " << outfile.c_str();
    LOG_S(ERROR) << "writing: " << outfile.string();
    
    std::ofstream ofs;
    // ofs.open(outfile.c_str(), std::ofstream::out);
    ofs.open(outfile.string(), std::ofstream::out);
    
    if(ofs.good())
      {
	nlohmann::json data = subj.to_json({});

	// std::string ext=outfile.extension();
  std::string ext = outfile.extension().string();
	if(ext==".json")
	  {
	    ofs << std::setw(4) << data;
	  }
	else if(ext==".jsonl")
	  {
	    ofs << data << "\n";
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
