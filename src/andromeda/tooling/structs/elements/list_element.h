//-*-C++-*-

/*
#ifndef ANDROMEDA_SUBJECTS_LIST_ELEMENT_H_
#define ANDROMEDA_SUBJECTS_LIST_ELEMENT_H_

namespace andromeda
{
  class list_element: public text_element
  {
  public:

    list_element(std::size_t i, 
		 std::string orig);

  public:
    
    std::size_t i;
    bool numeric;
  };

  list_element::list_element(std::size_t i, 
			     std::string orig):
    i(i), 
    numeric(false)
  {
    text_element::set(orig, NULL, NULL);
  }
  
}

#endif
*/
