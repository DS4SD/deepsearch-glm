//-*-C++-*-

#ifndef ANDROMEDA_UTILS_TEXT_TOKENIZER_H_
#define ANDROMEDA_UTILS_TEXT_TOKENIZER_H_

#include <sentencepiece_processor.h>

namespace andromeda
{
  namespace utils
  {
    class text_tokenizer
    {
    public:

      text_tokenizer(bool verbose);
      ~text_tokenizer();

      bool load_model(std::filesystem::path path);

      std::vector<int> encode(const std::string& text);
      std::string decode(const std::vector<int>& inds);


      
    private:

      std::shared_ptr<sentencepiece::SentencePieceProcessor> processor;
    };

    text_tokenizer::text_tokenizer(bool verbose)
    {
      std::filesystem::path resources_dir = glm_variables::get_resources_dir();
      std::filesystem::path tokenizer_path = resources_dir / "default-tokenizer.model";

      load_model(tokenizer_path);
    }

    bool text_tokenizer::load_model(std::filesystem::path path)
    {
      if(not std::filesystem::exists(path))
	{
	  LOG_S(ERROR) << "path does not exists: " << path; 
	  return false;
	}
      
      if(processor.use_count()==0)
	{
	  processor = std::make_shared<sentencepiece::SentencePieceProcessor>();
	}
      
      // const auto status = processor->Load(path.c_str());
      const auto status = processor->Load(path.string());


      if(!status.ok())
	{
	  LOG_S(ERROR) << status.ToString();
	  return false;
	}
      
      return true;
    }

    std::vector<int> text_tokenizer::encode(const std::string& text)
    {
      std::vector<int> result={};
      
      if(processor.use_count())
	{
	  return result;
	}

      processor->Encode(text, &result);
      //for (const int id : ids) {
      //std::cout << id << std::endl;
      //}
      
      return result;
    }

    std::string text_tokenizer::decode(const std::vector<int>& inds)
    {
      std::string result="";
      
      if(processor.use_count())
	{
	  return result;
	}

      processor->Decode(inds, &result);
      
      return result;
    }

  }

}

#endif
