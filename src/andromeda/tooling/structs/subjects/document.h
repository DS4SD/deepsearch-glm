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

    bool set_text(nlohmann::json& data);
    bool set_text(std::filesystem::path filepath,
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

    std::vector<std::size_t> pind_to_orig; // paragraphs-index to original maintext-index
    std::vector<std::size_t> tind_to_orig; // paragraphs-index to original tables-index

    std::vector<subject<PARAGRAPH> > paragraphs;
    std::vector<subject<TABLE> > tables;

    std::vector<base_property> properties;
    std::vector<base_entity> entities;
    std::vector<base_relation> relations;
  };

  subject<DOCUMENT>::subject():
    valid(true),
    applied_models(),

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


  bool subject<DOCUMENT>::set_text(nlohmann::json& data)
  {
    clear();

    orig = data;

    if(data.count("main-text"))
      {
        for(auto& item:data["main-text"])
          {
            if(item.count("text"))
              {
                std::string text="";
                text = item.value("text", text);

                subject<PARAGRAPH> subj;
                bool valid = subj.set_text(text);

                if(valid)
                  {
                    pind_to_orig.push_back(paragraphs.size());
                    paragraphs.push_back(subj);
                  }
              }
          }
      }
    else
      {
        LOG_S(WARNING) << "no `main-text` detected in pdf-document ...";
        return false;
      }

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

    return true;
  }

  bool subject<DOCUMENT>::set(nlohmann::json& data,
                              std::shared_ptr<utils::char_normaliser> char_normaliser,
                              std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    if(set_text(data))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }

  bool subject<DOCUMENT>::set_text(std::filesystem::path filepath, nlohmann::json& data)
  {
    this->filepath = filepath;
    return set_text(data);
  }

  bool subject<DOCUMENT>::set(std::filesystem::path filepath, nlohmann::json& data,
                              std::shared_ptr<utils::char_normaliser> char_normaliser,
                              std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    if(set_text(filepath, data))
      {
        return set_tokens(char_normaliser, text_normaliser);
      }

    return false;
  }



}

#endif
