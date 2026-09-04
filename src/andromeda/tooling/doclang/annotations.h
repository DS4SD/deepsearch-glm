//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_ANNOTATIONS_H_
#define ANDROMEDA_TOOLING_DOCLANG_ANNOTATIONS_H_

#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>

#include <andromeda/tooling/doclang/dclx_document.h>

namespace andromeda::doclang
{

  const static inline std::string ANNOTATIONS_DIR = "annotations";
  const static inline std::string PROPERTIES_CSV = ANNOTATIONS_DIR + "/properties.csv";
  const static inline std::string INSTANCES_CSV = ANNOTATIONS_DIR + "/instances.csv";
  const static inline std::string ENTITIES_CSV = ANNOTATIONS_DIR + "/entities.csv";
  const static inline std::string RELATIONS_CSV = ANNOTATIONS_DIR + "/relations.csv";
  const static inline std::string EDGES_CSV = ANNOTATIONS_DIR + "/edges.csv";
  const static inline std::string DOCUMENT_REFERENCE_BIB = ANNOTATIONS_DIR + "/document_reference.bib";
  const static inline std::string REFERENCES_BIB = ANNOTATIONS_DIR + "/references.bib";
  const static inline std::string SUMMARY_DCLG = ANNOTATIONS_DIR + "/summary.dclg";
  const static inline std::string TOC_DCLG = ANNOTATIONS_DIR + "/toc.dclg";
  const static inline std::string CONCEPTS_DCLG = ANNOTATIONS_DIR + "/concepts.dclg";

  inline std::size_t element_count(pugi::xml_node parent, std::string_view name)
  {
    const std::string name_value(name);
    std::size_t count = 0;
    for(const auto child:parent.children(name_value.c_str()))
      {
        (void)child;
        count += 1;
      }
    return count;
  }

  // parses one DCLG sidecar, forwards parse errors to the owning DCLX
  // document, then applies the sidecar-specific validation
  template<typename validator_type>
  inline bool parse_sidecar_dclg(dclx_document& doc,
                                 std::string_view xml,
                                 const std::string& path,
                                 validator_type&& validate,
                                 std::shared_ptr<dclg_document>& out)
  {
    auto sidecar = std::make_shared<dclg_document>();
    if(not sidecar->read(xml))
      {
        doc.set_last_error(path + ": " + sidecar->get_last_error());
        return false;
      }

    std::string error;
    if(not validate(*sidecar, error))
      {
        doc.set_last_error(error);
        return false;
      }

    out = std::move(sidecar);
    return true;
  }

  inline bool validate_summary_dclg(const dclg_document&, std::string&)
  {
    // a valid DCLG root is all a summary sidecar has to satisfy
    return true;
  }

  inline bool validate_toc_dclg(const dclg_document& sidecar, std::string& error)
  {
    const auto root = sidecar.root();
    if(element_count(root, "toc")!=1)
      {
        error = TOC_DCLG + " must contain one <toc> element";
        return false;
      }

    for(const auto entry:root.child("toc").children("entry"))
      {
        if(std::string_view(entry.attribute("xpath").value()).empty() or
           element_count(entry, "description")!=1)
          {
            error = TOC_DCLG + " entries require xpath and one <description>";
            return false;
          }
      }

    return true;
  }

  inline bool validate_concepts_dclg(const dclg_document& sidecar, std::string& error)
  {
    const auto root = sidecar.root();
    if(element_count(root, "concepts")!=1)
      {
        error = CONCEPTS_DCLG + " must contain one <concepts> element";
        return false;
      }

    for(const auto concept_node:root.child("concepts").children("concept"))
      {
        if(element_count(concept_node, "header")!=1 or
           element_count(concept_node, "abbreviation")>1 or
           element_count(concept_node, "description")>1 or
           element_count(concept_node, "table")>1)
          {
            error = CONCEPTS_DCLG + " concepts require one <header> and allow at most one optional child";
            return false;
          }
      }

    return true;
  }

  inline bool set_summary_dclg(dclx_document& doc, std::string_view xml)
  {
    std::shared_ptr<dclg_document> sidecar;
    if(not parse_sidecar_dclg(doc, xml, SUMMARY_DCLG, validate_summary_dclg, sidecar))
      {
        return false;
      }

    doc.set_summary(std::move(sidecar));
    return true;
  }

  inline bool set_toc_dclg(dclx_document& doc, std::string_view xml)
  {
    std::shared_ptr<dclg_document> sidecar;
    if(not parse_sidecar_dclg(doc, xml, TOC_DCLG, validate_toc_dclg, sidecar))
      {
        return false;
      }

    doc.set_toc(std::move(sidecar));
    return true;
  }

  inline bool set_concepts_dclg(dclx_document& doc, std::string_view xml)
  {
    std::shared_ptr<dclg_document> sidecar;
    if(not parse_sidecar_dclg(doc, xml, CONCEPTS_DCLG, validate_concepts_dclg, sidecar))
      {
        return false;
      }

    doc.set_concepts(std::move(sidecar));
    return true;
  }

  inline std::string csv_escape(const std::string& value)
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

  inline std::string csv_row(const std::vector<std::string>& row)
  {
    std::ostringstream oss;
    for(std::size_t i=0; i<row.size(); i++)
      {
        if(i>0)
          {
            oss << ",";
          }

        oss << csv_escape(row.at(i));
      }

    oss << "\n";
    return oss.str();
  }

  inline std::vector<std::vector<std::string> > parse_csv(std::string_view text)
  {
    std::vector<std::vector<std::string> > rows;
    std::vector<std::string> row;
    std::string cell;
    bool in_quotes = false;

    for(std::size_t i=0; i<text.size(); i++)
      {
        const char c = text.at(i);

        if(in_quotes)
          {
            if(c=='"')
              {
                if(i+1<text.size() and text.at(i+1)=='"')
                  {
                    cell += '"';
                    i += 1;
                  }
                else
                  {
                    in_quotes = false;
                  }
              }
            else
              {
                cell += c;
              }

            continue;
          }

        if(c=='"')
          {
            in_quotes = true;
          }
        else if(c==',')
          {
            row.push_back(cell);
            cell.clear();
          }
        else if(c=='\n')
          {
            row.push_back(cell);
            cell.clear();
            rows.push_back(row);
            row.clear();
          }
        else if(c=='\r')
          {
          }
        else
          {
            cell += c;
          }
      }

    if(not cell.empty() or not row.empty())
      {
        row.push_back(cell);
        rows.push_back(row);
      }

    return rows;
  }

  inline bool headers_match(const std::vector<std::string>& lhs,
                            const std::vector<std::string>& rhs)
  {
    return lhs.size()==rhs.size() and
      std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  template<typename value_type>
  inline value_type parse_integral_cell(const std::string& cell)
  {
    if constexpr(std::is_signed_v<value_type>)
      {
        return static_cast<value_type>(std::stoll(cell));
      }
    else
      {
        return static_cast<value_type>(std::stoull(cell));
      }
  }

  inline nlohmann::json nullable_integral_cell(const std::string& cell)
  {
    if(cell.empty() or cell=="null")
      {
        return nlohmann::json::value_t::null;
      }

    return std::stoull(cell);
  }

  inline bool parse_bool_cell(const std::string& cell)
  {
    std::string lower = cell;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    return lower=="true" or lower=="1" or lower=="yes";
  }

  inline bool load_properties_csv(std::string_view csv,
                                  std::vector<base_property>& properties,
                                  std::string& error)
  {
    auto rows = parse_csv(csv);
    if(rows.empty())
      {
        return true;
      }

    if(not headers_match(rows.at(0), base_property::HEADERS))
      {
        error = "unexpected header in " + PROPERTIES_CSV;
        return false;
      }

    for(std::size_t i=1; i<rows.size(); i++)
      {
        const auto& row = rows.at(i);
        if(row.size()!=base_property::HEADERS.size())
          {
            error = "unexpected row width in " + PROPERTIES_CSV;
            return false;
          }

        nlohmann::json json_row = nlohmann::json::array({
            row.at(0),
            parse_integral_cell<base_types::hash_type>(row.at(1)),
            row.at(2),
            row.at(3),
            row.at(4),
            std::stof(row.at(5))
          });

        base_property prop;
        if(not prop.from_json_row(json_row))
          {
            error = "could not parse row in " + PROPERTIES_CSV;
            return false;
          }

        properties.push_back(prop);
      }

    return true;
  }

  inline bool load_instances_csv(std::string_view csv,
                                 std::vector<base_instance>& instances,
                                 std::string& error)
  {
    auto rows = parse_csv(csv);
    if(rows.empty())
      {
        return true;
      }

    if(not headers_match(rows.at(0), base_instance::HEADERS))
      {
        error = "unexpected header in " + INSTANCES_CSV;
        return false;
      }

    for(std::size_t i=1; i<rows.size(); i++)
      {
        const auto& row = rows.at(i);
        if(row.size()!=base_instance::HEADERS.size())
          {
            error = "unexpected row width in " + INSTANCES_CSV;
            return false;
          }

        nlohmann::json json_row = nlohmann::json::array({
            row.at(0),
            row.at(1),
            parse_integral_cell<base_types::hash_type>(row.at(2)),
            row.at(3),
            row.at(4),
            std::stof(row.at(5)),
            parse_integral_cell<base_types::hash_type>(row.at(6)),
            parse_integral_cell<base_types::hash_type>(row.at(7)),
            nullable_integral_cell(row.at(8)),
            nullable_integral_cell(row.at(9)),
            parse_integral_cell<base_types::ind_type>(row.at(10)),
            parse_integral_cell<base_types::ind_type>(row.at(11)),
            parse_integral_cell<base_types::ind_type>(row.at(12)),
            parse_integral_cell<base_types::ind_type>(row.at(13)),
            parse_integral_cell<base_types::ind_type>(row.at(14)),
            parse_integral_cell<base_types::ind_type>(row.at(15)),
            parse_bool_cell(row.at(16)),
            row.at(17),
            row.at(18)
          });

        base_instance inst;
        if(not inst.from_json_row(json_row))
          {
            error = "could not parse row in " + INSTANCES_CSV;
            return false;
          }

        instances.push_back(inst);
      }

    return true;
  }

  inline bool load_relations_csv(std::string_view csv,
                                 std::vector<base_relation>& relations,
                                 std::string& error)
  {
    auto rows = parse_csv(csv);
    if(rows.empty())
      {
        return true;
      }

    std::size_t offset = 0;
    auto headers = base_relation::headers();
    if(not headers_match(rows.at(0), headers))
      {
        headers.insert(headers.begin(), "doclang_xpath");
        if(not headers_match(rows.at(0), headers))
          {
            error = "unexpected header in " + RELATIONS_CSV;
            return false;
          }

        offset = 1;
      }

    for(std::size_t i=1; i<rows.size(); i++)
      {
        const auto& row = rows.at(i);
        if(row.size()!=base_relation::headers().size()+offset)
          {
            error = "unexpected row width in " + RELATIONS_CSV;
            return false;
          }

        nlohmann::json json_row = nlohmann::json::array({
            parse_integral_cell<base_types::flvr_type>(row.at(offset+0)),
            row.at(offset+1),
            std::stof(row.at(offset+2)),
            parse_integral_cell<base_types::hash_type>(row.at(offset+3)),
            parse_integral_cell<base_types::hash_type>(row.at(offset+4)),
            row.at(offset+5),
            row.at(offset+6)
          });

        base_relation rel;
        if(not rel.from_json_row(json_row))
          {
            error = "could not parse row in " + RELATIONS_CSV;
            return false;
          }

        relations.push_back(rel);
      }

    return true;
  }

  inline bool load_entities_csv(std::string_view csv,
                                std::vector<base_entity>& entities,
                                std::string& error)
  {
    auto rows = parse_csv(csv);
    if(rows.empty())
      {
        return true;
      }

    if(not headers_match(rows.at(0), base_entity::HEADERS))
      {
        error = "unexpected header in " + ENTITIES_CSV;
        return false;
      }

    for(std::size_t i=1; i<rows.size(); i++)
      {
        const auto& row = rows.at(i);
        if(row.size()!=base_entity::HEADERS.size())
          {
            error = "unexpected row width in " + ENTITIES_CSV;
            return false;
          }

        nlohmann::json json_row = nlohmann::json::array({
            row.at(0),
            row.at(1),
            row.at(2),
            row.at(3),
            parse_integral_cell<base_types::hash_type>(row.at(4)),
            parse_integral_cell<base_types::cnt_type>(row.at(5)),
            row.at(6),
            parse_integral_cell<base_types::hash_type>(row.at(7))
          });

        base_entity ent;
        if(not ent.from_json_row(json_row))
          {
            error = "could not parse row in " + ENTITIES_CSV;
            return false;
          }

        entities.push_back(ent);
      }

    return true;
  }

  inline bool load_edges_csv(std::string_view csv,
                             std::vector<base_graph_edge>& edges,
                             std::string& error)
  {
    auto rows = parse_csv(csv);
    if(rows.empty())
      {
        return true;
      }

    if(not headers_match(rows.at(0), base_graph_edge::HEADERS))
      {
        error = "unexpected header in " + EDGES_CSV;
        return false;
      }

    for(std::size_t i=1; i<rows.size(); i++)
      {
        const auto& row = rows.at(i);
        if(row.size()!=base_graph_edge::HEADERS.size())
          {
            error = "unexpected row width in " + EDGES_CSV;
            return false;
          }

        nlohmann::json json_row = nlohmann::json::array({
            parse_integral_cell<base_types::hash_type>(row.at(0)),
            parse_integral_cell<base_types::flvr_type>(row.at(1)),
            row.at(2),
            parse_integral_cell<base_types::hash_type>(row.at(3)),
            parse_integral_cell<base_types::hash_type>(row.at(4)),
            parse_integral_cell<base_types::cnt_type>(row.at(5)),
            std::stof(row.at(6))
          });

        base_graph_edge edge;
        if(not edge.from_json_row(json_row))
          {
            error = "could not parse row in " + EDGES_CSV;
            return false;
          }

        edges.push_back(edge);
      }

    return true;
  }

  inline bool load_annotations(dclx_document& doc)
  {
    doc.clear_annotations();

    if(not doc.has_archive())
      {
        return true;
      }

    std::string error;

    auto properties_csv = doc.artifacts().text(PROPERTIES_CSV);
    if(properties_csv.has_value() and
       not load_properties_csv(properties_csv.value(), doc.mutable_properties(), error))
      {
        doc.set_last_error(error);
        return false;
      }

    auto instances_csv = doc.artifacts().text(INSTANCES_CSV);
    if(instances_csv.has_value() and
       not load_instances_csv(instances_csv.value(), doc.mutable_instances(), error))
      {
        doc.set_last_error(error);
        return false;
      }

    auto entities_csv = doc.artifacts().text(ENTITIES_CSV);
    if(entities_csv.has_value() and
       not load_entities_csv(entities_csv.value(), doc.mutable_entities(), error))
      {
        doc.set_last_error(error);
        return false;
      }

    auto relations_csv = doc.artifacts().text(RELATIONS_CSV);
    if(relations_csv.has_value() and
       not load_relations_csv(relations_csv.value(), doc.mutable_relations(), error))
      {
        doc.set_last_error(error);
        return false;
      }

    auto edges_csv = doc.artifacts().text(EDGES_CSV);
    if(edges_csv.has_value() and
       not load_edges_csv(edges_csv.value(), doc.mutable_edges(), error))
      {
        doc.set_last_error(error);
        return false;
      }

    auto document_reference = doc.artifacts().text(DOCUMENT_REFERENCE_BIB);
    if(document_reference.has_value())
      {
        doc.set_document_reference(std::string(document_reference.value()));
      }

    auto references = doc.artifacts().text(REFERENCES_BIB);
    if(references.has_value())
      {
        doc.set_references(std::string(references.value()));
      }

    auto summary = doc.artifacts().text(SUMMARY_DCLG);
    if(summary.has_value() and not set_summary_dclg(doc, summary.value()))
      {
        return false;
      }

    auto toc = doc.artifacts().text(TOC_DCLG);
    if(toc.has_value() and not set_toc_dclg(doc, toc.value()))
      {
        return false;
      }

    auto concepts = doc.artifacts().text(CONCEPTS_DCLG);
    if(concepts.has_value() and not set_concepts_dclg(doc, concepts.value()))
      {
        return false;
      }

    if(not entities_csv.has_value() and not doc.get_instances().empty())
      {
        doc.compute_entities();
      }

    return true;
  }

  inline std::string to_properties_csv(std::vector<base_property>& properties)
  {
    std::ostringstream oss;
    oss << csv_row(base_property::HEADERS);

    for(auto& prop:properties)
      {
        oss << csv_row(prop.to_row());
      }

    return oss.str();
  }

  inline std::string to_instances_csv(std::vector<base_instance>& instances)
  {
    std::ostringstream oss;
    oss << csv_row(base_instance::HEADERS);

    for(auto& inst:instances)
      {
        std::vector<std::string> row;
        auto json_row = inst.to_json_row();
        row.reserve(json_row.size());

        for(const auto& value:json_row)
          {
            if(value.is_null())
              {
                row.push_back("");
              }
            else if(value.is_string())
              {
                row.push_back(value.get<std::string>());
              }
            else if(value.is_boolean())
              {
                row.push_back(value.get<bool>()? "true":"false");
              }
            else
              {
                row.push_back(value.dump());
              }
          }

        oss << csv_row(row);
      }

    return oss.str();
  }

  inline std::string to_entities_csv(std::vector<base_entity>& entities)
  {
    std::ostringstream oss;
    oss << csv_row(base_entity::HEADERS);

    for(auto& ent:entities)
      {
        oss << csv_row(ent.to_row());
      }

    return oss.str();
  }

  inline std::string to_relations_csv(std::vector<base_relation>& relations)
  {
    std::ostringstream oss;
    oss << csv_row(base_relation::headers());

    for(auto& rel:relations)
      {
        oss << csv_row(rel.to_row(0));
      }

    return oss.str();
  }

  inline std::string to_edges_csv(std::vector<base_graph_edge>& edges)
  {
    std::ostringstream oss;
    oss << csv_row(base_graph_edge::HEADERS);

    for(auto& edge:edges)
      {
        oss << csv_row(edge.to_row());
      }

    return oss.str();
  }

}

#endif
