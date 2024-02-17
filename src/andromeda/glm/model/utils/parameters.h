//-*-C++-*-

#ifndef ANDROMEDA_MODELS_GLM_PARAMETERS_H_
#define ANDROMEDA_MODELS_GLM_PARAMETERS_H_

namespace andromeda
{
  namespace glm
  {
    class glm_parameters
    {
    public:

      const static inline std::string parameters_lbl = "parameters";

      const static inline std::string nlp_models_lbl = "nlp-models";

      const static inline std::string padding_lbl = "glm-padding";
      const static inline std::string paths_lbl = "glm-paths";

      const static inline std::string paths_concs_lbl = "keep-concatenation";     

      const static inline std::string paths_conns_lbl = "keep-connectors";
      const static inline std::string paths_terms_lbl = "keep-terms";
      const static inline std::string paths_verbs_lbl = "keep-verbs";

      const static inline std::string paths_sents_lbl = "keep-sentences";
      const static inline std::string paths_texts_lbl = "keep-texts";
      const static inline std::string paths_tabls_lbl = "keep-tables";
      const static inline std::string paths_fdocs_lbl = "keep-docs";
      
      typedef std::size_t index_type;

      typedef glm_nodes nodes_type;
      typedef glm_edges edges_type;
      
      typedef typename nodes_type::node_type node_type;
      typedef typename edges_type::edge_type edge_type;

    public:

      glm_parameters();
      glm_parameters(nlohmann::json& config, bool verbose);
      
      ~glm_parameters();

      void clear();
      
      nlohmann::json to_json();

      bool from_json(nlohmann::json& config, bool verbose);

    public:
      
      short padding;

      bool keep_concs, keep_conns, keep_verbs, keep_terms;      
      bool keep_sents, keep_texts, keep_tabls, keep_fdocs;
      
      std::vector<std::shared_ptr<andromeda::base_nlp_model> > models;
    };

    glm_parameters::glm_parameters():
      padding(1),

      keep_concs(true),
      
      keep_conns(true),
      keep_verbs(true),
      keep_terms(true),

      keep_sents(false),

      keep_texts(false),
      keep_tabls(false),
      keep_fdocs(false),
      
      models({})
    {}

    glm_parameters::glm_parameters(nlohmann::json& config, bool verbose):
      padding(1),

      keep_concs(true),
      
      keep_conns(true),
      keep_verbs(true),
      keep_terms(true),

      keep_sents(false),

      keep_texts(false),
      keep_tabls(false),
      keep_fdocs(false),      

      models({})
    {
      this->from_json(config, verbose);
    }    

    glm_parameters::~glm_parameters()
    {}

    void glm_parameters::clear()
    {}

    nlohmann::json glm_parameters::to_json()
    {
      nlohmann::json result;

      {
        result[padding_lbl] = padding;
      }

      {
	nlohmann::json& paths = result[paths_lbl];

	paths[paths_concs_lbl] = keep_concs;

	paths[paths_conns_lbl] = keep_conns;
	paths[paths_verbs_lbl] = keep_verbs;
	paths[paths_terms_lbl] = keep_terms;

	paths[paths_sents_lbl] = keep_sents;

	paths[paths_texts_lbl] = keep_texts;
	paths[paths_tabls_lbl] = keep_tabls;
	paths[paths_fdocs_lbl] = keep_fdocs;	
      }
      
      if(models.size()==0)
	{
	  std::string model_expr="conn;verb;term;abbreviation";
	  LOG_S(WARNING) << "falling back on default: " << model_expr;
	  
	  result[nlp_models_lbl] = model_expr;
	}
      else
	{
	  result[nlp_models_lbl] = from_models(models);
	}
      
      return result;
    }

    bool glm_parameters::from_json(nlohmann::json& config, bool verbose)
    {
      if(config.count(parameters_lbl))
	{
	  return this->from_json(config[parameters_lbl], verbose);
	}
      //LOG_S(INFO) << "parameters: " << config.dump(2);
      
      {
        padding = config.value(padding_lbl, padding);
      }

      {
	nlohmann::json& paths = config[paths_lbl];
	
	keep_concs = paths.value(paths_concs_lbl, keep_concs);

	keep_conns = paths.value(paths_conns_lbl, keep_conns);
	keep_verbs = paths.value(paths_verbs_lbl, keep_verbs);
	keep_terms = paths.value(paths_terms_lbl, keep_terms);

	keep_sents = paths.value(paths_sents_lbl, keep_sents);

	keep_texts = paths.value(paths_texts_lbl, keep_texts);
	keep_tabls = paths.value(paths_tabls_lbl, keep_tabls);
	keep_fdocs = paths.value(paths_fdocs_lbl, keep_fdocs);	
      }
      
      {
	std::string model_expr="conc;conn;verb;term";
	model_expr = config.value(nlp_models_lbl, model_expr);

	if(not to_models(model_expr, models, verbose))
	  {
	    LOG_S(ERROR) << "could not initialise the models with expression: "
			 << model_expr;
	    return false;
	  }
      }
      
      return true;
    }

  }

}

#endif
