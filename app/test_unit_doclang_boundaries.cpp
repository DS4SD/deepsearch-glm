//-*-C++-*-

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "libraries.h"
#include "andromeda/tooling/doclang.h"

int main()
{
  const std::string xml =
    "<doclang version=\"0.7\">"
    "<heading level=\"1\">Title</heading>"
    "<text>Body</text>"
    "</doclang>";

  andromeda::doclang::document doc;
  assert(andromeda::doclang::reader::read_dclg_buffer(xml, doc));

  unsigned count = 0;
  doc.iterate_elements([&](pugi::xml_node)
  {
    count += 1;
  });

  assert(count==2);

  andromeda::doclang::document_view view(
    std::make_shared<andromeda::doclang::document>(std::move(doc)));

  assert(view.valid());
  assert(view.text_like_elements().size()==2);

  std::cout << "test_unit_doclang_boundaries.exe passed\n";
  return 0;
}
