//-*-C++-*-

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "libraries.h"
#include "andromeda.h"

namespace
{

  struct app_args
  {
    bool help = false;
    std::filesystem::path input_path;
    std::filesystem::path output_path;
    std::string models;
    std::size_t progress_every = 25;
  };

  std::vector<andromeda::model_name> available_model_names()
  {
    return {
      andromeda::SPM,
      andromeda::LAPOS,
      andromeda::LANGUAGE,
      andromeda::SEMANTIC,
      andromeda::NAME,
      andromeda::LINK,
      andromeda::CITE,
      andromeda::QUOTE,
      andromeda::NUMVAL,
      andromeda::GEOLOC,
      andromeda::PARENTHESIS,
      andromeda::EXPRESSION,
      andromeda::SENTENCE,
      andromeda::REFERENCE,
      andromeda::CUSTOM_SPM,
      andromeda::CUSTOM_CRF,
      andromeda::CUSTOM_FST,
      andromeda::MATERIAL,
      andromeda::CONN,
      andromeda::TERM,
      andromeda::VERB,
      andromeda::ABBREVIATION,
      andromeda::VAU,
      andromeda::METADATA
    };
  }

  void print_available_models()
  {
    std::cout << "\nAvailable models:\n";

    for(auto name:available_model_names())
      {
        std::cout << "  " << andromeda::to_key(name) << "\n";
      }

    std::cout << "\nModel expressions can be separated with ';' or ','.\n";
  }

  std::filesystem::path default_output_path(const std::filesystem::path& input_path)
  {
    auto output_path = input_path;
    output_path.replace_filename(input_path.stem().string() + ".nlp.dclx");
    return output_path;
  }

  bool parse_arguments(int argc, char *argv[], app_args& args)
  {
    cxxopts::Options options("nlp-on-dclx", "Apply Andromeda NLP models on a DocLang .dclx file");

    options.add_options()
      ("i,input", "input .dclx file", cxxopts::value<std::string>())
      ("m,models", "model expression, for example \"term;reference\"", cxxopts::value<std::string>())
      ("o,output", "output annotated .dclx file", cxxopts::value<std::string>()->default_value(""))
      ("progress-every", "log progress every N text elements; 0 disables periodic progress logs",
       cxxopts::value<std::size_t>()->default_value("25"))
      ("h,help", "print usage");

    auto parsed = options.parse(argc, argv);

    if(parsed.count("help"))
      {
        std::cout << options.help() << "\n";
        print_available_models();
        args.help = true;
        return true;
      }

    if(parsed.count("input")==0 or parsed.count("models")==0)
      {
        std::cout << options.help() << "\n";
        print_available_models();
        return false;
      }

    args.input_path = parsed["input"].as<std::string>();
    args.models = parsed["models"].as<std::string>();
    args.progress_every = parsed["progress-every"].as<std::size_t>();

    const std::string output = parsed["output"].as<std::string>();
    args.output_path = output.empty()? default_output_path(args.input_path):std::filesystem::path(output);

    return true;
  }

}

int main(int argc, char *argv[])
{
  loguru::init(argc, argv);

  app_args args;
  if(not parse_arguments(argc, argv, args))
    {
      return 1;
    }

  if(args.help)
    {
      return 0;
    }

  if(not std::filesystem::exists(args.input_path))
    {
      LOG_S(ERROR) << "input file does not exist: " << args.input_path.string();
      return 2;
    }

  if(andromeda::doclang::reader::detect_format(args.input_path)!=andromeda::doclang::format::dclx)
    {
      LOG_S(ERROR) << "input file must have .dclx extension: " << args.input_path.string();
      return 3;
    }

  const auto output_dir = args.output_path.parent_path();
  if(not output_dir.empty() and not std::filesystem::exists(output_dir))
    {
      std::filesystem::create_directories(output_dir);
    }

  andromeda::doclang::dclx_document doc;
  if(not andromeda::doclang::reader::read(args.input_path, doc))
    {
      LOG_S(ERROR) << "could not read DocLang archive: " << doc.get_last_error();
      return 4;
    }
  LOG_S(INFO) << "read DocLang archive: " << args.input_path.string();

  std::vector<std::shared_ptr<andromeda::base_nlp_model> > models;
  if(not andromeda::to_models(args.models, models, true))
    {
      LOG_S(ERROR) << "could not initialise models: " << args.models;
      return 5;
    }

  andromeda::doclang::nlp_apply_options apply_options;
  apply_options.document_name = args.input_path.string();
  apply_options.progress_every = args.progress_every;

  andromeda::doclang::nlp_apply_result apply_result;
  if(not andromeda::doclang::apply_models(doc, models, apply_options, apply_result))
    {
      LOG_S(ERROR) << "could not apply models on DocLang document: " << doc.get_last_error();
      return 6;
    }

  if(not andromeda::doclang::writer::write_dclx(args.output_path, doc))
    {
      LOG_S(ERROR) << "could not write DocLang archive: " << doc.get_last_error();
      return 7;
    }

  LOG_S(INFO) << "wrote annotated DocLang archive: " << args.output_path.string()
              << " text_elements=" << apply_result.text_elements
              << " properties=" << doc.get_properties().size()
              << " instances=" << doc.get_instances().size()
              << " relations=" << doc.get_relations().size();

  return 0;
}
