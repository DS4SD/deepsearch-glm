//-*-C++-*-

#ifndef ANDROMEDA_SUBJECTS_TABLE_H_
#define ANDROMEDA_SUBJECTS_TABLE_H_

namespace andromeda
{

  template<>
  class subject<TABLE>: public base_subject
  {

  public:

    typedef table_element table_element_type;

  public:

    subject();
    subject(uint64_t dhash, std::string dloc);
    subject(uint64_t dhash, std::string dloc, std::shared_ptr<prov_element> prov);

    virtual ~subject();

    void clear();

    std::string get_path() const { return (provs.size()>0? (provs.at(0)->get_path()):"#"); }
    bool is_valid() { return (base_subject::valid); }

    //virtual nlohmann::json to_json();
    virtual nlohmann::json to_json(const std::set<std::string> filters);
    virtual bool from_json(const nlohmann::json& data);

    bool set_data(const nlohmann::json& data);

    bool set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                    std::shared_ptr<utils::text_normaliser> text_normaliser);

    bool set(nlohmann::json& data,
             std::shared_ptr<utils::char_normaliser> char_normaliser,
             std::shared_ptr<utils::text_normaliser> text_normaliser);

    void sort();

    typename std::vector<base_instance>::iterator insts_beg(std::array<uint64_t, 2> coor);
    typename std::vector<base_instance>::iterator insts_end(std::array<uint64_t, 2> coor);

    void show(bool prps, bool insts, bool rels);

    uint64_t get_hash() const { return hash; }
    std::string get_text() const;

    uint64_t num_rows() const { return nrows; }
    uint64_t num_cols() const { return ncols; }

    table_element_type& operator()(std::array<uint64_t,2> coor) { return data.at(coor.at(0)).at(coor.at(1)); }
    table_element_type& operator()(uint64_t i, uint64_t j) { return data.at(i).at(j); }

  private:

    void set_hash();

  public:

    std::vector<std::shared_ptr<prov_element> > provs;

    std::vector<std::shared_ptr<subject<TEXT> > > captions;
    std::vector<std::shared_ptr<subject<TEXT> > > footnotes;
    std::vector<std::shared_ptr<subject<TEXT> > > mentions;

    uint64_t nrows, ncols;
    std::vector<std::vector<table_element_type> > data;
  };

  subject<TABLE>::subject():
    base_subject(TABLE),

    provs({}),

    captions({}),
    footnotes({}),
    mentions({}),

    nrows(0),
    ncols(0),

    data({})
  {}

  subject<TABLE>::subject(uint64_t dhash, std::string dloc):
    base_subject(dhash, dloc, TABLE),

    provs({}),

    captions({}),
    footnotes({}),
    mentions({}),

    nrows(0),
    ncols(0),

    data({})
  {}

  subject<TABLE>::subject(uint64_t dhash, std::string dloc,
                          std::shared_ptr<prov_element> prov):
    base_subject(dhash, dloc, TABLE),

    provs({prov}),

    captions({}),
    footnotes({}),
    mentions({}),

    nrows(0),
    ncols(0),

    data({})
  {}

  subject<TABLE>::~subject()
  {}

  void subject<TABLE>::clear()
  {
    base_subject::clear();

    provs.clear();

    captions.clear();
    footnotes.clear();
    mentions.clear();

    nrows=0;
    ncols=0;

    data.clear();
  }

  nlohmann::json subject<TABLE>::to_json(const std::set<std::string> filters)
  {
    nlohmann::json result = base_subject::_to_json(filters);
    result[base_subject::prov_lbl] = base_subject::get_prov_refs(provs);
      
    {
      result["#-rows"] = nrows;
      result["#-cols"] = ncols;

      auto& json_table = result[base_subject::table_data_lbl];
      for(index_type i=0; i<nrows; i++)
        {
          nlohmann::json row = nlohmann::json::array({});

          for(index_type j=0; j<ncols; j++)
            {
              auto cell = data.at(i).at(j).to_json();
              row.push_back(cell);
            }

          json_table.push_back(row);
        }

      result[base_subject::table_data_lbl] = json_table;
    }

    if(filters.size()==0 or filters.count(base_subject::captions_lbl))
      {
        base_subject::to_json(result, base_subject::captions_lbl, captions, filters);
      }

    if(filters.size()==0 or filters.count(base_subject::footnotes_lbl))
      {
        base_subject::to_json(result, base_subject::footnotes_lbl, footnotes, filters);
      }

    if(filters.size()==0 or filters.count(base_subject::mentions_lbl))
      {
        base_subject::to_json(result, base_subject::mentions_lbl, mentions, filters);
      }

    return result;
  }

  bool subject<TABLE>::from_json(const nlohmann::json& json_table)
  {
    {
      nlohmann::json grid = json_table.at("data");
      
      for(ind_type i=0; i<grid.size(); i++)
	{
	  data.push_back({});
	  for(ind_type j=0; j<grid.at(i).size(); j++)
	    {
	      table_element cell(grid.at(i).at(j));
	      data.back().push_back(cell);
	    }
	}
    }
    
    base_subject::from_json(json_table, base_subject::captions_lbl, captions);
    base_subject::from_json(json_table, base_subject::footnotes_lbl, footnotes);
    base_subject::from_json(json_table, base_subject::mentions_lbl, mentions);

    return true;
  }

  bool subject<TABLE>::set_data(const nlohmann::json& item)
  {
    base_subject::clear_models();
    data.clear();

    if(item.count("data"))
      {
        nlohmann::json grid = item.at("data");

        std::set<int> ncols={};
        for(ind_type i=0; i<grid.size(); i++)
          {
            data.push_back({});
            for(ind_type j=0; j<grid.at(i).size(); j++)
              {
                std::string text = "";
                if(grid.at(i).at(j).count("text"))
                  {
                    text = grid.at(i).at(j).at("text");
                  }

                std::array<uint64_t,2> row_span={i,i+1};
                std::array<uint64_t,2> col_span={j,j+1};

                if(grid.at(i).at(j).count("spans"))
                  {
                    auto& spans = grid.at(i).at(j).at("spans");

                    for(auto& span:spans)
                      {
                        uint64_t si = span[0].get<uint64_t>();
                        uint64_t sj = span[1].get<uint64_t>();

                        row_span[0] = std::min(row_span[0], si);
                        row_span[1] = std::max(row_span[1], si);

                        col_span[0] = std::min(col_span[0], sj);
                        col_span[1] = std::max(col_span[1], sj);
                      }
                  }

                data.back().emplace_back(i,j,row_span,col_span,text);
              }
          }
      }

    if(data.size()>0)
      {
        nrows = data.size();
        ncols = data.at(0).size();

        set_hash();
        base_subject::valid = true;
      }
    else
      {
        data.clear();

        nrows=0;
        ncols=0;

        base_subject::valid = false;
      }

    return base_subject::valid;
  }

  void subject<TABLE>::set_hash()
  {
    std::vector<uint64_t> hashes={dhash};
    for(std::size_t i=0; i<data.size(); i++)
      {
        for(std::size_t j=0; j<data.at(i).size(); j++)
          {
            hashes.push_back(data.at(i).at(j).text_hash);
          }
      }

    hash = utils::to_hash(hashes);
  }

  bool subject<TABLE>::set_tokens(std::shared_ptr<utils::char_normaliser> char_normaliser,
                                  std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    valid = true;

    for(auto& row:data)
      {
        for(auto& cell:row)
          {
            valid = (valid and cell.set_tokens(char_normaliser, text_normaliser));
          }
      }

    return valid;
  }

  bool subject<TABLE>::set(nlohmann::json& grid,
                           std::shared_ptr<utils::char_normaliser> char_normaliser,
                           std::shared_ptr<utils::text_normaliser> text_normaliser)
  {
    bool task_0 = set_data(grid);
    bool task_1 = set_tokens(char_normaliser, text_normaliser);

    return (task_0 and task_1);
  }

  void subject<TABLE>::sort()
  {
    std::sort(instances.begin(), instances.end());
  }

  typename std::vector<base_instance>::iterator subject<TABLE>::insts_beg(std::array<uint64_t, 2> coor)
  {
    range_type min_range = {0, 0};

    base_instance fake(base_subject::hash, NULL_MODEL, "fake", "fake", "fake", coor, min_range, min_range,
                       min_range, min_range, min_range);

    return std::lower_bound(instances.begin(), instances.end(), fake);
  }

  typename std::vector<base_instance>::iterator subject<TABLE>::insts_end(std::array<uint64_t, 2> coor)
  {
    range_type max_range =
      { std::numeric_limits<uint64_t>::max(),
        std::numeric_limits<uint64_t>::max()};

    base_instance fake(base_subject::hash, NULL_MODEL, "fake", "fake", "fake", coor, max_range, max_range,
                       max_range, max_range, max_range);

    return std::upper_bound(instances.begin(), instances.end(), fake);
  }

  void subject<TABLE>::show(bool prps, bool insts, bool rels)
  {
    std::vector<std::vector<std::string> > grid={};
    for(uint64_t i=0; i<data.size(); i++)
      {
        grid.push_back({});
        for(uint64_t j=0; j<data.at(i).size(); j++)
          {
            grid.at(i).push_back(data.at(i).at(j).text);
          }
      }

    std::stringstream ss;

    if(provs.size()>0)
      {
        ss << "prov: "
           << provs.at(0)->get_page() << ", "
           << " ["
           << provs.at(0)->x0() << ", "
           << provs.at(0)->y0() << ", "
           << provs.at(0)->x1() << ", "
           << provs.at(0)->y1()
           << "]";
      }

    {
      ss << "\ntable: ";
      utils::show_table(grid, ss, 48);
    }

    //if(mdls)
    {
      ss << "\nmodels: ";
      for(auto model:applied_models)
        {
          ss << model << ", ";
        }
      ss << "[done]\n";
    }

    if(prps)
      {
        ss << tabulate(properties);
      }

    if(insts)
      {
        ss << tabulate(instances);
      }

    if(rels)
      {
        ss << tabulate(instances, relations);
      }

    LOG_S(INFO) << "NLP-output: \n" << ss.str();
  }

  std::string subject<TABLE>::get_text() const
  {
    std::stringstream ss;

    for(uint64_t i=0; i<data.size(); i++)
      {
        for(uint64_t j=0; j<data.at(i).size(); j++)
          {
            if(j+1==data.at(i).size())
              {
                ss << data.at(i).at(j).text << "\n";
              }
            else
              {
                ss << data.at(i).at(j).text << ", ";
              }
          }
      }

    std::string text = ss.str();
    return text;
  }

}

#endif
