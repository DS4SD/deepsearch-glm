//-*-C++-*-

#include <cassert>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <miniz/miniz.h>

#include "libraries.h"
#include "andromeda/tooling/doclang.h"
#include "andromeda/tooling/doclang/adapters.h"

namespace
{

  bool write_text_file(const std::filesystem::path& path, const std::string& text)
  {
    std::ofstream ofs(path, std::ios::binary);
    if(not ofs)
      {
        return false;
      }

    ofs.write(text.data(), static_cast<std::streamsize>(text.size()));
    return static_cast<bool>(ofs);
  }

  bool create_dclx(std::vector<std::byte>& out, const std::string& document_xml)
  {
    mz_zip_archive zip;
    std::memset(&zip, 0, sizeof(zip));

    if(!mz_zip_writer_init_heap(&zip, 0, 0))
      {
        return false;
      }

    const std::string content_types = "<Types></Types>";
    const std::string rels = "<Relationships></Relationships>";
    const std::string asset = "artifact-bytes";
    const std::string page = "page-bytes";

    bool success = true;
    success = success and mz_zip_writer_add_mem(&zip, "[Content_Types].xml",
                                                content_types.data(), content_types.size(), 0);
    success = success and mz_zip_writer_add_mem(&zip, "_rels/.rels",
                                                rels.data(), rels.size(), 0);
    success = success and mz_zip_writer_add_mem(&zip, "document.xml",
                                                document_xml.data(), document_xml.size(), 0);
    success = success and mz_zip_writer_add_mem(&zip, "assets/image_000001.png",
                                                asset.data(), asset.size(), 0);
    success = success and mz_zip_writer_add_mem(&zip, "pages/1.png",
                                                page.data(), page.size(), 0);

    void* raw = nullptr;
    size_t size = 0;

    if(success)
      {
        success = mz_zip_writer_finalize_heap_archive(&zip, &raw, &size);
      }

    mz_zip_writer_end(&zip);

    if(not success or raw==nullptr)
      {
        if(raw!=nullptr)
          {
            mz_free(raw);
          }
        return false;
      }

    out.resize(size);
    std::memcpy(out.data(), raw, size);
    mz_free(raw);

    return true;
  }

  int test_read_dclg()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<heading level=\"1\"><location value=\"1\"/>Title</heading>"
      "<text><![CDATA[DocLang text]]></text>"
      "</doclang>";

    const auto path = std::filesystem::temp_directory_path() / "docling-nlp-test-unit-doclang.dclg";
    const bool wrote_xml = write_text_file(path, xml);
    if(not wrote_xml)
      {
        return 1;
      }
    assert(wrote_xml);

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read(path, doc));
    assert(doc.root());
    assert(std::string(doc.root().attribute("version").value())=="0.7");
    assert(std::string(doc.root().child("heading").attribute("level").value())=="1");
    assert(std::string(doc.root().child("text").child_value())=="DocLang text");
    assert(not doc.has_archive());

    std::filesystem::remove(path);
    return 0;
  }

  int test_read_dclx_buffer()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<text>Archive text</text>"
      "</doclang>";

    std::vector<std::byte> bytes;
    const bool created_archive = create_dclx(bytes, xml);
    if(not created_archive)
      {
        return 1;
      }
    assert(created_archive);

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclx_buffer(bytes, doc));
    assert(doc.root());
    assert(doc.has_archive());
    assert(doc.artifacts().has("document.xml"));
    assert(doc.artifacts().has("assets/image_000001.png"));
    assert(doc.artifacts().has("pages/1.png"));
    assert(std::string(doc.root().child("text").child_value())=="Archive text");

    auto asset = doc.artifacts().text("assets/image_000001.png");
    if(not asset.has_value() or asset.value()!="artifact-bytes")
      {
        return 1;
      }
    assert(asset.has_value());
    assert(asset.value()=="artifact-bytes");

    return 0;
  }

  int test_read_dclx_annotations()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<text>Archive text</text>"
      "</doclang>";

    andromeda::doclang::archive zip;
    zip.set_text("[Content_Types].xml", "<Types></Types>");
    zip.set_text("_rels/.rels", "<Relationships></Relationships>");
    zip.set_text("document.xml", xml);
    zip.set_text(andromeda::doclang::PROPERTIES_CSV,
                 "type,subj_hash,subj_name,subj_path,label,confidence\n"
                 "language,123,text,/doclang[1]/text[1],en,0.99\n");
    zip.set_text(andromeda::doclang::INSTANCES_CSV,
                 "type,subtype,subj_hash,subj_name,subj_path,conf,hash,ihash,coor_i,coor_j,char_i,char_j,ctok_i,ctok_j,wtok_i,wtok_j,wtok-match,name,original\n"
                 "term,,123,text,/doclang[1]/text[1],1,456,789,,,0,7,0,1,0,1,true,Archive,Archive\n");
    zip.set_text(andromeda::doclang::RELATIONS_CSV,
                 "flvr,name,conf,hash_i,hash_j,name_i,name_j\n"
                 "42,contains,0.75,456,789,Archive,text\n");

    std::vector<std::byte> bytes;
    assert(zip.write_to_memory(bytes));

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclx_buffer(bytes, doc));

    assert(doc.get_properties().size()==1);
    assert(doc.get_instances().size()==1);
    assert(doc.get_relations().size()==1);

    assert(doc.get_properties().at(0).get_type()=="language");
    assert(doc.get_properties().at(0).get_subj_path()=="/doclang[1]/text[1]");
    assert(doc.get_instances().at(0).get_type()=="term");
    assert(doc.get_instances().at(0).get_name()=="Archive");
    assert(doc.get_relations().at(0).get_name()=="contains");

    return 0;
  }

  int test_write_dclx_annotations()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<text>Writer text</text>"
      "</doclang>";

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclg_buffer(xml, doc));

    doc.mutable_properties().emplace_back(123,
                                          andromeda::TEXT,
                                          "/doclang[1]/text[1]",
                                          andromeda::LANGUAGE,
                                          "en",
                                          0.99);

    andromeda::base_instance inst_i(123,
                                    andromeda::TEXT,
                                    "/doclang[1]/text[1]",
                                    andromeda::TERM,
                                    "term",
                                    "Writer",
                                    "Writer",
                                    {0, 6},
                                    {0, 1},
                                    {0, 1});

    andromeda::base_instance inst_j(123,
                                    andromeda::TEXT,
                                    "/doclang[1]/text[1]",
                                    andromeda::TERM,
                                    "term",
                                    "text",
                                    "text",
                                    {7, 11},
                                    {1, 2},
                                    {1, 2});

    doc.mutable_instances().push_back(inst_i);
    doc.mutable_instances().push_back(inst_j);
    doc.mutable_relations().emplace_back("contains", 0.75, inst_i, inst_j);

    std::vector<std::byte> bytes;
    assert(andromeda::doclang::writer::write_dclx_buffer(doc, bytes));

    andromeda::doclang::dclx_document restored;
    assert(andromeda::doclang::reader::read_dclx_buffer(bytes, restored));
    assert(restored.artifacts().has(andromeda::doclang::PROPERTIES_CSV));
    assert(restored.artifacts().has(andromeda::doclang::INSTANCES_CSV));
    assert(restored.artifacts().has(andromeda::doclang::RELATIONS_CSV));

    assert(restored.get_properties().size()==1);
    assert(restored.get_instances().size()==2);
    assert(restored.get_relations().size()==1);
    assert(restored.get_relations().at(0).get_name()=="contains");

    return 0;
  }

  int test_reject_invalid_xml()
  {
    andromeda::doclang::dclx_document doc;
    const std::string xml = "<doclang><text>missing close";

    assert(not andromeda::doclang::reader::read_dclg_buffer(xml, doc));
    assert(doc.get_last_error().find("could not parse DocLang XML")!=std::string::npos);

    return 0;
  }

  int test_preserve_mixed_content_order()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<text>before<![CDATA[cdata-value]]>after</text>"
      "<table>"
      "<fcel/>"
      "cell-a"
      "<lcel/>"
      "<nl/>"
      "<ched/>"
      "head"
      "</table>"
      "<picture>"
      "<caption>Figure caption</caption>"
      "<src uri=\"assets/image.png\"/>"
      "<list><ldiv><marker>a.</marker></ldiv>List body</list>"
      "<text>Picture text</text>"
      "</picture>"
      "</doclang>";

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclg_buffer(xml, doc));

    auto text_nodes = andromeda::doclang::child_content(doc.root().child("text"));
    assert(text_nodes.size()==3);
    assert(text_nodes.at(0).kind==andromeda::doclang::content_kind::text);
    assert(text_nodes.at(0).value=="before");
    assert(text_nodes.at(1).kind==andromeda::doclang::content_kind::cdata);
    assert(text_nodes.at(1).value=="cdata-value");
    assert(text_nodes.at(2).kind==andromeda::doclang::content_kind::text);
    assert(text_nodes.at(2).value=="after");

    auto table_nodes = andromeda::doclang::child_content(doc.root().child("table"));
    assert(table_nodes.size()==6);
    assert(table_nodes.at(0).kind==andromeda::doclang::content_kind::element);
    assert(table_nodes.at(0).name=="fcel");
    assert(table_nodes.at(1).kind==andromeda::doclang::content_kind::text);
    assert(table_nodes.at(1).value=="cell-a");
    assert(table_nodes.at(2).name=="lcel");
    assert(table_nodes.at(3).name=="nl");
    assert(table_nodes.at(4).name=="ched");
    assert(table_nodes.at(5).kind==andromeda::doclang::content_kind::text);
    assert(table_nodes.at(5).value=="head");

    std::vector<std::string> table_text;
    doc.iterate_table_text([&](pugi::xml_node table, pugi::xml_node text_node)
    {
      assert(std::string_view(table.name())=="table");
      table_text.push_back(text_node.value());
    });

    assert(table_text.size()==2);
    assert(table_text.at(0)=="cell-a");
    assert(table_text.at(1)=="head");

    std::vector<std::string> picture_text;
    doc.iterate_picture_text([&](pugi::xml_node picture, pugi::xml_node text_node)
    {
      assert(std::string_view(picture.name())=="picture");
      picture_text.push_back(andromeda::doclang::direct_text_content(text_node));
    });

    assert(picture_text.size()==3);
    assert(picture_text.at(0)=="Figure caption");
    assert(picture_text.at(1)=="List body");
    assert(picture_text.at(2)=="Picture text");

    const std::string serialized = andromeda::doclang::serialize_xml(doc.xml());
    assert(serialized.find("<![CDATA[cdata-value]]>")!=std::string::npos);
    assert(serialized.find("<fcel />")!=std::string::npos or
           serialized.find("<fcel/>")!=std::string::npos);

    return 0;
  }

  int test_doclang_at()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<text>Body text</text>"
      "<table><fcel/>cell-a<lcel/><nl/><ched/>head</table>"
      "<picture>"
      "<caption>Figure caption</caption>"
      "<src uri=\"assets/image.png\"/>"
      "<list><ldiv><marker>a.</marker></ldiv>List body</list>"
      "<text>Picture text</text>"
      "</picture>"
      "</doclang>";

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclg_buffer(xml, doc));

    assert(doc.at("/doclang[1]/text[1]")=="Body text");
    assert(doc.at("/doclang[1]/text[1]", "text")=="Body text");
    assert(doc.at("/doclang[1]/text[1]", "doclang").find("<text>Body text</text>")!=std::string::npos);
    assert(doc.at("/doclang[1]/table[1]/text()[1]")=="cell-a");
    assert(doc.at("/doclang[1]/table[1]", "text")=="cell-a\nhead");
    assert(doc.at("/doclang[1]/picture[1]", "text")=="Figure caption\nList body\nPicture text");
    assert(doc.at("/doclang[1]/missing[1]").empty());
    assert(doc.get_last_error().find("not found")!=std::string::npos);

    return 0;
  }

  int test_doclang_compute_entities()
  {
    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclg_buffer("<doclang><text>Body</text></doclang>", doc));

    auto& instances = doc.mutable_instances();
    using range_type = andromeda::base_types::range_type;

    for(std::size_t i=0; i<4; i++)
      {
        instances.emplace_back(123, andromeda::TEXT, "/doclang[1]/text[1]",
                               andromeda::TERM, "", "very tall man", "very tall man",
                               range_type{i, i+1}, range_type{i, i+1}, range_type{i, i+1});
      }

    for(std::size_t i=0; i<3; i++)
      {
        instances.emplace_back(123, andromeda::TEXT, "/doclang[1]/text[1]",
                               andromeda::TERM, "", "tall man", "tall man",
                               range_type{i, i+1}, range_type{i, i+1}, range_type{i, i+1});
      }

    for(std::size_t i=0; i<2; i++)
      {
        instances.emplace_back(123, andromeda::TEXT, "/doclang[1]/text[1]",
                               andromeda::TERM, "", "small man", "small man",
                               range_type{i, i+1}, range_type{i, i+1}, range_type{i, i+1});
      }

    doc.compute_entities();
    const auto& entities = doc.get_entities();
    assert(entities.size()==4);
    assert(andromeda::doclang::dclg_document::hash("tall man")==instances.at(4).get_ehash());

    bool found_man = false;
    bool found_tall_man = false;
    for(const auto& entity:entities)
      {
        if(entity.get_name()=="man")
          {
            found_man = true;
            assert(entity.get_entity_kind()=="derived");
            assert(entity.get_count()==9);
            assert(entity.get_parent().empty());
          }

        if(entity.get_name()=="tall man")
          {
            found_tall_man = true;
            assert(entity.get_hash()==andromeda::doclang::dclg_document::hash("tall man"));
            assert(entity.get_entity_kind()=="exact");
            assert(entity.get_count()==7);
            assert(entity.get_parent()=="man");
          }
      }

    assert(found_man);
    assert(found_tall_man);

    return 0;
  }

  int test_dclg_sidecars()
  {
    const std::string summary_dclg =
      "<doclang version=\"0.7\"><text>Summary</text></doclang>";
    const std::string toc_dclg =
      "<doclang version=\"0.7\"><toc>"
      "<entry xpath=\"/doclang[1]/heading[1]\"><description>Introduction</description></entry>"
      "</toc></doclang>";
    const std::string concepts_dclg =
      "<doclang version=\"0.7\"><concepts>"
      "<concept><header>FeSe</header><abbreviation>FeSe</abbreviation>"
      "<description>Material</description></concept>"
      "</concepts></doclang>";

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclg_buffer(
             "<doclang version=\"0.7\"><text>Body</text></doclang>", doc));

    // absent
    assert(not doc.has_summary());
    assert(not doc.has_toc());
    assert(not doc.has_concepts());
    assert(not doc.get_summary().has_value());
    assert(not doc.has_annotations());

    // valid: each sidecar is stored parsed, and keeps its own raw DCLG string
    assert(andromeda::doclang::set_summary_dclg(doc, summary_dclg));
    assert(andromeda::doclang::set_toc_dclg(doc, toc_dclg));
    assert(andromeda::doclang::set_concepts_dclg(doc, concepts_dclg));

    assert(doc.has_summary() and doc.has_toc() and doc.has_concepts());
    assert(doc.has_annotations());
    assert(doc.get_summary().value()->valid());
    assert(doc.get_summary().value()->raw()==summary_dclg);
    assert(doc.get_toc().value()->raw()==toc_dclg);
    assert(doc.get_concepts().value()->raw()==concepts_dclg);

    // the sidecar is a full DCLG document, not an opaque string
    assert(doc.get_summary().value()->at("/doclang[1]/text[1]")=="Summary");
    assert(std::string(doc.get_toc().value()->root().child("toc").child("entry")
                       .attribute("xpath").value())=="/doclang[1]/heading[1]");

    // invalid: malformed XML is rejected and reported against the sidecar path
    assert(not andromeda::doclang::set_summary_dclg(doc, "<doclang><text>oops"));
    assert(doc.get_last_error().find(andromeda::doclang::SUMMARY_DCLG)!=std::string::npos);
    assert(doc.get_last_error().find("could not parse DocLang XML")!=std::string::npos);

    assert(not andromeda::doclang::set_toc_dclg(
             doc, "<doclang><toc><entry /></toc></doclang>"));
    assert(doc.get_last_error().find("entries require xpath")!=std::string::npos);

    assert(not andromeda::doclang::set_concepts_dclg(
             doc, "<doclang><concepts><concept /></concepts></doclang>"));
    assert(doc.get_last_error().find("require one <header>")!=std::string::npos);

    // a rejected sidecar leaves the previously accepted one untouched
    assert(doc.get_summary().value()->raw()==summary_dclg);
    assert(doc.get_toc().value()->raw()==toc_dclg);

    // replacement
    const std::string replaced_dclg =
      "<doclang version=\"0.7\"><text>Replaced</text></doclang>";
    assert(andromeda::doclang::set_summary_dclg(doc, replaced_dclg));
    assert(doc.get_summary().value()->raw()==replaced_dclg);
    assert(doc.get_summary().value()->at("/doclang[1]/text[1]")=="Replaced");

    // DCLX round trip
    std::vector<std::byte> bytes;
    assert(andromeda::doclang::writer::write_dclx_buffer(doc, bytes));

    andromeda::doclang::dclx_document restored;
    assert(andromeda::doclang::reader::read_dclx_buffer(bytes, restored));
    assert(restored.has_summary() and restored.has_toc() and restored.has_concepts());
    assert(restored.get_summary().value()->valid());
    assert(restored.get_summary().value()->raw()==replaced_dclg);
    assert(restored.get_toc().value()->raw()==toc_dclg);
    assert(restored.get_concepts().value()->raw()==concepts_dclg);
    assert(restored.get_concepts().value()->root().child("concepts")
           .child("concept").child("header").child_value()==std::string("FeSe"));

    // clearing
    restored.clear_summary();
    restored.clear_toc();
    restored.clear_concepts();
    assert(not restored.has_summary());
    assert(not restored.has_toc());
    assert(not restored.has_concepts());
    assert(not restored.get_summary().has_value());

    // a cleared sidecar is dropped from the archive on the next write
    std::vector<std::byte> cleared_bytes;
    assert(andromeda::doclang::writer::write_dclx_buffer(restored, cleared_bytes));

    andromeda::doclang::dclx_document reread;
    assert(andromeda::doclang::reader::read_dclx_buffer(cleared_bytes, reread));
    assert(not reread.has_summary());
    assert(not reread.has_toc());
    assert(not reread.has_concepts());

    return 0;
  }

  int test_dclx_without_sidecars_stays_valid()
  {
    const std::string xml =
      "<doclang version=\"0.7\"><text>Archive text</text></doclang>";

    std::vector<std::byte> bytes;
    assert(create_dclx(bytes, xml));

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclx_buffer(bytes, doc));
    assert(doc.valid());
    assert(not doc.has_summary());
    assert(not doc.has_toc());
    assert(not doc.has_concepts());
    assert(doc.raw()==xml);

    return 0;
  }

  int test_read_only_document_view()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<page_header><location value=\"1\"/>Header</page_header>"
      "<heading level=\"2\">"
      "<location value=\"10\"/>"
      "<location value=\"20\"/>"
      "<location value=\"30\"/>"
      "<location value=\"40\"/>"
      "Section"
      "</heading>"
      "<text>Body</text>"
      "<table><fcel/>cell<nl/></table>"
      "</doclang>";

    auto doc = std::make_shared<andromeda::doclang::dclx_document>();
    assert(andromeda::doclang::reader::read_dclg_buffer(xml, *doc));

    unsigned iterated = 0;
    doc->iterate_elements([&](pugi::xml_node)
    {
      iterated += 1;
    });
    assert(iterated==4);

    unsigned headings_by_name = 0;
    doc->iterate_elements("heading", [&](pugi::xml_node node)
    {
      headings_by_name += 1;
      assert(std::string_view(node.name())=="heading");
    });
    assert(headings_by_name==1);

    andromeda::doclang::document_view view(doc);
    assert(view.valid());

    auto body = view.body_elements();
    assert(body.size()==4);
    assert(body.at(0).name()=="page_header");
    assert(body.at(1).name()=="heading");
    assert(body.at(2).name()=="text");
    assert(body.at(3).name()=="table");

    auto headings = view.elements_by_name("heading");
    assert(headings.size()==1);
    assert(headings.at(0).heading_level().has_value());
    assert(headings.at(0).heading_level().value()==2);
    assert(headings.at(0).text_content()=="Section");

    auto loc = headings.at(0).location();
    assert(loc.size()==4);
    assert(loc.at(0)==10.0F);
    assert(loc.at(1)==20.0F);
    assert(loc.at(2)==30.0F);
    assert(loc.at(3)==40.0F);

    auto text_like = view.text_like_elements();
    assert(text_like.size()==3);
    assert(text_like.at(0).name()=="page_header");
    assert(text_like.at(1).name()=="heading");
    assert(text_like.at(2).name()=="text");

    auto tables = view.table_elements();
    assert(tables.size()==1);
    assert(tables.at(0).name()=="table");

    return 0;
  }

  int test_subject_adapter_preserves_doclang_metadata()
  {
    const std::string xml =
      "<doclang version=\"0.7\">"
      "<heading level=\"2\">Section</heading>"
      "<text>Body</text>"
      "<table><fcel/>Cell<nl/></table>"
      "</doclang>";

    andromeda::doclang::dclx_document doc;
    assert(andromeda::doclang::reader::read_dclg_buffer(xml, doc));

    andromeda::subject<andromeda::DOCUMENT> subject;
    andromeda::doclang::adapter_options options;
    options.document_name = "adapter-test";

    assert(andromeda::doclang::subject_adapter::to_subject_document(doc, subject, options));
    assert(subject.get_name()=="adapter-test");
    assert(subject.texts.size()==3);
    assert(subject.tables.empty());

    assert(subject.texts.at(0)->get_text()=="Section");
    assert(subject.texts.at(0)->payload["doclang_name"]=="heading");
    assert(subject.texts.at(0)->payload["doclang_heading_level"]==2);

    assert(subject.texts.at(1)->get_text()=="Body");
    assert(subject.texts.at(1)->payload["doclang_name"]=="text");

    assert(subject.texts.at(2)->get_text()=="Cell");
    assert(subject.texts.at(2)->payload["doclang_name"]=="table");
    assert(subject.texts.at(2)->payload["doclang_path"]=="#/doclang/2");

    andromeda::subject<andromeda::DOCUMENT> no_table_subject;
    options.include_tables_as_text = false;
    assert(andromeda::doclang::subject_adapter::to_subject_document(doc, no_table_subject, options));
    assert(no_table_subject.texts.size()==2);

    return 0;
  }

}

int main()
{
  test_read_dclg();
  test_read_dclx_buffer();
  test_read_dclx_annotations();
  test_write_dclx_annotations();
  test_reject_invalid_xml();
  test_preserve_mixed_content_order();
  test_doclang_at();
  test_doclang_compute_entities();
  test_dclg_sidecars();
  test_dclx_without_sidecars_stays_valid();
  test_read_only_document_view();
  test_subject_adapter_preserves_doclang_metadata();

  std::cout << "test_unit_doclang_reader.exe passed\n";
  return 0;
}
