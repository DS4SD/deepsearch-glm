//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_ELEMENTS_TABLE_ELEMENT_H_
#define ANDROMEDA_SUBJECTS_ELEMENTS_TABLE_ELEMENT_H_

namespace andromeda
{
  class table_element: public text_element
  {
  public:

    table_element(std::size_t i, std::size_t j,
		  std::string orig);

    table_element(std::size_t i, std::size_t j,
		  std::size_t col_span,
		  std::size_t row_span,
		  std::string orig);

  public:
    
    std::size_t i, j;
    std::size_t col_span, row_span;

    bool col_header, row_header, numeric;
  };

  table_element::table_element(std::size_t i, std::size_t j,
			       std::string orig):
    //text_element(orig, NULL, NULL),
    text_element(),
    i(i), j(j),
    col_span(1),
    row_span(1),
    col_header(false),
    row_header(false),
    numeric(false)
  {
    text_element::set(orig, NULL, NULL);
  }

  table_element::table_element(std::size_t i, std::size_t j,
			       std::size_t col_span,
			       std::size_t row_span,
			       std::string orig):
    //text_element(orig, NULL, NULL),
    text_element(),
    i(i), j(j),
    col_span(col_span),
    row_span(row_span),
    col_header(false),
    row_header(false),
    numeric(false)
  {
    text_element::set(orig, NULL, NULL);
  }    

}

#endif
