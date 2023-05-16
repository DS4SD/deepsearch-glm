//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_ELEMENTS_TABLE_ELEMENT_H_
#define ANDROMEDA_SUBJECTS_ELEMENTS_TABLE_ELEMENT_H_

namespace andromeda
{
  class table_element: public text_element
  {
  public:

    table_element(uint64_t i,
		  uint64_t j,
		  std::string orig);

    table_element(uint64_t i,
		  uint64_t j,
		  uint64_t col_span,
		  uint64_t row_span,
		  std::string orig);

    std::array<uint64_t, 2> get_coor() { return {i,j}; }
    std::array<uint64_t, 2> get_span() { return {col_span, row_span}; }

    std::string get_text() const { return text; }

    bool is_col_header() { return col_header; }
    bool is_row_header() { return row_header; }
    bool is_numeric() { return numeric; }

    void show();
    
  private:
    
    uint64_t i, j;
    uint64_t col_span, row_span;

    bool col_header, row_header, numeric;
  };

  table_element::table_element(uint64_t i,
			       uint64_t j,
			       std::string orig):
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

  table_element::table_element(uint64_t i, uint64_t j,
			       uint64_t col_span,
			       uint64_t row_span,
			       std::string orig):
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

  void table_element::show()
  {
    LOG_S(INFO) << "table(" << i << ", " << j << "): " << text;
    LOG_S(INFO) << tabulate(this->word_tokens, text);
  }
  
}

#endif
