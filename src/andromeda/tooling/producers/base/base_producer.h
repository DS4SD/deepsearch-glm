//-*-C++-*-

#ifndef ANDROMEDA_PRODUCERS_BASE_PRODUCERS_H_
#define ANDROMEDA_PRODUCERS_BASE_PRODUCERS_H_

namespace andromeda
{
  class base_producer
  {
  public:

    const static inline std::string subject_lbl = "subject-type";
    const static inline std::string producers_lbl = "producers";
    
    const static inline std::string maxnum_docs_lbl = "input-max-documents";
    
    const static inline std::string iformat_lbl = "input-format";
    const static inline std::string ipaths_lbl = "input-paths";
    
    const static inline std::string write_output_lbl = "output";
    
    const static inline std::string opath_lbl = "output-path";
    const static inline std::string oformat_lbl = "output-format";

    const static inline std::string null_opath = "null";
    
    typedef andromeda::base_nlp_model model_type;
    typedef std::shared_ptr<model_type> model_ptr_type;

    typedef subject<TEXT> paragraph_type;
    typedef subject<TABLE> table_type;

    typedef subject<DOCUMENT> doc_type;
    
  public:

    base_producer();

    base_producer(std::vector<model_ptr_type> models);

    base_producer(nlohmann::json& config,
		  std::vector<model_ptr_type>& models);
    
    base_producer(nlohmann::json& config,
		  std::vector<model_ptr_type>& models,
		  bool verbose);
    
    virtual ~base_producer() {}

    virtual subject_name get_subject_name()=0;
    
    nlohmann::json get_configuration() { return configuration; }
    
    std::vector<model_ptr_type> get_models() { return models; }
    
    virtual bool reset_pointer() { return false; };
    virtual bool set_ofs(std::filesystem::path path) { return false; }
    
    virtual bool keep_reading(std::size_t cnt) { return ((path_itr!=paths.end()) and (cnt<maxnum_docs)); }
    
    /* NEXT will execute `read` and `apply` in consecutive order */
    
    virtual bool next(std::string& text, std::size_t& cnt) { return false; };
    
    virtual bool next(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool next(paragraph_type& subj, std::size_t& cnt) { return false; };
    virtual bool next(doc_type& subj, std::size_t& cnt) { return false; };

    /* READ does only set the `orig` part of the subject */
    
    virtual bool read(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool read(paragraph_type& subj, std::size_t& cnt) { return false; };
    virtual bool read(doc_type& subj, std::size_t& cnt) { return false; };

    /* APPLY only does the char/text-normalisation and NLP-models */
    
    virtual bool apply(table_type& subj) { return false; };
    virtual bool apply(paragraph_type& subj) { return false; };
    virtual bool apply(doc_type& subj) { return false; };    

  protected:

    virtual bool initialise(nlohmann::json& config);

    bool find_filepaths();

    bool get_output_file(std::filesystem::path& outfile);

    bool get_output_file(std::filesystem::path& outfile,
			 std::filesystem::path& ifilepath,
			 std::filesystem::path& ifilename);
    
  protected:

    bool verbose;
    
    nlohmann::json configuration;
    
    std::vector<model_ptr_type> models;

    std::shared_ptr<utils::char_normaliser> char_normaliser;
    std::shared_ptr<utils::text_normaliser> text_normaliser;
    
    std::vector<std::filesystem::path> paths;

    std::vector<std::filesystem::path>::iterator path_itr;
    std::vector<std::filesystem::path>::iterator path_end;

    std::size_t curnum_docs, maxnum_docs;
    std::string iformat, oformat;

    bool write_output;
    std::string opath;
  };

  base_producer::base_producer():
    verbose(false),
    configuration({}),
    models({}),

    char_normaliser(std::make_shared<utils::char_normaliser>(verbose)),
    text_normaliser(std::make_shared<utils::text_normaliser>(verbose)),

    paths({}),

    path_itr(paths.begin()),
    path_end(paths.end()),

    curnum_docs(0),
    maxnum_docs(-1),

    iformat("txt"),
    oformat("annot.txt"),

    write_output(false),
    opath(null_opath)
  {}

  base_producer::base_producer(std::vector<model_ptr_type> models):
    verbose(false),
    configuration({}),
    models(models),

    char_normaliser(std::make_shared<utils::char_normaliser>(verbose)),
    text_normaliser(std::make_shared<utils::text_normaliser>(verbose)),

    paths({}),

    path_itr(paths.begin()),
    path_end(paths.end()),

    curnum_docs(0),
    maxnum_docs(-1),

    iformat("txt"),
    oformat("annot.txt"),

    write_output(false),
    opath(null_opath)
  {}

  base_producer::base_producer(nlohmann::json& config, std::vector<model_ptr_type>& models):
    verbose(false),
    configuration(config),
    models(models),
    
    char_normaliser(std::make_shared<utils::char_normaliser>(verbose)),
    text_normaliser(std::make_shared<utils::text_normaliser>(verbose)),
    
    paths({}),
    
    path_itr(paths.begin()),
    path_end(paths.end()),
    
    curnum_docs(0),    
    maxnum_docs(-1),
    
    iformat("txt"),
    oformat("annot.txt"),
    
    write_output(false),
    opath(null_opath)    
  {
    initialise(config);
  }

  base_producer::base_producer(nlohmann::json& config, std::vector<model_ptr_type>& models, bool verbose):
    verbose(verbose),
    configuration(config),
    models(models),

    char_normaliser(std::make_shared<utils::char_normaliser>(verbose)),
    text_normaliser(std::make_shared<utils::text_normaliser>(verbose)),

    paths({}),

    path_itr(paths.begin()),
    path_end(paths.end()),

    curnum_docs(0),    
    maxnum_docs(-1),
    
    iformat("txt"),
    oformat("annot.txt"),

    write_output(false),
    opath(null_opath)    
  {
    initialise(config);
  }
  
  bool base_producer::initialise(nlohmann::json& config)
  {
    configuration = config;

    curnum_docs = 0;
    maxnum_docs = configuration.value(maxnum_docs_lbl, maxnum_docs);
    
    iformat = configuration.value(iformat_lbl, iformat);

    oformat = "annot."+iformat;
    oformat = configuration.value(oformat_lbl, oformat);

    write_output = configuration.value(write_output_lbl, write_output);
    opath = configuration.value(opath_lbl, null_opath);

    if(opath!=null_opath and (not std::filesystem::exists(opath)))
      {
	try
	  {
	    LOG_S(INFO) << "creating output-directory: " << opath.c_str();
	    std::filesystem::create_directory(opath);
	  }
	catch(std::exception& exc)
	  {
	    LOG_S(ERROR) << "could not create output-directory: " << exc.what();
	  }
      }

    if(configuration.count(ipaths_lbl))
      {
	return find_filepaths();
      }

    return true;
  }

  bool base_producer::find_filepaths()
  {
    std::vector<std::string> items={};
    items = configuration.value(ipaths_lbl, items);

    if(items.size()==0)
      {
	LOG_S(ERROR) << "could not find any files from configurations: \n"
		     << std::setw(2) << configuration;
	return false;
      }
    
    this->paths={};
    for(auto& item:items)
      {
	LOG_S(INFO) << "item: " << item;
	
	std::filesystem::path path(item.c_str());

	if(not std::filesystem::exists(path))
	  {
	    LOG_S(WARNING) << "file does not exist: " << item;
	    continue;
	  }
	else if(std::filesystem::is_directory(path))
	  {
	    for(auto dir_entry:std::filesystem::directory_iterator(path))
	      {
		std::string fn = dir_entry.path().string();
		if(fn.ends_with(iformat) and (not fn.ends_with(oformat)))
		  {
		    paths.push_back(dir_entry.path());
		  }
	      }
	  }
	else if(item.ends_with(iformat) and (not item.ends_with(oformat)))
	  {
	    this->paths.push_back(item.c_str());
	  }
	else
	  {
	    LOG_S(WARNING) << "ignoring " << item;
	  }
      }

    if((this->paths).size()==0)
      {
	LOG_S(ERROR) << "could not find any files to produce from ...";
	return false;
      }
    else
      {
	LOG_S(INFO) << "found " << (this->paths).size() << " files to ingest!";

	//for(auto itr=paths.begin(); itr!=paths.end(); itr++)
	//{
	//LOG_S(INFO) << "\t path: " << *itr;
	//}
      }

    std::sort(paths.begin(), paths.end());
    
    return true;
  }

  bool base_producer::get_output_file(std::filesystem::path& out)
  {
    if(not write_output)
      {
	return false;
      }

    if(path_itr==path_end)
      {
	LOG_S(WARNING) << __FILE__ << ":" << __LINE__;
	return false;
      }
    
    if(opath!=null_opath)
      {
	std::filesystem::path odir(opath.c_str());
	std::filesystem::path ifile = (path_itr->filename());
	
	// std::string ofile = utils::replace(ifile.c_str(), iformat, oformat);
  std::string ofile = utils::replace(ifile.string(), iformat, oformat);
	std::filesystem::path outfile(ofile.c_str());
	
	out = odir / outfile;
      }
    else
      {
	std::filesystem::path idir = (path_itr->parent_path());
	std::filesystem::path ifile = (path_itr->filename());
	
	// std::string ofile = utils::replace(ifile.c_str(), iformat, oformat);
  std::string ofile = utils::replace(ifile.string(), iformat, oformat);
	std::filesystem::path outfile(ofile.c_str());
	
	out = idir / outfile;
      }

    //LOG_S(INFO) << "opening for writing: " << out.c_str();
    
    return true;
  }

  bool base_producer::get_output_file(std::filesystem::path& outfile,
				      std::filesystem::path& ifiledir,
				      std::filesystem::path& ifilename)
  {
    if(not write_output)
      {
	return false;
      }

    //if(path_itr==path_end)
    //{
    //LOG_S(WARNING) << __FILE__ << ":" << __LINE__;
    //return false;
    //}
    
    if(opath!=null_opath)
      {
	std::filesystem::path odir(opath.c_str());
	
	// std::string ofile = utils::replace(ifilename.c_str(), iformat, oformat);
  std::string ofile = utils::replace(ifilename.string(), iformat, oformat);
	std::filesystem::path ofilename(ofile.c_str());
	
	outfile = odir / ofilename;		
      }
    else
      {
	std::filesystem::path odir(ifiledir.c_str());
	
	// std::string ofile = utils::replace(ifilename.c_str(), iformat, oformat);
  std::string ofile = utils::replace(ifilename.string(), iformat, oformat);
	std::filesystem::path ofilename(ofile.c_str());
	
	outfile = odir / ofilename;	
      }
    
    //LOG_S(WARNING) << "opath: " << opath.c_str();
    
    return true;
  }
  
}

#endif
