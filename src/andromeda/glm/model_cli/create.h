//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_H_
#define ANDROMEDA_MODELS_GLM_MODEL_CLI_CREATE_H_

#include <future>
#include <thread>
#include <mutex>

#include <andromeda/glm/model_cli/create/config.h>
#include <andromeda/glm/model_cli/create/logger.h>
#include <andromeda/glm/model_cli/create/model_merger.h>
#include <andromeda/glm/model_cli/create/model_creator.h>

namespace andromeda
{
  namespace glm
  {    
    template<typename model_type>
    class model_cli<CREATE, model_type>
    {
      typedef typename model_type::hash_type hash_type;
      typedef typename model_type::index_type index_type;

      typedef typename model_type::node_type node_type;
      typedef typename model_type::edge_type edge_type;

      typedef typename model_type::nodes_type nodes_type;
      typedef typename model_type::edges_type edges_type;

    public:

      model_cli(std::shared_ptr<model_type> model);
      
      model_cli(std::shared_ptr<model_type> model,
		nlohmann::json config);

      ~model_cli();

      nlohmann::json to_config();
      
      void create(std::shared_ptr<base_producer>& producer);
      
    private:

      void initialise();
      void finalise();

      template<typename producer_type>
      void update_mt(std::shared_ptr<producer_type>& producer);      

      template<typename producer_type>
      std::size_t update_task(std::size_t thread_id,
			      std::mutex& read_mtx, std::mutex& update_mtx,			      
			      std::size_t& line_count, std::atomic<std::size_t>& merge_count,
			      nlohmann::json& config, std::shared_ptr<producer_type>& reader,
			      std::shared_ptr<create_log> log,
			      std::shared_ptr<model_type> loc_model,
			      std::shared_ptr<model_type> fin_model);
      
    private:

      std::shared_ptr<model_type> model_ptr;

      create_config configuration;
    };

    template<typename model_type>
    model_cli<CREATE, model_type>::model_cli(std::shared_ptr<model_type> model_ptr):
      model_ptr(model_ptr),
      configuration()
    {}
    
    template<typename model_type>
    model_cli<CREATE, model_type>::model_cli(std::shared_ptr<model_type> model_ptr,
					     nlohmann::json config):
      model_ptr(model_ptr),
      configuration(config)
    {}

    template<typename model_type>
    model_cli<CREATE, model_type>::~model_cli()
    {}

    template<typename model_type>
    nlohmann::json model_cli<CREATE, model_type>::to_config()
    {
      nlohmann::json config = nlohmann::json::object({});
      config["mode"] = to_string(CREATE);

      {
	config["parameters"] = (model_ptr->get_parameters()).to_json();
      }
      
      {
	nlohmann::json item = configuration.to_json();      
	config.merge_patch(item);
      }
      
      {
	nlohmann::json item = model_op<SAVE>::to_config();
	config.merge_patch(item);
      }
      
      {
	nlohmann::json item = model_op<LOAD>::to_config();
	config.merge_patch(item);
      }    
      
      return config;
    }

    template<typename model_type>
    void model_cli<CREATE, model_type>::create(std::shared_ptr<base_producer>& producer)
    {
      initialise();

      switch(producer->get_subject_name())
	{
	case TEXT:
	  {
	    auto producer_ = std::dynamic_pointer_cast<andromeda::producer<andromeda::TEXT> >(producer);
	    update_mt(producer_);
	  }
	  break;
	  
	case DOCUMENT:
	  {
	    auto producer_ = std::dynamic_pointer_cast<andromeda::producer<andromeda::DOCUMENT> >(producer);
	    update_mt(producer_);
	  }
	  break;	  

	default:
	  {
	    LOG_S(WARNING) << "GLM-model create does not support producer-type: "
			   << to_string(producer->get_subject_name());
	      
	  }
	}
      
      finalise();

      LOG_S(INFO) << "done creating glm!";
    }    

    template<typename model_type>
    void model_cli<CREATE, model_type>::initialise()
    {
      LOG_S(INFO) << "initialise glm: ";
      LOG_S(INFO) << " -> reserve nodes: "
		  << std::scientific << std::setprecision(2)
		  << configuration.max_total_nodes;
      LOG_S(INFO) << " -> reserve edges: "
		  << std::scientific << std::setprecision(2)
		  << configuration.max_total_edges;

      model_ptr->initialise(configuration.max_total_nodes,
			    configuration.max_total_edges);
    }

    template<typename model_type>
    void model_cli<CREATE, model_type>::finalise()
    {
      LOG_S(INFO) << "finalise glm";

      model_cli<AUGMENT, model_type> augmenter(model_ptr);
      augmenter.augment();
    }

    template<typename model_type>
    template<typename producer_type>
    void model_cli<CREATE, model_type>::update_mt(std::shared_ptr<producer_type>& producer)
    {
      LOG_S(INFO) << "start creating glm (#-threads: " << configuration.num_threads << ") ...";
      
      std::shared_ptr<producer_type> reader = producer;
      {
	reader->reset_pointer();
      }

      std::vector<std::shared_ptr<model_type> > models;
      {
	for(std::size_t id=0; id<configuration.num_threads; id++)
	  {
	    auto model_ptr = std::make_shared<model_type>();
	    models.push_back(model_ptr);
	  }
      }

      std::mutex read_mtx, update_mtx;

      std::size_t line_count=0;
      std::atomic<std::size_t> merge_count=0;
      
      nlohmann::json config = (model_ptr->get_parameters()).to_json();
      std::vector<std::future<std::size_t> > results(configuration.num_threads);

      std::shared_ptr<create_log> log = std::make_shared<create_log>(configuration.model_dir);      
      
      std::size_t total_text_read=0;

      auto& nodes = model_ptr->get_nodes();
      auto& edges = model_ptr->get_edges();
      
      edges.show_bucket_distribution();
      
      if(configuration.num_threads==1)
	{
	  LOG_S(INFO) << "launching single threaded mode ...";
	  
	  update_task(0, read_mtx, update_mtx,
		      line_count, merge_count,
		      config, reader, log,
		      models.at(0), model_ptr);
	}
      else
	{
	  LOG_S(INFO) << "launching " << results.size() << " threads ...";	    
	  for(std::size_t id=0; id<results.size(); id++)
	    {
	      results.at(id) = std::async(std::launch::async,
					  &model_cli<CREATE, model_type>::update_task<producer_type>,
					  this, id,
					  std::ref(read_mtx), std::ref(update_mtx),
					  std::ref(line_count), std::ref(merge_count), 
					  std::ref(config), std::ref(reader),
					  log, models.at(id), model_ptr);
	      
	      if(not results.at(id).valid())
		{
		  LOG_S(ERROR) << "thread (" << id << ") is not valid after async";
		}	  
	    }
	  
	  for(std::size_t id=0; id<results.size(); id++)
	    {
	      if(not results.at(id).valid())
		{
		  LOG_S(ERROR) << "thread (" << id << ") is not valid --> skip wait!";
		  continue;
		}	  
	      
	      try
		{
		  total_text_read += results.at(id).get();
		}
	      catch(const std::exception& e)
		{
		  LOG_S(ERROR) << "error from thread (" << id << "): "
			       << e.what();
		}
	    }
	}

      log->save();
      
      LOG_S(INFO) << "total text read: " << total_text_read;
      
      LOG_S(INFO) << " final GLM:  "
		  << "#-nodes: " << std::setw(8) << nodes.size() << ", "
		  << "#-edges: " << std::setw(8) << edges.size();

      nodes.show_bucket_distribution();
      edges.show_bucket_distribution();
    }

    template<typename model_type>
    template<typename producer_type>
    std::size_t model_cli<CREATE, model_type>::update_task(std::size_t thread_id,
							   std::mutex& read_mtx,
							   std::mutex& update_mtx,
							   std::size_t& tot_line_count,
							   std::atomic<std::size_t>& merge_count,
							   nlohmann::json& config,
							   std::shared_ptr<producer_type>& reader,
							   std::shared_ptr<create_log> log,
							   std::shared_ptr<model_type> loc_model,
							   std::shared_ptr<model_type> fin_model)
    {
      std::size_t loc_line_count=0, curr_line_count=0;

      typedef typename producer_type::subject_type subject_type;
      subject_type subj;
      
      // initialise local models
      {
	loc_model->configure(config, false);
	
	loc_model->initialise((1.1*configuration.max_local_nodes),
			      (1.1*configuration.max_local_edges));
      }
      
      model_merger<model_type> merger(fin_model, configuration.enforce_max_size);
      model_creator creator(loc_model);
      
      auto& nlp_models = (loc_model->get_parameters()).models;
      producer_type nlp(nlp_models);

      if(configuration.write_nlp_output)
	{
	  if(nlp.get_subject_name()==TEXT)
	    {
	      std::filesystem::path file = "nlp-"+std::to_string(thread_id) + ".jsonl";	  
	      std::filesystem::path path = configuration.nlp_output_dir / file;
	      
	      LOG_S(WARNING) << "writing NLP to: " << path;
	      configuration.write_nlp_output = nlp.set_ofs(path);
	    }
	  else if(nlp.get_subject_name()==DOCUMENT)
	    {
	      std::filesystem::path odir = configuration.nlp_output_dir;
	      
	      LOG_S(WARNING) << "writing NLP to: " << odir;
	      configuration.write_nlp_output = nlp.set_ofs(odir);
	    }
	  else
	    {}
	}
      
      while(true)
	{
	  bool read=true;
	  {
	    std::scoped_lock lock(read_mtx);
	    read = reader->read(subj, tot_line_count);
	  }
	  
	  if(read)
	    {
	      loc_line_count += 1;
	      curr_line_count += 1;
	      
	      nlp.apply(subj);
	      
	      if(configuration.write_nlp_output)
		{
		  nlp.write(subj);
		}

	      std::set<hash_type> doc_ents={};
	      creator.update(subj, doc_ents);
	    }
	  else	    
	    {
	      //LOG_S(WARNING) << "was not able to read -> breaking loop ...";
	      break;
	    }

	  bool my_turn = ((merge_count)%(configuration.num_threads)==thread_id);
	  bool read_enough = (curr_line_count >= configuration.min_local_line_count);
	  bool forced_merge = (curr_line_count >= configuration.max_local_line_count or
			       ((loc_model->get_nodes()).size() > configuration.max_local_nodes) or
			       ((loc_model->get_edges()).size() > configuration.max_local_edges)
			       );

	  if((my_turn and read_enough) or forced_merge)
	    {
	      {
		std::scoped_lock lock(update_mtx);

		double merge_time = merger.merge(loc_model);

		log->log(thread_id, tot_line_count, curr_line_count, merge_time, fin_model);

		merge_count += 1;

		curr_line_count = 0;
	      }
	      
	      {
		loc_model->initialise(1.1*configuration.max_local_nodes,
				      1.1*configuration.max_local_edges);		
	      }
	    }
	  
	  if(forced_merge and configuration.local_reading_break)
	    {
	      LOG_S(WARNING) << "reaching maximum number of documents ("
			     << curr_line_count
			     << "): force break update loop";
	      break;
	    }
	}

      {
	std::scoped_lock lock(update_mtx);
	
	double merge_time = merger.merge(loc_model);

	log->log(thread_id, tot_line_count, curr_line_count, merge_time, fin_model);
      }
      
      loc_model->initialise();
      
      return loc_line_count;
    }

  }

}

#endif
