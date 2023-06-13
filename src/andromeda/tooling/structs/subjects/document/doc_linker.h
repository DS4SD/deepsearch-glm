//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_DOC_LINKER_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_DOC_LINKER_H_

namespace andromeda
{
  class doc_linker: public base_types
  {

  public:

    doc_linker();

    template<typename doc_type>
    void find_captions(doc_type& doc);
    
  private:

  };

  doc_linker::doc_linker()
  {}

  template<typename doc_type>
  void doc_linker::find_captions(doc_type& doc)
  {
    auto& provs = doc.provs;
    std::set<ind_type> page_nums={};

    for(auto& prov:provs)
      {
	page_nums.insert(prov->page);
      }

    for(auto page_num:page_nums)
      {
	std::shared_ptr<prov_element> delim;
	delim->page = page_num;
	
	auto lb = std::lower_bound(provs.begin(), provs.end(), delim,
				   [](const std::shared_ptr<prov_element> lhs,
				      const std::shared_ptr<prov_element> rhs)
				   {
				     return lhs->page<rhs->page;
				   });
	
	auto ub = std::upper_bound(provs.begin(), provs.end(), delim,
				   [](const std::shared_ptr<prov_element> lhs,
				      const std::shared_ptr<prov_element> rhs)
				   {
				     return lhs->page<rhs->page;
				   });

	
      }
  }
  
}

#endif
