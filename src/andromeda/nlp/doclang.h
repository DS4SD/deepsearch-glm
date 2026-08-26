//-*-C++-*-

#ifndef ANDROMEDA_NLP_DOCLANG_H_
#define ANDROMEDA_NLP_DOCLANG_H_

#include <algorithm>
#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <andromeda/tooling/doclang.h>

namespace andromeda::doclang
{

  struct nlp_apply_options
  {
    std::string document_name;
    std::size_t progress_every = 25;
  };

  struct nlp_apply_result
  {
    std::size_t visited_elements = 0;
    std::size_t text_elements = 0;
    std::size_t skipped_empty_elements = 0;
  };

  inline uint64_t elapsed_ms(std::chrono::steady_clock::time_point start)
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now()-start).count();
  }

  inline nlohmann::json payload_for(pugi::xml_node node, const std::string& xpath)
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

  inline std::string indexed_xpath(pugi::xml_node node)
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

  inline std::string indexed_text_xpath(pugi::xml_node text_node)
  {
    const std::string parent_path = indexed_xpath(text_node.parent());
    std::size_t index = 1;

    for(pugi::xml_node sibling=text_node.previous_sibling();
        sibling;
        sibling=sibling.previous_sibling())
      {
        if((sibling.type()==pugi::node_pcdata or sibling.type()==pugi::node_cdata) and
           not is_xml_whitespace(sibling.value()))
          {
            index += 1;
          }
      }

    std::stringstream ss;
    ss << parent_path << "/text()[" << index << "]";
    return ss.str();
  }

  inline void append_items(subject<TEXT>& subject,
                           const std::string& doclang_xpath,
                           std::vector<base_property>& properties,
                           std::vector<base_instance>& instances,
                           std::vector<base_relation>& relations)
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
        relations.push_back(rel);
      }
  }

  inline void process_text_subject(
    const std::vector<std::shared_ptr<base_nlp_model> >& models,
    const std::shared_ptr<utils::char_normaliser>& char_normaliser,
    const std::shared_ptr<utils::text_normaliser>& text_normaliser,
    uint64_t doc_hash,
    const std::string& doc_name,
    pugi::xml_node metadata_node,
    const std::string& subject_type,
    const std::string& text,
    const std::string& subject_path,
    bool log_element,
    std::size_t text_index,
    std::size_t visited_elements,
    std::vector<base_property>& properties,
    std::vector<base_instance>& instances,
    std::vector<base_relation>& relations)
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

    const auto element_start = std::chrono::steady_clock::now();
    const std::string subject_dloc = doc_name + "#" + subject_path;
    subject<TEXT> subj(doc_hash, subject_dloc);
    if(not subj.set_text(text))
      {
        return;
      }

    subj.set_type(subject_type);
    subj.payload = payload_for(metadata_node, subject_path);

    subj.set_tokens(char_normaliser, text_normaliser);

    for(auto& model:models)
      {
        const auto model_start = std::chrono::steady_clock::now();
        if(log_element)
          {
            LOG_S(INFO) << "applying model "
                        << to_key(model->get_name())
                        << " to xpath=" << subject_path;
          }

        model->apply(subj);

        const auto model_elapsed = elapsed_ms(model_start);
        if(log_element or model_elapsed>2000)
          {
            LOG_S(INFO) << "applied model "
                        << to_key(model->get_name())
                        << " to xpath=" << subject_path
                        << " elapsed_ms=" << model_elapsed
                        << " properties=" << subj.properties.size()
                        << " instances=" << subj.instances.size()
                        << " relations=" << subj.relations.size();
          }
      }

    append_items(subj, subject_path, properties, instances, relations);

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

  inline bool apply_models(document& doc,
                           const std::vector<std::shared_ptr<base_nlp_model> >& models,
                           const nlp_apply_options& options,
                           nlp_apply_result& result)
  {
    doc.clear_annotations();

    auto char_normaliser = std::make_shared<utils::char_normaliser>(false);
    auto text_normaliser = std::make_shared<utils::text_normaliser>(false);

    const std::string doc_name =
      options.document_name.empty()? doc.get_source_path().string():options.document_name;
    const auto doc_hash = utils::to_reproducible_hash(doc_name);

    std::size_t element_index = 0;
    const auto iteration_start = std::chrono::steady_clock::now();

    LOG_S(INFO) << "starting DocLang element iteration";
    doc.iterate_elements([&](pugi::xml_node node)
    {
      result.visited_elements += 1;

      if(std::string_view(node.name())=="table")
        {
          doc.iterate_table_text(node, [&](pugi::xml_node table, pugi::xml_node text_node)
          {
            const std::string text = utils::strip(std::string(text_node.value()));
            if(text.empty())
              {
                return;
              }

            result.text_elements += 1;
            const std::string subject_path = indexed_text_xpath(text_node);
            const bool log_element =
              (options.progress_every>0 and
               result.text_elements%options.progress_every==0) or
              result.text_elements==1 or text.size()>10000;

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
                                 result.text_elements,
                                 result.visited_elements,
                                 doc.mutable_properties(),
                                 doc.mutable_instances(),
                                 doc.mutable_relations());

            element_index += 1;
          });

          return;
        }

      if(is_picture_like_name(node.name()))
        {
          doc.iterate_picture_text(node, [&](pugi::xml_node picture, pugi::xml_node text_node)
          {
            const std::string text = utils::strip(direct_text_content(text_node));
            if(text.empty())
              {
                return;
              }

            result.text_elements += 1;
            const std::string subject_path = indexed_xpath(text_node);
            const bool log_element =
              (options.progress_every>0 and
               result.text_elements%options.progress_every==0) or
              result.text_elements==1 or text.size()>10000;

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
                                 result.text_elements,
                                 result.visited_elements,
                                 doc.mutable_properties(),
                                 doc.mutable_instances(),
                                 doc.mutable_relations());

            element_index += 1;
          });

          return;
        }

      const std::string text = utils::strip(direct_text_content(node));
      if(text.empty())
        {
          result.skipped_empty_elements += 1;
          return;
        }

      std::string subject_path = indexed_xpath(node);
      if(subject_path.empty())
        {
          std::stringstream ss;
          ss << "#/doclang/" << element_index;
          subject_path = ss.str();
        }

      result.text_elements += 1;
      const bool log_element =
        (options.progress_every>0 and result.text_elements%options.progress_every==0) or
        result.text_elements==1 or text.size()>10000;

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
                           result.text_elements,
                           result.visited_elements,
                           doc.mutable_properties(),
                           doc.mutable_instances(),
                           doc.mutable_relations());

      element_index += 1;
    });

    LOG_S(INFO) << "finished DocLang element iteration"
                << " visited=" << result.visited_elements
                << " text_elements=" << result.text_elements
                << " skipped_empty=" << result.skipped_empty_elements
                << " elapsed_ms=" << elapsed_ms(iteration_start);

    return true;
  }

}

#endif
