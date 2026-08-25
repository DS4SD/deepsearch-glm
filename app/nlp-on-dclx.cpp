//-*-C++-*-

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "libraries.h"
#include "andromeda.h"
#include "andromeda/tooling/doclang.h"

namespace
{

  struct app_args
  {
    bool help = false;
    std::filesystem::path input_path;
    std::filesystem::path output_dir;
    std::string models;
    std::size_t progress_every = 25;
  };

  using steady_clock = std::chrono::steady_clock;

  uint64_t elapsed_ms(steady_clock::time_point start)
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      steady_clock::now()-start).count();
  }

  struct relation_row
  {
    std::string doclang_xpath;
    andromeda::base_relation relation;
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

  std::string json_cell_to_string(const nlohmann::json& value)
  {
    if(value.is_null())
      {
        return "";
      }

    if(value.is_string())
      {
        return value.get<std::string>();
      }

    if(value.is_boolean())
      {
        return value.get<bool>()? "true":"false";
      }

    return value.dump();
  }

  std::vector<std::string> json_row_to_strings(const nlohmann::json& row)
  {
    std::vector<std::string> result;
    result.reserve(row.size());

    for(const auto& value:row)
      {
        result.push_back(json_cell_to_string(value));
      }

    return result;
  }

  std::string normalise_doclang_path(const std::string& path)
  {
    if(path.size()>1 and path.at(0)=='#' and path.at(1)=='/')
      {
        return path.substr(1);
      }

    return path;
  }

  std::string csv_escape(const std::string& value)
  {
    bool needs_quotes = false;
    for(char c:value)
      {
        if(c==',' or c=='"' or c=='\n' or c=='\r')
          {
            needs_quotes = true;
            break;
          }
      }

    if(not needs_quotes)
      {
        return value;
      }

    std::string result = "\"";
    for(char c:value)
      {
        if(c=='"')
          {
            result += "\"\"";
          }
        else
          {
            result += c;
          }
      }
    result += "\"";

    return result;
  }

  void write_csv_row(std::ofstream& ofs, const std::vector<std::string>& row)
  {
    for(std::size_t i=0; i<row.size(); i++)
      {
        if(i>0)
          {
            ofs << ",";
          }

        ofs << csv_escape(row.at(i));
      }

    ofs << "\n";
  }

  bool write_properties_csv(const std::filesystem::path& path,
                            std::vector<andromeda::base_property>& properties)
  {
    std::ofstream ofs(path);
    if(not ofs)
      {
        LOG_S(ERROR) << "could not open output file: " << path.string();
        return false;
      }

    write_csv_row(ofs, andromeda::base_property::HEADERS);

    for(auto& prop:properties)
      {
        auto row = prop.to_row();
        row.at(3) = normalise_doclang_path(row.at(3));
        write_csv_row(ofs, row);
      }

    return true;
  }

  bool write_instances_csv(const std::filesystem::path& path,
                           std::vector<andromeda::base_instance>& instances)
  {
    std::ofstream ofs(path);
    if(not ofs)
      {
        LOG_S(ERROR) << "could not open output file: " << path.string();
        return false;
      }

    write_csv_row(ofs, andromeda::base_instance::HEADERS);

    for(auto& inst:instances)
      {
        auto row = json_row_to_strings(inst.to_json_row());
        row.at(4) = normalise_doclang_path(row.at(4));
        write_csv_row(ofs, row);
      }

    return true;
  }

  bool write_relations_csv(const std::filesystem::path& path,
                           std::vector<relation_row>& relations)
  {
    std::ofstream ofs(path);
    if(not ofs)
      {
        LOG_S(ERROR) << "could not open output file: " << path.string();
        return false;
      }

    auto headers = andromeda::base_relation::headers();
    headers.insert(headers.begin(), "doclang_xpath");
    write_csv_row(ofs, headers);

    for(auto& rel:relations)
      {
        auto row = rel.relation.to_row(0);
        row.insert(row.begin(), rel.doclang_xpath);
        write_csv_row(ofs, row);
      }

    return true;
  }

  bool parse_arguments(int argc, char *argv[], app_args& args)
  {
    cxxopts::Options options("nlp-on-dclx", "Apply Andromeda NLP models on a DocLang .dclx file");

    options.add_options()
      ("i,input", "input .dclx file", cxxopts::value<std::string>())
      ("m,models", "model expression, for example \"term;reference\"", cxxopts::value<std::string>())
      ("o,output-dir", "output directory for CSV files", cxxopts::value<std::string>()->default_value(""))
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

    std::string output_dir = parsed["output-dir"].as<std::string>();
    if(output_dir.empty())
      {
        args.output_dir = args.input_path.parent_path();
      }
    else
      {
        args.output_dir = output_dir;
      }

    if(args.output_dir.empty())
      {
        args.output_dir = ".";
      }

    return true;
  }

  nlohmann::json payload_for(pugi::xml_node node, const std::string& xpath)
  {
    nlohmann::json payload = nlohmann::json::object({});

    payload["doclang_xpath"] = xpath;
    payload["doclang_name"] = node.name();

    if(node.attribute("level"))
      {
        payload["doclang_heading_level"] = node.attribute("level").as_uint();
      }

    nlohmann::json locations = nlohmann::json::array({});
    for(pugi::xml_node loc:node.children("location"))
      {
        if(loc.attribute("value"))
          {
            locations.push_back(loc.attribute("value").as_double());
          }
      }

    if(not locations.empty())
      {
        payload["doclang_location"] = locations;
      }

    return payload;
  }

  std::string indexed_xpath(pugi::xml_node node)
  {
    std::vector<std::string> parts;

    for(pugi::xml_node curr=node; curr; curr=curr.parent())
      {
        if(curr.type()!=pugi::node_element)
          {
            continue;
          }

        std::size_t index = 1;
        for(pugi::xml_node sibling=curr.previous_sibling(curr.name());
            sibling;
            sibling=sibling.previous_sibling(curr.name()))
          {
            index += 1;
          }

        std::stringstream ss;
        ss << "/" << curr.name() << "[" << index << "]";
        parts.push_back(ss.str());
      }

    std::reverse(parts.begin(), parts.end());

    std::string result;
    for(const auto& part:parts)
      {
        result += part;
      }

    return result;
  }

  std::string indexed_text_xpath(pugi::xml_node text_node)
  {
    const std::string parent_path = indexed_xpath(text_node.parent());
    std::size_t index = 1;

    for(pugi::xml_node sibling=text_node.previous_sibling();
        sibling;
        sibling=sibling.previous_sibling())
      {
        if((sibling.type()==pugi::node_pcdata or sibling.type()==pugi::node_cdata) and
           not andromeda::doclang::is_xml_whitespace(sibling.value()))
          {
            index += 1;
          }
      }

    std::stringstream ss;
    ss << parent_path << "/text()[" << index << "]";
    return ss.str();
  }

  void append_items(andromeda::subject<andromeda::TEXT>& subject,
                    const std::string& doclang_xpath,
                    std::vector<andromeda::base_property>& properties,
                    std::vector<andromeda::base_instance>& instances,
                    std::vector<relation_row>& relations)
  {
    for(auto& prop:subject.properties)
      {
        if(prop.get_subj_path()=="#")
          {
            properties.emplace_back(prop.get_subj_hash(),
                                    prop.get_subj_name(),
                                    doclang_xpath,
                                    prop.get_model(),
                                    prop.get_label(),
                                    prop.get_conf());
          }
        else
          {
            properties.push_back(prop);
          }
      }

    for(auto& inst:subject.instances)
      {
        instances.push_back(inst);
      }

    for(auto& rel:subject.relations)
      {
        relations.push_back({doclang_xpath, rel});
      }
  }

  void process_text_subject(
    const std::vector<std::shared_ptr<andromeda::base_nlp_model> >& models,
    const std::shared_ptr<andromeda::utils::char_normaliser>& char_normaliser,
    const std::shared_ptr<andromeda::utils::text_normaliser>& text_normaliser,
    uint64_t doc_hash,
    const std::string& doc_name,
    pugi::xml_node metadata_node,
    const std::string& subject_type,
    const std::string& text,
    const std::string& subject_path,
    bool log_element,
    std::size_t text_index,
    std::size_t visited_elements,
    std::vector<andromeda::base_property>& properties,
    std::vector<andromeda::base_instance>& instances,
    std::vector<relation_row>& relations)
  {
    if(log_element)
      {
        LOG_S(INFO) << "processing DocLang text element "
                    << "text_index=" << text_index
                    << " visited=" << visited_elements
                    << " name=" << subject_type
                    << " xpath=" << subject_path
                    << " chars=" << text.size();
      }

    const auto element_start = steady_clock::now();
    const std::string subject_dloc = doc_name + "#" + subject_path;
    andromeda::subject<andromeda::TEXT> subject(doc_hash, subject_dloc);
    if(not subject.set_text(text))
      {
        return;
      }

    subject.set_type(subject_type);
    subject.payload = payload_for(metadata_node, subject_path);

    subject.set_tokens(char_normaliser, text_normaliser);

    for(auto& model:models)
      {
        const auto model_start = steady_clock::now();
        if(log_element)
          {
            LOG_S(INFO) << "applying model "
                        << andromeda::to_key(model->get_name())
                        << " to xpath=" << subject_path;
          }

        model->apply(subject);

        const auto model_elapsed = elapsed_ms(model_start);
        if(log_element or model_elapsed>2000)
          {
            LOG_S(INFO) << "applied model "
                        << andromeda::to_key(model->get_name())
                        << " to xpath=" << subject_path
                        << " elapsed_ms=" << model_elapsed
                        << " properties=" << subject.properties.size()
                        << " instances=" << subject.instances.size()
                        << " relations=" << subject.relations.size();
          }
      }

    append_items(subject, subject_path, properties, instances, relations);

    const auto element_elapsed = elapsed_ms(element_start);
    if(log_element or element_elapsed>5000)
      {
        LOG_S(INFO) << "finished DocLang text element "
                    << "text_index=" << text_index
                    << " xpath=" << subject_path
                    << " elapsed_ms=" << element_elapsed
                    << " total_properties=" << properties.size()
                    << " total_instances=" << instances.size()
                    << " total_relations=" << relations.size();
      }
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

  if(not std::filesystem::exists(args.output_dir))
    {
      std::filesystem::create_directories(args.output_dir);
    }

  andromeda::doclang::document doc;
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

  auto char_normaliser = std::make_shared<andromeda::utils::char_normaliser>(false);
  auto text_normaliser = std::make_shared<andromeda::utils::text_normaliser>(false);

  std::vector<andromeda::base_property> properties;
  std::vector<andromeda::base_instance> instances;
  std::vector<relation_row> relations;

  const auto doc_hash = andromeda::utils::to_reproducible_hash(args.input_path.string());
  const std::string doc_name = args.input_path.string();

  std::size_t element_index = 0;
  std::size_t visited_elements = 0;
  std::size_t text_elements = 0;
  std::size_t skipped_empty_elements = 0;
  const auto iteration_start = steady_clock::now();

  LOG_S(INFO) << "starting DocLang element iteration";
  doc.iterate_elements([&](pugi::xml_node node)
  {
    visited_elements += 1;

    if(std::string_view(node.name())=="table")
      {
        doc.iterate_table_text(node, [&](pugi::xml_node table, pugi::xml_node text_node)
        {
          const std::string text =
            andromeda::utils::strip(std::string(text_node.value()));
          if(text.empty())
            {
              return;
            }

          text_elements += 1;

          const std::string subject_path = indexed_text_xpath(text_node);
          const bool log_element =
            (args.progress_every>0 and text_elements%args.progress_every==0) or
            text_elements==1 or
            text.size()>10000;

          process_text_subject(models,
                               char_normaliser,
                               text_normaliser,
                               doc_hash,
                               doc_name,
                               table,
                               "table_text",
                               text,
                               subject_path,
                               log_element,
                               text_elements,
                               visited_elements,
                               properties,
                               instances,
                               relations);

          element_index += 1;
        });

        return;
      }

    if(std::string_view(node.name())=="picture" or std::string_view(node.name())=="figure")
      {
        doc.iterate_picture_text(node, [&](pugi::xml_node picture, pugi::xml_node text_node)
        {
          const std::string text =
            andromeda::utils::strip(andromeda::doclang::direct_text_content(text_node));
          if(text.empty())
            {
              return;
            }

          text_elements += 1;

          const std::string subject_path = indexed_xpath(text_node);
          const bool log_element =
            (args.progress_every>0 and text_elements%args.progress_every==0) or
            text_elements==1 or
            text.size()>10000;

          process_text_subject(models,
                               char_normaliser,
                               text_normaliser,
                               doc_hash,
                               doc_name,
                               picture,
                               text_node.name(),
                               text,
                               subject_path,
                               log_element,
                               text_elements,
                               visited_elements,
                               properties,
                               instances,
                               relations);

          element_index += 1;
        });

        return;
      }

    const std::string text =
      andromeda::utils::strip(andromeda::doclang::direct_text_content(node));
    if(text.empty())
      {
        skipped_empty_elements += 1;
        return;
      }

    const std::string xpath = indexed_xpath(node);
    std::string subject_path = xpath;
    if(subject_path.empty())
      {
        std::stringstream ss;
        ss << "#/doclang/" << element_index;
        subject_path = ss.str();
      }

    text_elements += 1;

    const bool log_element =
      (args.progress_every>0 and text_elements%args.progress_every==0) or
      text_elements==1 or
      text.size()>10000;

    process_text_subject(models,
                         char_normaliser,
                         text_normaliser,
                         doc_hash,
                         doc_name,
                         node,
                         node.name(),
                         text,
                         subject_path,
                         log_element,
                         text_elements,
                         visited_elements,
                         properties,
                         instances,
                         relations);

    element_index += 1;
  });

  LOG_S(INFO) << "finished DocLang element iteration"
              << " visited=" << visited_elements
              << " text_elements=" << text_elements
              << " skipped_empty=" << skipped_empty_elements
              << " elapsed_ms=" << elapsed_ms(iteration_start);

  const std::filesystem::path stem = args.input_path.stem();

  const std::filesystem::path properties_path =
    args.output_dir / (stem.string() + ".properties.csv");
  const std::filesystem::path instances_path =
    args.output_dir / (stem.string() + ".instances.csv");
  const std::filesystem::path relations_path =
    args.output_dir / (stem.string() + ".relations.csv");

  const bool wrote_properties = write_properties_csv(properties_path, properties);
  const bool wrote_instances = write_instances_csv(instances_path, instances);
  const bool wrote_relations = write_relations_csv(relations_path, relations);

  if(not (wrote_properties and wrote_instances and wrote_relations))
    {
      return 6;
    }

  LOG_S(INFO) << "wrote properties: " << properties_path.string()
              << " rows=" << properties.size();
  LOG_S(INFO) << "wrote instances: " << instances_path.string()
              << " rows=" << instances.size();
  LOG_S(INFO) << "wrote relations: " << relations_path.string()
              << " rows=" << relations.size();

  return 0;
}
