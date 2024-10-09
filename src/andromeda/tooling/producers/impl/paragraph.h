//-*-C++-*-

#ifndef ANDROMEDA_PRODUCERS_TEXT_H_
#define ANDROMEDA_PRODUCERS_TEXT_H_

#include <iostream>
#include <fstream>

namespace andromeda
{

  template<>
  class producer<TEXT>: public base_producer
  {
  public:

    typedef paragraph_type subject_type;

  public:

    producer();
    producer(std::vector<model_ptr_type> models);
    producer(nlohmann::json& config, std::vector<model_ptr_type>& models);

    producer(const producer<TEXT>& other);

    ~producer();

    nlohmann::json to_json();

    virtual subject_name get_subject_name() { return TEXT; }

    virtual bool initialise(nlohmann::json& config);
    virtual bool reset_pointer();

    virtual bool set_ofs(std::filesystem::path path);

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

    bool initialise(std::string filename,
                    std::string format="txt",
                    std::string key="");

  private:

    std::string key;
    std::size_t start_line, curr_line;

    std::ifstream ifs;
    std::ofstream ofs;
  };

  producer<TEXT>::producer():
    base_producer(),

    key(""),

    start_line(0),
    curr_line(0)
  {}

  producer<TEXT>::producer(std::vector<model_ptr_type> models):
    base_producer(models),

    key(""),

    start_line(0),
    curr_line(0)
  {}

  producer<TEXT>::producer(nlohmann::json& config, std::vector<model_ptr_type>& models):
    base_producer(config, models),

    key(""),

    start_line(0),
    curr_line(0)
  {
    initialise(config);
  }

  producer<TEXT>::producer(const producer<TEXT>& other):
    base_producer(),

    key(other.key),

    start_line(other.start_line),
    curr_line(other.curr_line)
  {}

  producer<TEXT>::~producer()
  {}

  nlohmann::json producer<TEXT>::to_json()
  {
    nlohmann::json configs = nlohmann::json::array({});

    {
      nlohmann::json config = nlohmann::json::object({});
      config[base_producer::subject_lbl] = to_string(TEXT);

      config[maxnum_docs_lbl] = "<optional:int>";

      config[iformat_lbl] = "<str:jsonl>";
      config[oformat_lbl] = "<str:annot.jsonl>";

      std::vector<std::string> paths = { "<path-to-file>" };
      config[ipaths_lbl] = paths;

      config[write_output_lbl] = false;

      config["key"] = "<optional:string>";
      config["start-line"] = "<optional:int>";

      configs.push_back(config);
    }

    {
      nlohmann::json config = nlohmann::json::object({});
      config[base_producer::subject_lbl] = to_string(TEXT);

      config[maxnum_docs_lbl] = "<optional:int>";

      config[iformat_lbl] = "jsonl";
      config[oformat_lbl] = "annot.jsonl";

      std::vector<std::string> paths = { "../data/arxiv-abstracts/arxiv-metadata-oai-snapshot.jsonl" };
      config[ipaths_lbl] = paths;

      config[write_output_lbl] = true;

      // std::filesystem::path opath(paths.front().c_str());
      std::filesystem::path opath(paths.front());
      config[opath_lbl] = opath.parent_path();

      config["key"] = "abstract";
      config["start-line"] = 0;

      configs.push_back(config);
    }

    return configs;
  }

  bool producer<TEXT>::initialise(nlohmann::json& config)
  {
    if(not base_producer::initialise(config))
      {
        return false;
      }

    key = base_producer::configuration.value("key", key);
    start_line = base_producer::configuration.value("start-line", start_line);

    return reset_pointer();
  }

  bool producer<TEXT>::reset_pointer()
  {
    curr_line = 0;

    path_itr = paths.begin();
    path_end = paths.end();

    if(ifs.is_open())
      {
        ifs.close();
      }

    if(ofs.is_open())
      {
        ofs.close();
      }

    return true;
  }

  bool producer<TEXT>::set_ofs(std::filesystem::path path)
  {
    if(ofs.is_open())
      {
        ofs.close();
      }

    base_producer::write_output = true;
    // ofs.open(path.c_str(), std::ofstream::out);
    ofs.open(path, std::ofstream::out);

    return ofs.good();
  }

  bool producer<TEXT>::next(std::string& text,
                            std::size_t& cnt)
  {
    if(cnt++>=maxnum_docs)
      {
        static bool show=true;
        if(show)
          {
            show=false;
            LOG_S(WARNING) << "count is exceeding max-count: " << cnt
                           << " versus " << maxnum_docs;
          }

        return false;
      }

    //LOG_S(INFO) << "(path_itr==path_end): " << (path_itr==path_end);

    std::string line;
    while(not (ifs.is_open() and std::getline(ifs, line)))
      {
        //LOG_S(INFO) << "file open: " << ifs.is_open() << " -> " << line;

        if(path_itr==paths.end())
          {
            //LOG_S(WARNING) << "path_itr==paths.end()";
            return false;
          }
        else if(ifs.is_open())
          {
            ifs.close();
            path_itr++;
          }
        else
          {
            //LOG_S(INFO) << "opening for reading: " << (path_itr->c_str());
            // ifs.open(path_itr->c_str(), std::ifstream::in);
            ifs.open(*path_itr, std::ifstream::in);

            std::filesystem::path outfile;
            if(get_output_file(outfile))
              {
                // LOG_S(WARNING) << "writing to: " << outfile.c_str();
                LOG_S(WARNING) << "writing to: " << outfile.string();
                // ofs.open(outfile.c_str(), std::ofstream::out);
                ofs.open(outfile, std::ofstream::out);
              }

            curr_line=0;
          }
      }

    if(curr_line<start_line)
      {
        while(curr_line<start_line and ifs.is_open() and std::getline(ifs, line))
          {
            curr_line += 1;
          }
      }
    else
      {
        curr_line += 1;
      }

    if(iformat=="txt")
      {
        text = line;
        return true;
      }
    else if(iformat=="jsonl")
      {
        try
          {
            auto data = nlohmann::json::parse(line);
            text = data.value(key, "");
          }
        catch(std::exception& exc)
          {
            LOG_S(ERROR) << "could not json-parse line: `" << line << "`"
                         << " with error (" << exc.what() << ")";
            return false;
          }

        return true;
      }
    else
      {
        LOG_S(WARNING) << "can not support format " << iformat;
      }

    return false;
  }

  bool producer<TEXT>::next(paragraph_type& subject,
                            std::size_t& cnt)
  {
    if(read(subject, cnt))
      {
        return apply(subject);
      }

    return false;
  }

  bool producer<TEXT>::read(paragraph_type& subject,
                            std::size_t& cnt)
  {
    subject.clear();
    
    std::string line;
    if(next(line, cnt) and subject.set_text(line))
      {
        return true;
      }

    return false;
  }

  bool producer<TEXT>::write(paragraph_type& subj)
  {
    if(write_output and ofs.good())
      {
        ofs << subj.to_json({}) << "\n";
        return true;
      }

    return false;
  }

  bool producer<TEXT>::apply(paragraph_type& subject)
  {
    subject.set_tokens(char_normaliser, text_normaliser);

    for(auto& model:models)
      {
        model->apply(subject);
      }

    subject.finalise();

    return true;
  }

}

#endif
