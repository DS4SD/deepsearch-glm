//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_POS_PATTERN_H_
#define ANDROMEDA_MODELS_ENTITIES_POS_PATTERN_H_

namespace andromeda
{
  class base_pos_pattern
  {
    typedef typename pcre2_group::index_type index_type;
    typedef typename pcre2_group::range_type range_type;
    
  protected:

    base_pos_pattern();
    ~base_pos_pattern();

    void get_ranges(subject<TEXT>& subj,
                    std::vector<range_type >& ranges_01,
                    std::vector<range_type >& ranges_02);

    void get_chunks(text_element& elem,
		    std::vector<pcre2_expr>& exprs,
		    std::vector<pcre2_item>& chunks);
    
    std::vector<std::size_t> get_indices(std::string& text);

    bool overlaps(range_type& range,
                  std::vector<range_type >& ranges);

    bool contains(range_type& range,
                  std::vector<range_type >& ranges);

    void add_instances(model_name name, subject<TEXT>& subj,
		       std::vector<range_type >& ranges_01,
		       std::vector<range_type >& ranges_02,
		       std::vector<pcre2_item>& chunks);

    void add_instances(model_name name, subject<TABLE>& subj,
		       range_type coor,
		       range_type row_span,
		       range_type col_span,
		       std::vector<range_type >& ranges_01,
		       std::vector<range_type >& ranges_02,
		       std::vector<pcre2_item>& chunks);    
    
  protected:

    const static inline std::set<model_name> text_dependencies = {SENTENCE, LAPOS};
    const static inline std::set<model_name> table_dependencies = {LAPOS};
    
    const static inline std::set<model_name> dependencies = text_dependencies;
    
    pcre2_expr indices_expr;
  };

  base_pos_pattern::base_pos_pattern():
    indices_expr("help-regex", "indices", R"(\d+)")
  {}

  base_pos_pattern::~base_pos_pattern()
  {}

  void base_pos_pattern::get_ranges(subject<TEXT>& subj,
                                    std::vector<range_type>& ranges_01,
                                    std::vector<range_type>& ranges_02)
  {
    for(auto& ent_i:subj.instances)
      {
        if((ent_i.is_model(PARENTHESIS) and ent_i.is_subtype("reference")) or
           (ent_i.is_model(LINK)))
          {
            ranges_01.push_back(ent_i.get_char_range());
          }
        else if(ent_i.is_model(NAME) or
                ent_i.is_model(NUMVAL))
          {
            ranges_02.push_back(ent_i.get_char_range());
          }
      }
  }

  void base_pos_pattern::get_chunks(text_element& subj,
				    std::vector<pcre2_expr>& exprs,
                                    std::vector<pcre2_item>& chunks)
  {
    chunks.clear();

    auto& word_tokens = subj.get_word_tokens();
    
    std::stringstream ss;
    for(std::size_t l=0; l<word_tokens.size(); l++)
      {
        ss << word_tokens.at(l).get_pos() << R"({)" << l << R"(})";
      }

    std::string encoding = ss.str();
    for(auto& expr:exprs)
      {
        expr.find_all(encoding, chunks);

        for(auto& chunk:chunks)
          {
            utils::mask(encoding, chunk.rng);
          }
      }
  }  

  std::vector<std::size_t> base_pos_pattern::get_indices(std::string& text)
  {
    std::vector<std::size_t> token_inds={};

    std::vector<pcre2_item> inds;
    indices_expr.find_all(text, inds);

    for(pcre2_item& ind:inds)
      {
        try
          {
            int token_ind = std::stoi(ind.text);
            token_inds.push_back(token_ind);
          }
        catch(std::exception& e)
          {}
      }

    return token_inds;
  }

  bool base_pos_pattern::overlaps(range_type& range,
                                  std::vector<range_type >& ranges)
  {
    for(auto& rng:ranges)
      {
        if((rng[0]<=range[0] and range[0]<rng[1]) or
           (rng[0]<=range[1] and range[1]<rng[1]))
          {
            return true;
          }
      }

    return false;
  }

  bool base_pos_pattern::contains(range_type& range,
                                  std::vector<range_type >& ranges)
  {
    for(auto& rng:ranges)
      {
        if((rng[0]<=range[0] and range[0]<=rng[1]) and
           (rng[0]<=range[1] and range[1]<=rng[1]))
          {
            return true;
          }
      }

    return false;
  }

  void base_pos_pattern::add_instances(model_name name, subject<TEXT>& subj,
				      std::vector<range_type >& ranges_01,
				      std::vector<range_type >& ranges_02,
				      std::vector<pcre2_item>& chunks)
  {
    auto& word_tokens = subj.get_word_tokens();
    
    for(pcre2_item& chunk:chunks)
      {
	std::vector<std::size_t> token_inds = get_indices(chunk.text);	
	
	std::string type=chunk.type;
	std::string subtype=chunk.subtype;
	std::size_t ci=0, cj=0;

	std::vector<std::pair<std::string, std::string> > words;
	for(std::size_t l=0; l<token_inds.size(); l++)
	  {
	    std::size_t ind = token_inds.at(l);
	    auto& token = word_tokens.at(ind);

	    if(l==0)
	      {
		ci = token.get_rng(0);
		cj = token.get_rng(1);
	      }
	    else
	      {
		cj = token.get_rng(1);
	      }

	    words.emplace_back(token.get_word(), token.get_pos());
	  }

	range_type char_range={ci,cj};

	range_type ctok_range = subj.get_char_token_range(char_range);
	range_type wtok_range = subj.get_word_token_range(char_range);

	std::string orig = subj.from_char_range(char_range);
	std::string text = subj.from_wtok_range(wtok_range);
	
	if(not overlaps(char_range, ranges_01) and
	   not contains(char_range, ranges_02) and
	   (char_range[1]-char_range[0])>1)
	  {
	    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
				       name, subtype,
				       text, orig,
				       char_range, ctok_range, wtok_range);
	  }
      }
  }

  void base_pos_pattern::add_instances(model_name name, subject<TABLE>& subj,
				       range_type coor,
				       range_type row_span, range_type col_span,
				       std::vector<range_type >& ranges_01,
				       std::vector<range_type >& ranges_02,
				       std::vector<pcre2_item>& chunks)
  {
    for(pcre2_item& chunk:chunks)
      {
	std::vector<std::size_t> token_inds = get_indices(chunk.text);	
	
	std::string type=chunk.type;
	std::string subtype=chunk.subtype;
	std::size_t ci=0,cj=0;

	auto& elem = subj(coor);
	auto& word_tokens = elem.get_word_tokens();	

	std::vector<std::pair<std::string, std::string> > words;
	for(std::size_t l=0; l<token_inds.size(); l++)
	  {
	    std::size_t ind = token_inds.at(l);
	    auto& token = word_tokens.at(ind);

	    if(l==0)
	      {
		ci = token.get_rng(0);
		cj = token.get_rng(1);
	      }
	    else
	      {
		cj = token.get_rng(1);
	      }

	    words.emplace_back(token.get_word(), token.get_pos());
	  }

	range_type char_range={ci,cj};

	range_type ctok_range = elem.get_char_token_range(char_range);
	range_type wtok_range = elem.get_word_token_range(char_range);

	std::string orig = elem.from_char_range(char_range);
	std::string text = elem.from_wtok_range(wtok_range);
	
	if(not overlaps(char_range, ranges_01) and
	   not contains(char_range, ranges_02) and
	   (char_range[1]-char_range[0])>1)
	  {
	    subj.instances.emplace_back(subj.get_hash(), subj.get_name(), subj.get_self_ref(),
					name, subtype,
					text, orig,
					coor, row_span, col_span,
					char_range, ctok_range, wtok_range);
	  }
      }    
  }

}

#endif

#include <andromeda/nlp/ent/pos_pattern/conn.h>
#include <andromeda/nlp/ent/pos_pattern/term.h>
#include <andromeda/nlp/ent/pos_pattern/verb.h>
