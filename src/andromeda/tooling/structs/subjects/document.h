//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_H_

namespace andromeda
{

  template<>
  class subject<DOCUMENT>
  {
  public:

    subject();
    ~subject();

    void show();

    void clear();

    nlohmann::json to_json();

    void show(bool txt=true, bool mdls=false,
              bool ctokens=false, bool wtokens=true,
              bool prps=true, bool ents=true, bool rels=true);

    bool set_data(nlohmann::json& data);

    bool set_data(std::filesystem::path filepath,
                  nlohmann::json& data);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool set(nlohmann::json& data,
             std::shared_ptr<utils::char_normaliser> char_normaliser,
             std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool set(std::filesystem::path filepath, nlohmann::json& data,
             std::shared_ptr<utils::char_normaliser> char_normaliser,
             std::shared_ptr<utils::text_normaliser> text_normaliser);

  public:

    bool valid;

    std::set<std::string> applied_models;

    std::filesystem::path filepath;
    nlohmann::json orig;

    std::string doc_name;
    uint64_t doc_hash;

    std::vector<std::size_t> pind_to_orig; // paragraphs-index to original maintext-index
    std::vector<std::size_t> tind_to_orig; // tables-index to original tables-index

    std::vector<subject<PARAGRAPH> > paragraphs;
    std::vector<subject<TABLE> > tables;

    std::vector<base_property> properties;
    std::vector<base_entity> entities;
    std::vector<base_relation> relations;
  };

  subject<DOCUMENT>::subject():
    valid(true),
    applied_models(),

    doc_name(""),
    doc_hash(-1),

    pind_to_orig({}),
    tind_to_orig({}),

    paragraphs(),
    tables(),

    properties(),
    entities(),
    relations()
  {}

  subject<DOCUMENT>::~subject()
  {}

  nlohmann::json subject<DOCUMENT>::to_json()
  {
    nlohmann::json result = orig;

    if(result.count("main-text"))
      {
        std::vector<std::string> keys = {"hash", "orig", "text",
                                         "properties", "entities", "relations"};

        for(std::size_t l=0; l<paragraphs.size(); l++)
          {
            if(paragraphs.at(l).valid)
              {
                auto para = paragraphs.at(l).to_json();

                std::size_t ind = pind_to_orig.at(l);
                auto& item = result["main-text"][ind];

                for(std::string key:keys)
                  {
                    item[key] = para[key];
                  }
              }
          }
      }

    return result;
  }

  void subject<DOCUMENT>::clear()
  {
    valid = false;

    orig = nlohmann::json::object({});

    pind_to_orig.clear();
    tind_to_orig.clear();

    paragraphs.clear();
    tables.clear();

    properties.clear();
    entities.clear();
    relations.clear();
  }

  void subject<DOCUMENT>::show(bool txt, bool mdls,
                               bool ctok, bool wtok,
                               bool prps, bool ents, bool rels)
  {
    for(auto paragraph:paragraphs)
      {
        paragraph.show(txt,mdls, ctok,wtok, prps,ents,rels);
      }
  }

  bool subject<DOCUMENT>::set_data(std::filesystem::path filepath, nlohmann::json& data)
  {
    this->filepath = filepath;
    return set_data(data);
  }

  bool subject<DOCUMENT>::set_data(nlohmann::json& data)
  {
    clear();

    if(data.count("file-info") and
       data["file-info"].count("document-hash"))
      {
        doc_name = data["file-info"].value("document-hash", doc_name);
        doc_hash = utils::to_hash(doc_name);
      }
    else
      {
        LOG_S(WARNING) << "no `file-info.document-hash detected ...`";

        doc_name = filepath.c_str();
        doc_hash = utils::to_hash(doc_name);
      }

    orig = data;

    if(data.count("main-text"))
      {
        uint64_t ind=0;
        for(auto& item:data["main-text"])
          {
            if(item.count("text")==1)
              {
                subject<PARAGRAPH> subj(doc_hash, ind);
                bool valid = subj.set_data(item);

                if(valid)
                  {
                    pind_to_orig.push_back(paragraphs.size());
                    paragraphs.push_back(subj);
                  }
              }

            ind += 1;
          }
      }
    else
      {
        LOG_S(WARNING) << "no `main-text` detected in pdf-document ...";
        return false;
      }

    if(data.count("tables"))
      {
        uint64_t ind=0;
        for(auto& item:data["tables"])
          {
            if(item.count("data")==1)
              {
                subject<TABLE> subj(doc_hash, ind);
                bool valid = subj.set_data(item);

                if(valid)
                  {
                    tind_to_orig.push_back(tables.size());
                    tables.push_back(subj);
                  }
              }

            ind += 1;
          }
      }
    else
      {
        LOG_S(WARNING) << "no `tables` detected in pdf-document ...";
        return false;
      }

    /*
      for(auto& item:paragraphs)
      {
      item.show(true, false, false, false, false, false, false);
      }

      for(auto& item:tables)
      {
      item.show(false, false, false);
      }

      LOG_S(WARNING) << "set doc ...";
      std::string tmp;
      std::cin >> tmp;
    */

    return true;
  }

  bool subject<DOCUMENT>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                                     std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    for(auto& paragraph:paragraphs)
      {
        bool valid = paragraph.set_tokens(char_normaliser, text_normaliser);

        if(not valid)
          {
            LOG_S(WARNING) << __FILE__ << ":" << __FUNCTION__
                           << " --> unvalid text detected in main-text";
          }
      }

    for(auto& table:tables)
      {
        bool valid = table.set_tokens(char_normaliser, text_normaliser);

        if(not valid)
          {
            LOG_S(WARNING) << __FILE__ << ":" << __FUNCTION__
                           << " --> unvalid text detected in main-text";
          }
      }

    return true;
  }

  bool subject<DOCUMENT>::set(nlohmann::json& data,
                              std::shared_ptr<utils::char_normaliser> char_normaliser,
                              std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    if(set_data(data))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }

  bool subject<DOCUMENT>::set(std::filesystem::path filepath, nlohmann::json& data,
                              std::shared_ptr<utils::char_normaliser> char_normaliser,
                              std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    if(set_data(filepath, data))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }

}

#endif
