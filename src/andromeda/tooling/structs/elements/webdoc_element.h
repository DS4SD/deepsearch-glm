//-*-C++-*-

/*
#ifndef ANDROMEDA_SUBJECTS_WEBDOC_ELEMENT_H_
#define ANDROMEDA_SUBJECTS_WEBDOC_ELEMENT_H_

namespace andromeda
{
  class webdoc_element
  {
  public:

    webdoc_element(nlohmann::json& elem);

  public:

    std::size_t hash;
    std::string html;
    std::string type;

    std::size_t depth;
    std::size_t parent;
    std::vector<std::size_t> children;

    subject_name subject_type;
    std::size_t subject_index;
  };

  webdoc_element::webdoc_element(nlohmann::json& elem):

    hash(elem["hash"]),
    html(elem["html"]),
    type(elem["type"]),

    depth(elem["depth"]),
    parent(elem["parent"]),
    children(elem["children"]),

    subject_type(UNDEF),
    subject_index(-1)
  {}
  
}

#endif
*/
