//-*-C++-*-

#include <cassert>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "libraries.h"
#include "andromeda/tooling/doclang/dclg_document.h"

namespace
{

  const std::string SAMPLE_DCLG =
    "<doclang version=\"0.7\">"
    "<heading level=\"1\">Title</heading>"
    "<text>Body text</text>"
    "<table><fcel/>cell-a<lcel/><nl/><ched/>head</table>"
    "<picture>"
    "<caption>Figure caption</caption>"
    "<src uri=\"assets/image.png\"/>"
    "<list><ldiv><marker>a.</marker></ldiv>List body</list>"
    "<text>Picture text</text>"
    "</picture>"
    "</doclang>";

  int test_read_valid_dclg()
  {
    andromeda::doclang::dclg_document doc;

    assert(not doc.valid());
    assert(doc.raw().empty());

    assert(doc.read(SAMPLE_DCLG));
    assert(doc.valid());
    assert(doc.get_last_error().empty());
    assert(std::string(doc.root().attribute("version").value())=="0.7");
    assert(std::string(doc.root().child("heading").child_value())=="Title");

    return 0;
  }

  int test_raw_string_retention()
  {
    andromeda::doclang::dclg_document doc;
    assert(doc.read(SAMPLE_DCLG));
    assert(doc.raw()==SAMPLE_DCLG);

    // a re-read replaces both the raw string and the parsed DOM
    assert(doc.read("<doclang><text>Second</text></doclang>"));
    assert(doc.raw()=="<doclang><text>Second</text></doclang>");
    assert(std::string(doc.root().child("text").child_value())=="Second");

    doc.clear();
    assert(doc.raw().empty());
    assert(not doc.valid());

    return 0;
  }

  int test_reject_invalid_xml()
  {
    andromeda::doclang::dclg_document doc;
    assert(doc.read(SAMPLE_DCLG));

    assert(not doc.read("<doclang><text>missing close"));
    assert(not doc.valid());
    assert(doc.raw().empty());
    assert(doc.get_last_error().find("could not parse DocLang XML")!=std::string::npos);

    return 0;
  }

  int test_reject_missing_doclang_root()
  {
    andromeda::doclang::dclg_document doc;

    assert(not doc.read("<other><text>Body</text></other>"));
    assert(not doc.valid());
    assert(doc.raw().empty());
    assert(doc.get_last_error().find("does not contain root <doclang>")!=std::string::npos);

    return 0;
  }

  int test_iterate_elements()
  {
    andromeda::doclang::dclg_document doc;
    assert(doc.read(SAMPLE_DCLG));

    std::vector<std::string> names;
    doc.iterate_elements([&](pugi::xml_node node)
    {
      names.push_back(node.name());
    });

    assert(names.size()==4);
    assert(names.at(0)=="heading");
    assert(names.at(1)=="text");
    assert(names.at(2)=="table");
    assert(names.at(3)=="picture");

    unsigned texts = 0;
    doc.iterate_elements("text", [&](pugi::xml_node node)
    {
      texts += 1;
      assert(std::string_view(node.name())=="text");
    });

    // only the direct <doclang> child, not the one nested in <picture>
    assert(texts==1);

    return 0;
  }

  int test_iterate_table_text()
  {
    andromeda::doclang::dclg_document doc;
    assert(doc.read(SAMPLE_DCLG));

    std::vector<std::string> values;
    doc.iterate_table_text([&](pugi::xml_node table, pugi::xml_node text_node)
    {
      assert(std::string_view(table.name())=="table");
      values.push_back(text_node.value());
    });

    assert(values.size()==2);
    assert(values.at(0)=="cell-a");
    assert(values.at(1)=="head");

    return 0;
  }

  int test_iterate_picture_text()
  {
    andromeda::doclang::dclg_document doc;
    assert(doc.read(SAMPLE_DCLG));

    std::vector<std::string> values;
    doc.iterate_picture_text([&](pugi::xml_node picture, pugi::xml_node text_node)
    {
      assert(std::string_view(picture.name())=="picture");
      values.push_back(andromeda::doclang::direct_text_content(text_node));
    });

    assert(values.size()==3);
    assert(values.at(0)=="Figure caption");
    assert(values.at(1)=="List body");
    assert(values.at(2)=="Picture text");

    return 0;
  }

  int test_at_modes()
  {
    andromeda::doclang::dclg_document doc;
    assert(doc.read(SAMPLE_DCLG));

    assert(doc.at("/doclang[1]/text[1]")=="Body text");
    assert(doc.at("/doclang[1]/text[1]", "text")=="Body text");
    assert(doc.at("/doclang[1]/text[1]", "doclang").find("<text>Body text</text>")!=std::string::npos);
    assert(doc.at("/doclang[1]/table[1]/text()[1]")=="cell-a");
    assert(doc.at("/doclang[1]/table[1]", "text")=="cell-a\nhead");
    assert(doc.at("/doclang[1]/picture[1]", "text")=="Figure caption\nList body\nPicture text");

    assert(doc.at("/doclang[1]/missing[1]").empty());
    assert(doc.get_last_error().find("not found")!=std::string::npos);

    assert(doc.at("/doclang[1]/text[1]", "nonsense").empty());
    assert(doc.get_last_error().find("unsupported DocLang access mode")!=std::string::npos);

    return 0;
  }

  int test_hash_is_reproducible()
  {
    assert(andromeda::doclang::dclg_document::hash("tall man")==
           andromeda::doclang::dclg_document::hash("tall man"));
    assert(andromeda::doclang::dclg_document::hash("tall man")!=
           andromeda::doclang::dclg_document::hash("small man"));

    return 0;
  }

}

int main()
{
  test_read_valid_dclg();
  test_raw_string_retention();
  test_reject_invalid_xml();
  test_reject_missing_doclang_root();
  test_iterate_elements();
  test_iterate_table_text();
  test_iterate_picture_text();
  test_at_modes();
  test_hash_is_reproducible();

  std::cout << "test_unit_dclg_document.exe passed\n";
  return 0;
}
