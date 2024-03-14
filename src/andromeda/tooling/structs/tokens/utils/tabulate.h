//-*-C++-*-

#ifndef ANDROMEDA_STRUCTS_TOKENS_TABULATE_H_
#define ANDROMEDA_STRUCTS_TOKENS_TABULATE_H_

namespace andromeda
{
  std::string tabulate(std::vector<char_token>& tokens)
  {
    std::vector<std::string> header = char_token::HEADERS;
    std::vector<std::vector<std::string>> data={};
    
    std::size_t cnt=0;
    for(auto& token:tokens)
      {
	std::vector<std::string> row = { std::to_string(token.get_rng(0)),
					 std::to_string(token.get_rng(1)),
					 std::to_string(token.len()),
					 std::to_string(cnt++),
					 std::to_string(token.get_ind()), 
					 token.get_orig(), token.get_norm()};
	data.push_back(row);
      }
    
    return utils::to_string(header, data);
  }

  std::string tabulate(std::vector<word_token>& tokens)
  {
    std::vector<std::string> headers = word_token::HEADERS;
    std::vector<std::vector<std::string>> data={};

    std::size_t cnt=0;
    for(auto& token:tokens)
      {
	std::string word = token.get_word();
	std::string orig = "-";
	
	auto inds = token.get_inds();
	auto subws = token.get_subws();
	
	word = utils::to_fixed_size(word, 48);
		
	std::vector<std::string> row = { std::to_string(token.get_hash()),
					 std::to_string(token.get_rng(0)),
					 std::to_string(token.get_rng(1)),
					 std::to_string(cnt++),
					 token.get_pos(),
					 utils::to_string(token.get_tags()),
					 (token.is_known()? "true":"false"),
					 word, orig,
					 utils::to_string(inds),
					 utils::to_string(subws)};
	
	assert(row.size()==headers.size());
	
	data.push_back(row);
      }
    
    return utils::to_string(headers, data);
  }
  
  std::string tabulate(std::vector<word_token>& tokens, std::string text)
  {
    std::vector<std::string> headers = word_token::HEADERS;
    std::vector<std::vector<std::string>> data={};

    std::size_t cnt=0;
    for(auto& token:tokens)
      {
	std::string word = token.get_word();
	std::string orig = token.get_orig(text);
	
	auto inds = token.get_inds();
	auto subws = token.get_subws();
	
	word = utils::to_fixed_size(word, 48);
	orig = utils::to_fixed_size(orig, 48);
	
	std::vector<std::string> row = { std::to_string(token.get_hash()),
					 std::to_string(token.get_rng(0)),
					 std::to_string(token.get_rng(1)),
					 std::to_string(cnt++),
					 token.get_pos(),
					 utils::to_string(token.get_tags()),
					 (token.is_known()? "true":"false"),
					 word, orig,
					 utils::to_string(inds),
					 utils::to_string(subws)};
	
	assert(row.size()==headers.size());
	
	data.push_back(row);
      }
    
    return utils::to_string(headers, data);
  }

  nlohmann::json to_json(std::vector<word_token>& tokens,
			 std::string& text)
  {
    nlohmann::json result = nlohmann::json::object({});

    auto& headers = result["headers"];
    headers = word_token::HEADERS;

    auto& data = result["data"];
    data = nlohmann::json::array({});
    
    std::size_t ind=0;
    for(auto& token:tokens)
      {
	std::string word = token.get_word();
	std::string orig = token.get_orig(text);

	auto inds = token.get_inds();
	auto subws = token.get_subws();
	
	nlohmann::json row = nlohmann::json::array({});
	{
	  row.push_back(token.get_hash());
	  
	  row.push_back(token.get_rng(0));
	  row.push_back(token.get_rng(1));

	  row.push_back(ind++);

	  row.push_back(token.get_pos());
	  row.push_back(utils::to_string(token.get_tags()));

	  row.push_back((token.is_known()? true:false));
	  
	  row.push_back(word);
	  row.push_back(orig);

	  row.push_back(inds);
	  row.push_back(subws);
	}

	assert(row.size()==headers.size());
	data.push_back(row);
      }

    return result;
  }  
  
  bool from_json(std::vector<word_token>& tokens,
		 const nlohmann::json& data)
  {
    typedef typename word_token::hash_type hash_type;
    typedef typename word_token::index_type index_type;
    
    std::vector<std::string> headers={};    
    headers = data.value("headers", headers);

    std::size_t hash_ind = utils::index_of("hash", headers);
    
    std::size_t char_i_ind = utils::index_of("char_i", headers);
    std::size_t char_j_ind = utils::index_of("char_j", headers);

    std::size_t pos_ind = utils::index_of("pos", headers);
    std::size_t tag_ind = utils::index_of("tag", headers);
    std::size_t knwn_ind = utils::index_of("known", headers);

    std::size_t text_ind = utils::index_of("word", headers);
    std::size_t orig_ind = utils::index_of("original", headers);

    std::size_t inds_ind = utils::index_of("inds", headers);
    std::size_t subws_ind = utils::index_of("subws", headers);
    
    const std::size_t mONE=-1;
    
    if(char_i_ind==mONE or char_j_ind==mONE or
       pos_ind==mONE or tag_ind==mONE or
       text_ind==mONE or orig_ind==mONE or
       inds_ind==mONE or subws_ind==mONE)
      {
	LOG_S(ERROR) << "can not find the correct column index for word-token";
	return false;
      }
    
    for(auto& row:data["data"])
      {
	hash_type hash;
	index_type char_i, char_j;
	bool known;
	std::string text, orig, pos, tag;

	std::vector<int> inds={};
	std::vector<std::string> subws={};
	
	hash = row[hash_ind].get<hash_type>();
	
	char_i = row[char_i_ind].get<index_type>();
	char_j = row[char_j_ind].get<index_type>();

	pos = row[pos_ind].get<std::string>();
	tag = row[tag_ind].get<std::string>();
	known = row[knwn_ind].get<bool>();

	text = row[text_ind].get<std::string>();
	//orig = row[orig_ind].get<std::string>();
       	
	std::set<std::string> tags={};
	utils::from_string(tag, tags);

	inds = row[inds_ind].get<std::vector<int> >();
	subws = row[subws_ind].get<std::vector<std::string> >();
	
	word_token wt(hash, char_i, char_j, pos, tags, known, text, inds, subws);
	tokens.push_back(wt);
      }

    return true;
  }
  
}

#endif

