//-*-C++-*-

#ifndef ANDROMEDA_PRODUCERS_PROMPT_H_
#define ANDROMEDA_PRODUCERS_PROMPT_H_

namespace andromeda
{

  template<>
  class producer<PROMPT>: public base_producer
  {
  public:

    typedef paragraph_type subject_type;
    
  public:

    producer();
    producer(std::vector<model_ptr_type> models);
    producer(nlohmann::json& config, std::vector<model_ptr_type>& models);

    producer(const producer<PROMPT>& other);

    ~producer();

    nlohmann::json to_json();
    
    virtual subject_name get_subject_name() { return PROMPT; }

    virtual bool keep_reading(std::size_t cnt) { return (not quit); }
    
    virtual bool initialise(nlohmann::json& config);
    
    virtual bool reset_pointer() { return true; }
    
    /* next */

    virtual bool next(std::string& text, std::size_t& cnt);

    virtual bool next(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool next(paragraph_type& subj, std::size_t& cnt);
    virtual bool next(doc_type& subj, std::size_t& cnt) { return false; };

    /* read */

    virtual bool read(table_type& subj, std::size_t& cnt) { return false; };
    virtual bool read(paragraph_type& subj, std::size_t& cnt);
    virtual bool read(doc_type& subj, std::size_t& cnt) { return false; };

    /* write */

    virtual bool write(table_type& subj) { return false; };
    virtual bool write(paragraph_type& subj);
    virtual bool write(doc_type& subj) { return false; };

    /* apply */

    virtual bool apply(table_type& subj) { return false; };
    virtual bool apply(paragraph_type& subj);
    virtual bool apply(doc_type& subj) { return false; };

  private:

    bool quit;
    
    std::ofstream ofs;
  };

  producer<PROMPT>::producer():
    base_producer(),
    quit(false)
  {}

  producer<PROMPT>::producer(std::vector<model_ptr_type> models):
    base_producer(models),
    quit(false)
  {}

  producer<PROMPT>::producer(nlohmann::json& config, std::vector<model_ptr_type>& models):
    base_producer(config, models),
    quit(false)
  {
    initialise(config);
  }

  producer<PROMPT>::producer(const producer<PROMPT>& other):
    base_producer(),
    quit(false)
  {}

  producer<PROMPT>::~producer()
  {}

  nlohmann::json producer<PROMPT>::to_json()
  {
    nlohmann::json configs = nlohmann::json::array({});

    {
      nlohmann::json config = nlohmann::json::object({});
      config[base_producer::subject_lbl] = to_string(PROMPT);
      
      config[oformat_lbl] = "jsonl";
      config[write_output_lbl] = true;
      
      std::filesystem::path opath("./out.jsonl");
      config[opath_lbl] = opath.parent_path();
      
      configs.push_back(config);
    }

    return configs;
  }

  bool producer<PROMPT>::initialise(nlohmann::json& config)
  {
    return base_producer::initialise(config);
  }
  
  bool producer<PROMPT>::next(std::string& text,
			      std::size_t& cnt)
  {
    std::cout << "text: " << std::flush;

    std::string line;
    std::getline(std::cin, line);

    text = line;
    if(text=="quit")
      {
	quit = true;
      }

    return (not quit);
  }

  bool producer<PROMPT>::next(paragraph_type& subject,
			      std::size_t& cnt)
  {
    if(read(subject, cnt))
      {
        return apply(subject);
      }

    return false;
  }

  bool producer<PROMPT>::read(paragraph_type& subject,
			      std::size_t& cnt)
  {
    std::string line;
    if(next(line, cnt) and subject.set_text(line))
      {
        return true;
      }

    return false;
  }
  
  bool producer<PROMPT>::write(paragraph_type& subj)
  {
    if(write_output and ofs.good())
      {
        ofs << subj.to_json({}) << "\n";
        return true;
      }

    return false;
  }

  bool producer<PROMPT>::apply(paragraph_type& subject)
  {
    subject.set_tokens(char_normaliser, text_normaliser);

    for(auto& model:models)
      {
        model->apply(subject);
      }

    return true;
  }
  
}

#endif
