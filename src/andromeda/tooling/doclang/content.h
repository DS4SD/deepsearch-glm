//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_CONTENT_H_
#define ANDROMEDA_TOOLING_DOCLANG_CONTENT_H_

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <pugixml.hpp>

namespace andromeda::doclang
{

  enum class content_kind
  {
    element,
    text,
    cdata,
    other
  };

  struct content_node
  {
    content_kind kind;
    std::string_view name;
    std::string_view value;
    pugi::xml_node node;
  };

  bool is_xml_whitespace(const char* value)
  {
    if(value==nullptr)
      {
        return true;
      }

    for(const char* ptr=value; *ptr!='\0'; ptr++)
      {
        const char c = *ptr;
        if(c!=' ' and c!='\n' and c!='\r' and c!='\t')
          {
            return false;
          }
      }

    return true;
  }

  content_kind to_content_kind(pugi::xml_node_type type)
  {
    switch(type)
      {
      case pugi::node_element:
        return content_kind::element;

      case pugi::node_pcdata:
        return content_kind::text;

      case pugi::node_cdata:
        return content_kind::cdata;

      default:
        return content_kind::other;
      }
  }

  std::vector<content_node> child_content(pugi::xml_node parent,
                                          bool include_whitespace_text=false)
  {
    std::vector<content_node> result;

    for(pugi::xml_node child:parent.children())
      {
        const content_kind kind = to_content_kind(child.type());

        if((kind==content_kind::text or kind==content_kind::cdata) and
           not include_whitespace_text and is_xml_whitespace(child.value()))
          {
            continue;
          }

        result.push_back({kind, child.name(), child.value(), child});
      }

    return result;
  }

  std::string serialize_xml(const pugi::xml_document& doc)
  {
    std::ostringstream oss;
    doc.save(oss);
    return oss.str();
  }

  std::string direct_text_content(pugi::xml_node parent)
  {
    std::ostringstream oss;

    for(pugi::xml_node child:parent.children())
      {
        if(child.type()==pugi::node_pcdata or child.type()==pugi::node_cdata)
          {
            oss << child.value();
          }
      }

    return oss.str();
  }

}

#endif
