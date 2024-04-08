//-*-C++-*-

#ifndef ANDROMEDA_GLM_MODEL_NODES_BASE_NODE_H
#define ANDROMEDA_GLM_MODEL_NODES_BASE_NODE_H

namespace andromeda
{

  namespace glm
  {

    class base_node: public base_types
    {
    public:

      const static inline std::string hash_lbl = "hash";
      const static inline std::string flvr_lbl = "flvr";
      const static inline std::string name_lbl = "name";

      const static inline std::string text_lbl = "text";
      
      const static inline std::string nodes_lbl = "nodes";
      const static inline std::string edges_lbl = "edges";

      const static inline std::string nodes_text_lbl = "nodes-text";
      
      const static inline std::string tokens_path_lbl = "tokens";
      const static inline std::string tokens_text_lbl = "tokens-text";

      const static inline std::string cnt_lbl = "counters";

      const static inline std::string word_cnt_lbl = "total-count";
      const static inline std::string sent_cnt_lbl = "sentence-count";
      const static inline std::string text_cnt_lbl = "text-count";
      const static inline std::string tabl_cnt_lbl = "table-count";
      const static inline std::string fdoc_cnt_lbl = "document-count";
      
      const static inline std::vector<std::string> headers = {hash_lbl, flvr_lbl, name_lbl,
							      word_cnt_lbl, sent_cnt_lbl,
							      text_cnt_lbl, tabl_cnt_lbl, fdoc_cnt_lbl,
      							      text_lbl, nodes_text_lbl, tokens_text_lbl};
      
    public:

      base_node();
      
      base_node(flvr_type flvr, hash_type hash);
      base_node(flvr_type flvr, hash_type hash, const std::string& text);

      base_node(flvr_type flavor, const std::string& text);
      base_node(flvr_type flavor, const std::vector<std::string>& text);
      base_node(flvr_type flavor, const std::vector<hash_type>& path);

      bool is_valid() const { return (hash!=node_names::UNKNOWN_HASH or text_ptr!=NULL or
				      nodes_ptr!=NULL or edges_ptr!=NULL); }
      
      hash_type get_hash() const { return hash; };
      flvr_type get_flvr() const { return flvr; };

      std::string get_name() const { return node_names::to_name(flvr); };
      
      cnt_type count() const { return word_cnt; }
      ind_type length() const { return (nodes_ptr==NULL? 1: nodes_ptr->size()); }

      cnt_type get_word_cnt() const { return word_cnt; }
      cnt_type get_sent_cnt() const { return sent_cnt; }
      cnt_type get_text_cnt() const { return text_cnt; }
      cnt_type get_tabl_cnt() const { return tabl_cnt; }
      cnt_type get_fdoc_cnt() const { return fdoc_cnt; }
      cnt_type get_subj_cnt() const { return (text_cnt+tabl_cnt+fdoc_cnt); }

      void incr_word_cnt()                  { word_cnt += 1; }
      void incr_sent_cnt(bool present=true) { sent_cnt += (present? 1:0); }
      void incr_text_cnt(bool present=true) { text_cnt += (present? 1:0); }
      void incr_tabl_cnt(bool present=true) { tabl_cnt += (present? 1:0); }
      void incr_fdoc_cnt(bool present=true) { fdoc_cnt += (present? 1:0); }

      std::vector<hash_type> get_nodes() { return (nodes_ptr==NULL? std::vector<hash_type>({}): *nodes_ptr.get()); }
      std::vector<hash_type> get_edges() { return (edges_ptr==NULL? std::vector<hash_type>({}): *edges_ptr.get()); }

      std::string get_text() const;

      template<typename nodes_type>
      std::string get_text(nodes_type& nodes, bool connected) const;

      template<typename nodes_type>
      std::size_t get_token_path(nodes_type& nodes, std::vector<hash_type>& path) const;

      template<typename nodes_type>
      std::string get_token_text(nodes_type& nodes, std::vector<hash_type>& path) const;
      
      void clear();
      
      void update(const base_node& other);

      nlohmann::json to_json();

      template<typename nodes_type>
      nlohmann::json to_json(nodes_type& nodes);

      bool from_json(const nlohmann::json& node);

      nlohmann::json to_row();

      template<typename nodes_type>
      nlohmann::json to_row(nodes_type& nodes);

      friend bool operator<(const base_node& lhs, const base_node& rhs);
      
      friend std::ofstream& operator<<(std::ofstream& os, const base_node& node);
      friend std::ifstream& operator>>(std::ifstream& is, base_node& node);
      
    private:

      void initialise();

    private:

      // flavor of the node (see `node_names`)
      flvr_type flvr;
      
      // hash of the base_node
      hash_type hash;

      cnt_type word_cnt; // number of appearances in total
      cnt_type sent_cnt; // number of appearances in different sentences
      cnt_type text_cnt; // number of appearances in different texts
      cnt_type tabl_cnt; // number of appearances in different tables
      cnt_type fdoc_cnt; // number of appearances in different docs

      std::shared_ptr<std::string> text_ptr;

      std::shared_ptr<std::vector<hash_type>> nodes_ptr;
      std::shared_ptr<std::vector<hash_type>> edges_ptr;
    };

    base_node::base_node():
      flvr(node_names::UNKNOWN_FLVR),
      hash(node_names::UNKNOWN_HASH),

      word_cnt(0),
      sent_cnt(0),
      text_cnt(0),
      tabl_cnt(0),
      fdoc_cnt(0),

      text_ptr(NULL),

      nodes_ptr(NULL),
      edges_ptr(NULL)
    {}

    base_node::base_node(flvr_type flvr, hash_type hash):
      flvr(flvr),
      hash(hash),

      word_cnt(0),
      sent_cnt(0),

      text_cnt(flvr==node_names::TEXT? 1:0),
      tabl_cnt(flvr==node_names::TABL? 1:0),
      fdoc_cnt(flvr==node_names::FDOC? 1:0),

      text_ptr(NULL),

      nodes_ptr(NULL),
      edges_ptr(NULL)
    {}    

    /* this constructor is useful for text, table and document nodes */
    base_node::base_node(flvr_type flavor, hash_type hash, const std::string& text_):
      flvr(flavor),
      hash(hash),

      word_cnt(0),
      sent_cnt(0),
      text_cnt(0),
      tabl_cnt(0),
      fdoc_cnt(0),

      text_ptr(std::make_shared<std::string>(text_)),

      nodes_ptr(NULL),
      edges_ptr(NULL)
    {
      initialise();
    }
    
    base_node::base_node(flvr_type flavor, const std::string& text_):
      flvr(flavor),
      hash(node_names::UNKNOWN_HASH),

      word_cnt(0),
      sent_cnt(0),
      text_cnt(0),
      tabl_cnt(0),
      fdoc_cnt(0),

      text_ptr(std::make_shared<std::string>(text_)),

      nodes_ptr(NULL),
      edges_ptr(NULL)
    {
      initialise();
    }

    base_node::base_node(flvr_type flavor, const std::vector<std::string>& path):
      flvr(flavor),
      hash(node_names::UNKNOWN_HASH),

      word_cnt(0),
      sent_cnt(0),
      text_cnt(0),
      tabl_cnt(0),
      fdoc_cnt(0),

      text_ptr(NULL),

      nodes_ptr(std::make_shared<std::vector<hash_type> >()),
      edges_ptr(NULL)
    {
      for(std::string ptext:path)
        {
          base_node node(node_names::WORD_TOKEN, ptext);
          nodes_ptr->push_back(node.get_hash());
        }

      initialise();
    }

    base_node::base_node(flvr_type flavor, const std::vector<hash_type>& path):
      flvr(flavor),
      hash(node_names::UNKNOWN_HASH),
      
      word_cnt(0),
      sent_cnt(0),
      text_cnt(0),
      tabl_cnt(0),
      fdoc_cnt(0),
      
      text_ptr(NULL),

      nodes_ptr(std::make_shared<std::vector<hash_type> >(path)),
      edges_ptr(NULL)
    {
      initialise();
    }

    void base_node::clear()
    {
      hash = node_names::UNKNOWN_HASH;
      flvr = node_names::UNKNOWN_FLVR;

      word_cnt = 0;
      sent_cnt = 0;
      text_cnt = 0;
      tabl_cnt = 0;
      fdoc_cnt = 0;
      
      if( text_ptr!=NULL) { text_ptr.reset(); }
      if(nodes_ptr!=NULL) { nodes_ptr.reset(); }
      if(edges_ptr!=NULL) { edges_ptr.reset(); }      
    }
    
    void base_node::initialise()
    {
      if(hash!=node_names::UNKNOWN_HASH)
	{}
      else if(text_ptr!=NULL)
        {
          switch(flvr)
            {
	    case node_names::SUBW_TOKEN:
            case node_names::WORD_TOKEN:
            case node_names::SYNTX:	      
            case node_names::LABEL:
	    case node_names::SUBLABEL:
              {
                std::string item = "__"+node_names::to_name(flvr)+"_"+*text_ptr.get()+"__";
                hash = utils::to_reproducible_hash(item);
              }
              break;

            default:
              {
                //LOG_S(ERROR) << "no support for flvr " << node_names::to_name.at(flvr) << " "
		//<< "in word " << *text_ptr.get();
                hash = node_names::UNKNOWN_HASH;
              }
            }
        }
      else if(nodes_ptr!=NULL and edges_ptr==NULL)
        {
          std::vector<hash_type> norm_path = *nodes_ptr.get();

          switch(flvr)
            {
            case node_names::CONT:
              {
                std::sort(norm_path.begin(), norm_path.end());
              }
              break;

            case node_names::TERM:
              {
                std::sort(norm_path.begin(), --(norm_path.end()));
              }
              break;

            default:
              {}
            }

          norm_path.push_back(flvr);
          hash = utils::to_hash(norm_path);
        }
      else
        {
          LOG_S(ERROR) << __FILE__ << ":" << __LINE__ << " "
                       << "no initialisation defined!!";
        }
    }

    void base_node::update(const base_node& other)
    {
      if(hash!=other.get_hash() or
	 flvr!=other.get_flvr() )
        {
          LOG_S(ERROR) << "mis-matching nodes ...";

          LOG_S(ERROR) << " -> " << hash       << ", "
                       << "name: '" << std::setw(12) << get_name() << "', "
                       << "text: '" << std::setw(12) << get_text() << "'";

          LOG_S(ERROR) << " -> " << other.get_hash() << ", "
                       << "name: '" << std::setw(12) << other.get_name() << "', "
                       << "text: '" << std::setw(12) << other.get_text() << "'";
        }

      word_cnt += other.get_word_cnt();
      sent_cnt += other.get_sent_cnt();
      text_cnt += other.get_text_cnt();
      tabl_cnt += other.get_tabl_cnt();
      fdoc_cnt += other.get_fdoc_cnt();
    }

    std::string base_node::get_text() const
    {
      std::string res="<not-resolved>";

      if(text_ptr!=NULL)
        {
          res = *text_ptr.get();
        }

      return res;
    }

    template<typename nodes_type>
    std::size_t base_node::get_token_path(nodes_type& nodes, std::vector<hash_type>& path) const
    {
      path.clear();
      
      switch(flvr)
	{
	case node_names::WORD_TOKEN:
	  {
	    path.push_back(hash);
	  }
	  break;

	case node_names::CONN:
	case node_names::VERB:
	case node_names::TERM:
	case node_names::SENT:
	case node_names::TEXT:
	  {
	    if(nodes_ptr==NULL)
	      {
		//LOG_S(WARNING) << "nodes is NULL for " << node_names::to_name(flvr);
		return 0;
	      }
	    
	    for(hash_type hash:*nodes_ptr.get())
	      {
		base_node node;
		if(nodes.get(hash, node))
		  {
		    std::vector<hash_type> new_path={};
		    node.get_token_path(nodes, new_path);

		    for(auto hash:new_path)
		      {
			path.push_back(hash);
		      }
		  }
	      }
	  }
	  break;

	case node_names::TABL: 
	case node_names::SECT:
	case node_names::FDOC:
	case node_names::BOOK:
	  {}
	  break;

	default:
	  {}
	}

      return path.size();
    }

    template<typename nodes_type>
    std::string base_node::get_token_text(nodes_type& nodes, std::vector<hash_type>& path) const
    {
      get_token_path(nodes, path);      
      
      std::stringstream ss;
      for(std::size_t l=0; l<path.size(); l++)
	{
	  std::string conn = (l+1==path.size()? "":" ");
	  
	  base_node node;
	  if(nodes.get(path.at(l), node))
	    {
	      std::string token = node.get_text();
	      ss << token << conn;
	    }
	}

      return ss.str();
    }

    template<typename nodes_type>
    std::string base_node::get_text(nodes_type& nodes_coll, bool connected) const
    {
      std::string conn="";
      if(connected)
	{
          switch(flvr)
            {
            case node_names::CONT:
              {
                conn="-";
              }
              break;
	      /*
            case node_names::CONN:
            case node_names::VERB:
            case node_names::TERM:
              {
                conn="_";
              }
              break;

            case node_names::SENT:
            case node_names::TEXT:
	      {
                conn=" ";		
	      }
	      break;

            case node_names::TABL:
            case node_names::SECT:
            case node_names::FDOC:
	    case node_names::BOOK:
	      {
		return "<unresolved>";
	      }
	      break;
	      */
	      
            default:
              {
                //conn=" ";
		conn="";
              }
            }
	}
      
      if(text_ptr!=NULL)
        {
          std::string res = *text_ptr.get();
          return res;
        }
      else if(nodes_ptr!=NULL)
        {
          std::stringstream ss;
          for(hash_type phash:*nodes_ptr.get())
            {
	      base_node node;			
              if(nodes_coll.get(phash, node))
                {
                  ss << node.get_text(nodes_coll, connected) << conn;
                }
              else
                {
                  ss << "__unknown__" << conn;
                }
            }
	  
          std::string res = ss.str();
	  if(flvr==node_names::CONT)
	    {
	      res.pop_back();
	    }
	  
	  res = utils::replace(res, word_token::get_space(), " ");
	  res = utils::strip(res);
	  
          return res;
        }
      else if(flvr==node_names::TEXT or node_names::TABL or node_names::FDOC)
	{
	  return "";
	}
      else
        {
	  //assert(false);
          LOG_S(ERROR) << __FILE__ << ":" << __LINE__ << " "
                       << "both text and nodes are NULL: can not resolve text!";

          return "__NULL__";
        }
    }

    nlohmann::json base_node::to_json()
    {
      nlohmann::json data = nlohmann::json::object({});
      {
        data[hash_lbl] = hash;
        data[flvr_lbl] = flvr;
        data[name_lbl] = node_names::to_name(flvr);

        data[text_lbl] = nlohmann::json::value_t::null;
        if(text_ptr!=NULL)
          {
            data[text_lbl] = *text_ptr.get();
          }

        data[nodes_lbl] = nlohmann::json::value_t::null;
        if(nodes_ptr!=NULL)
          {
            data[nodes_lbl] = *nodes_ptr.get();
          }

        data[edges_lbl] = nlohmann::json::value_t::null;
        if(edges_ptr!=NULL)
          {
            data[edges_lbl] = *edges_ptr.get();
          }

        data[cnt_lbl] = nlohmann::json::object({});
        data[cnt_lbl][word_cnt_lbl] = word_cnt;
        data[cnt_lbl][sent_cnt_lbl] = sent_cnt;
        data[cnt_lbl][text_cnt_lbl] = text_cnt;
        data[cnt_lbl][tabl_cnt_lbl] = tabl_cnt;
        data[cnt_lbl][fdoc_cnt_lbl] = fdoc_cnt;
      }

      return data;
    }

    template<typename nodes_type>
    nlohmann::json base_node::to_json(nodes_type& nodes_coll)
    {
      nlohmann::json data = to_json();

      {
	std::string tmp = get_text(nodes_coll, true);
	data[nodes_text_lbl] = tmp;
      }

      {
	std::vector<hash_type> tokens_path = {};
	get_token_path(nodes_coll, tokens_path);
	
	std::string tokens_text = get_token_text(nodes_coll, tokens_path);

	data[tokens_path_lbl] = tokens_path;
	data[tokens_text_lbl] = tokens_text;
      }      
      
      return data;
    }
    
    bool base_node::from_json(const nlohmann::json& data)
    {
      hash = data[hash_lbl].get<hash_type>();
      flvr = data[flvr_lbl].get<flvr_type>();

      if(not data[text_lbl].is_null())
        {
          std::string item = data[text_lbl].get<std::string>();
          text_ptr = std::make_shared<std::string>(item);
        }

      if(not data[nodes_lbl].is_null())
        {
          std::vector<hash_type> item = data[nodes_lbl].get<std::vector<hash_type> >();
          nodes_ptr = std::make_shared<std::vector<hash_type> >(item);
        }

      if(not data[edges_lbl].is_null())
        {
          std::vector<hash_type> item = data[edges_lbl].get<std::vector<hash_type> >();
          edges_ptr = std::make_shared<std::vector<hash_type> >(item);
        }

      word_cnt = data[cnt_lbl][word_cnt_lbl].get<cnt_type>();
      sent_cnt = data[cnt_lbl][sent_cnt_lbl].get<cnt_type>();
      text_cnt = data[cnt_lbl][text_cnt_lbl].get<cnt_type>();
      tabl_cnt = data[cnt_lbl][tabl_cnt_lbl].get<cnt_type>();
      fdoc_cnt = data[cnt_lbl][fdoc_cnt_lbl].get<cnt_type>();

      return true;
    }

    nlohmann::json base_node::to_row()
    {
      nlohmann::json row = nlohmann::json::array({});
      {
	row.push_back(hash);
	row.push_back(flvr);
        row.push_back(node_names::to_name(flvr));

	row.push_back(word_cnt);
	row.push_back(sent_cnt);
	row.push_back(text_cnt);
	row.push_back(tabl_cnt);
	row.push_back(fdoc_cnt);

	if(text_ptr==NULL)
	  {
	    row.push_back(nlohmann::json::value_t::null);
	  }
	else
	  {
	    row.push_back(*text_ptr.get());	    
	  }

	row.push_back(nlohmann::json::value_t::null);
	row.push_back(nlohmann::json::value_t::null);
      }

      return row;
    }

    template<typename nodes_type>
    nlohmann::json base_node::to_row(nodes_type& nodes)
    {
      nlohmann::json row = nlohmann::json::array({});
      {
	row.push_back(hash);
	row.push_back(flvr);
        row.push_back(node_names::to_name(flvr));

	row.push_back(word_cnt);
	row.push_back(sent_cnt);
	row.push_back(text_cnt);
	row.push_back(tabl_cnt);
	row.push_back(fdoc_cnt);

	if(text_ptr==NULL)
	  {
	    row.push_back(nlohmann::json::value_t::null);
	  }
	else
	  {
	    row.push_back(*text_ptr.get());	    
	  }

	{
	  std::string item = get_text(nodes, true);
	  row.push_back(item);
	}

	{
	  std::string item = get_text(nodes, false);
	  row.push_back(item);		
	}
      }

      return row;
    }

    bool operator<(const base_node& lhs, const base_node& rhs)
    {
      if(lhs.flvr==rhs.flvr)
        {
          if(lhs.word_cnt==rhs.word_cnt)
            {
              return lhs.hash<rhs.hash;
            }

          return lhs.word_cnt>rhs.word_cnt;
        }

      return lhs.flvr<rhs.flvr;
    }

    std::ofstream& operator<<(std::ofstream& os, const base_node& node)
    {
      assert(node.is_valid());
      
      os.write((char*)&node.hash, sizeof(node.hash));
      os.write((char*)&node.flvr, sizeof(node.flvr));

      os.write((char*)&node.word_cnt, sizeof(node.word_cnt));
      os.write((char*)&node.sent_cnt, sizeof(node.sent_cnt));
      os.write((char*)&node.text_cnt, sizeof(node.text_cnt));
      os.write((char*)&node.tabl_cnt, sizeof(node.tabl_cnt));
      os.write((char*)&node.fdoc_cnt, sizeof(node.fdoc_cnt));

      auto& text_ptr = node.text_ptr;
      auto& nodes_ptr = node.nodes_ptr;
      auto& edges_ptr = node.edges_ptr;
      
      int16_t chars_len = ( text_ptr==NULL? -1 :  text_ptr.get()->size());
      int16_t nodes_len = (nodes_ptr==NULL? -1 : nodes_ptr.get()->size());
      int16_t edges_len = (edges_ptr==NULL? -1 : edges_ptr.get()->size());

      os.write((char*)&chars_len, sizeof(chars_len));
      os.write((char*)&nodes_len, sizeof(nodes_len));
      os.write((char*)&edges_len, sizeof(edges_len));

      if(chars_len>-1)
        {
          os.write(text_ptr.get()->c_str(),  sizeof(char)*chars_len);
        }

      if(nodes_len>-1)
        {
          for(const auto& nhash:*nodes_ptr.get())
            {
              os.write((char*)&nhash, sizeof(nhash));
            }
        }

      if(edges_len>-1)
        {
          for(const auto& ehash:*edges_ptr.get())
            {
              os.write((char*)&ehash, sizeof(ehash));
            }
        }

      return os;
    }

    std::ifstream& operator>>(std::ifstream& is, base_node& node)
    {
      typedef typename base_node::hash_type hash_type;
      
      is.read((char*)&node.hash, sizeof(node.hash));
      is.read((char*)&node.flvr, sizeof(node.flvr));

      is.read((char*)&node.word_cnt, sizeof(node.word_cnt));
      is.read((char*)&node.sent_cnt, sizeof(node.sent_cnt));
      is.read((char*)&node.text_cnt, sizeof(node.text_cnt));
      is.read((char*)&node.tabl_cnt, sizeof(node.tabl_cnt));
      is.read((char*)&node.fdoc_cnt, sizeof(node.fdoc_cnt));

      int16_t chars_len, nodes_len, edges_len;

      is.read((char*)&chars_len, sizeof(chars_len));
      is.read((char*)&nodes_len, sizeof(nodes_len));
      is.read((char*)&edges_len, sizeof(edges_len));
      
      if(chars_len>-1)
        {
          std::string line(chars_len, ' ');
          is.read((char*)&line[0],  sizeof(char)*chars_len);
	  
          node.text_ptr = std::make_shared<std::string>(line);
        }

      if(nodes_len>-1)
        {	  
          std::vector<hash_type> nhashes(nodes_len);
          for(hash_type& nhash:nhashes)
            {
              is.read((char*)&nhash, sizeof(nhash));
            }

          node.nodes_ptr = std::make_shared<std::vector<hash_type> >(nhashes);
        }

      if(edges_len>-1)
        {
          std::vector<hash_type> ehashes(edges_len);
          for(hash_type& ehash:ehashes)
            {
              is.read((char*)&ehash, sizeof(ehash));
            }

          node.edges_ptr = std::make_shared<std::vector<hash_type> >(ehashes);
        }
      
      assert(node.is_valid());
      
      return is;
    }
    
  }

}

#endif
