//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_DOCUMENT_DOC_MAINTEXT_H_
#define ANDROMEDA_SUBJECTS_DOCUMENT_DOC_MAINTEXT_H_

namespace andromeda
{
  class doc_maintext: public base_types
  {

  public:

    doc_maintext();
    
    template<typename doc_type>
    void filter_maintext(doc_type& doc);

    template<typename doc_type>
    void concatenate_maintext(doc_type& doc);

  private:
    
  };

  doc_maintext::doc_maintext()
  {}

  template<typename doc_type>
  void doc_maintext::filter_maintext(doc_type& doc)
  {
    std::set<std::string> maintext_types = {"title", "subtitle-level-1", "paragraph", "formula"};

    auto& paragraphs = doc.paragraphs;

    auto itr=paragraphs.begin();
    while(itr!=paragraphs.end())
      {
        std::string type = (*itr)->provs.at(0)->type;

        if(itr->use_count()>1)
          {
            itr = paragraphs.erase(itr);
          }
        else if(maintext_types.count(type)==0)
          {
            if(type=="caption")
              {
                (*itr)->provs.at(0)->type = "paragraph";
                (*itr)->provs.at(0)->name = "paragraph";

                itr++;
              }
            else
              {
                itr = paragraphs.erase(itr);
              }
          }
        else
          {
            itr++;
          }
      }
  }

  template<typename doc_type>
  void doc_maintext::concatenate_maintext(doc_type& doc)
  {
    auto& paragraphs = doc.paragraphs;

    for(ind_type l=0; l+1<paragraphs.size(); l++)
      {
        auto& curr = paragraphs.at(l+0);

        if(not curr->is_valid())
          {
            continue;
          }

        auto& next = paragraphs.at(l+1);

        auto& curr_prov = curr->provs.back();
        auto& next_prov = next->provs.front();

        auto& curr_text = curr->text;
        auto& next_text = next->text;

        if(curr_prov->type!="paragraph" or
           next_prov->type!="paragraph" or
           curr_text.size()==0 or
           next_text.size()==0)
          {
            continue;
          }

        char lc = curr_text.back();
        char fc = next_text.front();

        if(lc=='-' and 'a'<=fc<='z' and
           (curr_prov->is_strictly_left_of(*next_prov) or
            (curr_prov->page)+1==next_prov->page))
          {
            LOG_S(WARNING) << "attempting to concat text: \n`" << curr_text << "`"
                           << "\n with \n`" << next_text << "`";

            curr->concatenate(next);
            next->valid=false;
          }
      }

    {
      auto itr=paragraphs.begin();
      while(itr!=paragraphs.end())
        {
          if((*itr)->valid)
            {
              itr++;
            }
          else
            {
              itr = paragraphs.erase(itr);
            }
        }
    }
  }

}

#endif
