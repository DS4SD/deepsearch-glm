//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_LOG_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_LOG_H_

namespace andromeda
{
  namespace glm
  {
    class create_log
    {
      //typedef std::vector<std::string> row_type;
      typedef std::tuple<std::size_t, // thread-id

			 std::size_t, // total line-count
			 std::size_t, // local line-count
			 std::size_t, // incr line-count
			 std::size_t, std::size_t, std::size_t, std::size_t,// total count
                         double, double, double, double, // % increase 
			 double, double, double, double,// timings [msec]
			 double, double, double, double// max-load-factor                        
			 > row_type;

    public:

      create_log(std::string log_dir);

      template<typename glm_model_type>
      void log(std::size_t id, std::size_t line, std::size_t loc_lines,
	       double merge_time, std::shared_ptr<glm_model_type> model);
      
      void save();
      
    private:

      std::filesystem::path log_dir;

      std::string ctime;
      
      std::filesystem::path
	filename_csv, filepath_csv,
	filename_txt, filepath_txt;

      std::chrono::time_point<std::chrono::system_clock> start, prev;
      std::map<std::size_t, std::chrono::time_point<std::chrono::system_clock> > prev_id;

      std::chrono::duration<double, std::milli> total_t, delta_t, delta_id;

      std::vector<std::string> header;
      std::vector<row_type> data;
    };

    create_log::create_log(std::string log_dir):
      log_dir(log_dir),
      ctime(utils::get_current_time()),
      
      filename_csv("model-creation-"+ctime+".csv"),
      filepath_csv(log_dir/filename_csv),
      
      filename_txt("model-creation-"+ctime+".txt"),
      filepath_txt(log_dir/filename_txt),
      
      start(std::chrono::system_clock::now()),
      prev(std::chrono::system_clock::now()),

      prev_id(),

      total_t(prev-start), delta_t(prev-start),  delta_id(prev-start),

      header({"id", "total-lines", "local-lines", "cumulative-lines",
              "nodes", "edges", "tokens", "concepts", 
	      "new-nodes [%]", "new-edges [%]", "new-tokens [%]", "new-concepts [%]", 
	      "time [msec]", "delta [msec]", "delta-id [msec]",  "merge [msec]",
	      "load-factor nodes", "load-factor edges", "max-load-factor nodes", "max-load-factor edges"
	}),
      data({})
    {
      if(not std::filesystem::exists(log_dir))
        {
          std::filesystem::create_directory(log_dir);
        }
    }

    template<typename glm_model_type>
    void create_log::log(std::size_t id, std::size_t tot_lines, std::size_t loc_lines,
			 double merge_time, std::shared_ptr<glm_model_type> model)
    {
      auto now = std::chrono::system_clock::now();

      total_t = (now-start);
      delta_t = (now-prev);

      if(prev_id.count(id)==0)
        {
          delta_id = (now-start);
        }
      else
        {
	  auto tmp = prev_id.at(id);
          delta_id = (now-tmp);
        }

      prev = now;
      prev_id[id] = now;

      std::size_t cum_lines = loc_lines;

      auto& nodes = model->get_nodes();
      auto& edges = model->get_edges();

      auto curr_nodes = nodes.size();
      auto curr_edges = edges.size();

      auto curr_tokens   = nodes.size(node_names::WORD_TOKEN);
      auto curr_concepts = nodes.size(node_names::TERM);
      
      double lf_nodes = nodes.load_factor();
      double lf_edges = edges.load_factor();

      double mlf_nodes = nodes.max_load_factor();
      double mlf_edges = edges.max_load_factor();
      
      double perc_nodes = 100.0;
      double perc_edges = 100.0;

      double perc_tokens = 100.0;
      double perc_concepts = 100.0;
      
      if(data.size()==0)
        {
          row_type row
            = std::make_tuple(id, tot_lines, loc_lines, cum_lines,
			      curr_nodes, curr_edges, curr_tokens, curr_concepts,
			      perc_nodes, perc_edges, perc_tokens, perc_concepts,
                              (total_t).count(), (delta_t).count(), (delta_id).count(), merge_time,
			      lf_nodes, lf_edges, mlf_nodes, mlf_edges);
	  
	  data.push_back(row);
        }
      else
	{
	  auto last = data.back();

	  cum_lines += std::get<3>(last);

	  auto prev_nodes    = std::get<4>(last);
	  auto prev_edges    = std::get<5>(last);
	  auto prev_tokens   = std::get<6>(last);
	  auto prev_concepts = std::get<7>(last);
	  
	  perc_nodes    = 100*double(curr_nodes    - prev_nodes    )/( 1e-6+double(prev_nodes   ));
	  perc_edges    = 100*double(curr_edges    - prev_edges    )/( 1e-6+double(prev_edges   ));
	  perc_tokens   = 100*double(curr_tokens   - prev_tokens   )/( 1e-6+double(prev_tokens  ));
	  perc_concepts = 100*double(curr_concepts - prev_concepts )/( 1e-6+double(prev_concepts));
	  
          row_type row
            = std::make_tuple(id, tot_lines, loc_lines, cum_lines, // 1-3
			      curr_nodes, curr_edges, curr_tokens, curr_concepts,
			      perc_nodes, perc_edges, perc_tokens, perc_concepts,			      
			      (total_t).count(), (delta_t).count(), (delta_id).count(), merge_time, 
			      lf_nodes, lf_edges, mlf_nodes, mlf_edges); 

	  data.push_back(row);
	}

      {
	if(data.size()==1)
	  {
	    LOG_S(INFO) 
	      << std::setw(2) << "id" << ", "
	      << std::scientific << std::setprecision(2)
	      << std::setw(6) << "tot-text" << " ("
	      << std::setw(6) << "loc-text" << "), "
	      
	  << std::setw(6) << "#-nodes" << ", "
	  << std::setw(6) << "#-edges" << ", "
	  << std::setw(6) << "#-token" << ", "
	  << std::setw(6) << "#-terms" << ": ["

	  << std::setw(6) << "#-nodes" << " %, "
	  << std::setw(6) << "#-edges" << " %, "
	  << std::setw(6) << "#-token" << " %, "
	  << std::setw(6) << "#-terms" << " %]"
	  << std::flush;
	  }
	
	auto& last = data.back();
	LOG_S(INFO) 
	  << std::setw(2) << id << ", "
	  << std::scientific << std::setprecision(2)
	  << std::setw(6) << double(cum_lines) << " ("
	  << std::setw(6) << double(loc_lines) << "), "
	  
	  << std::setw(6) << double(std::get<4>(last)) << ", "
	  << std::setw(6) << double(std::get<5>(last)) << ", "
	  << std::setw(6) << double(std::get<6>(last)) << ", "
	  << std::setw(6) << double(std::get<7>(last)) << ": ["

	  << std::setw(6) << double(std::get< 8>(last)) << " %, "
	  << std::setw(6) << double(std::get< 9>(last)) << " %, "
	  << std::setw(6) << double(std::get<10>(last)) << " %, "
	  << std::setw(6) << double(std::get<11>(last)) << " %]"
	  << std::flush;
      }
    }

    void create_log::save()
    {
      utils::to_csv(filepath_csv, header, data);
      utils::to_txt(filepath_txt, header, data);
    }
    
  }

}

#endif
