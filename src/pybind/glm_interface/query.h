//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_GLM_INTERFACE_QUERY_H
#define PYBIND_ANDROMEDA_GLM_INTERFACE_QUERY_H

#include "andromeda.h"

namespace andromeda_py
{
  class glm_query: public base_log,
		   //public andromeda::glm::base_types,
		   public andromeda::base_types,
                   public andromeda::glm::model_types
  {
    typedef andromeda::glm::model                      glm_model_type;
    typedef andromeda::glm::query_flow<glm_model_type> glm_flow_type;

    typedef typename glm_flow_type::flow_id_type flow_id_type;

    typedef std::shared_ptr<andromeda::glm::query_baseop> qry_baseop_ptr_type;
    
  public:

    glm_query();
    ~glm_query();

    nlohmann::json to_config();
    bool from_config(nlohmann::json config);

    void clear();

    nlohmann::json validate();
    
    flow_id_type get_last_flid() { return (flow.size()>0? flow.back()->get_flid():-1); }
    
    glm_query& select(nlohmann::json& params);

    glm_query& traverse(nlohmann::json& params);
    
    glm_query& filter_by(nlohmann::json& params);
    
    glm_query& join(nlohmann::json& params);
    glm_query& intersect(nlohmann::json& params);
    
    glm_query& subgraph(nlohmann::json& params);

  private:

    std::set<flow_id_type> get_dependencies(nlohmann::json& params);
    
  private:

    std::shared_ptr<glm_model_type> model;
    andromeda::glm::query_flow<glm_model_type> flow;
  };

  glm_query::glm_query():
    base_log(),

    model(NULL),
    flow(model)
  {}

  glm_query::~glm_query()
  {}

  nlohmann::json glm_query::to_config()
  {
    try
      {
	return flow.to_config();
      }
    catch(std::exception& exc)
      {	
	LOG_S(ERROR) << "could not create query config: "
		     << exc.what();

	nlohmann::json message = nlohmann::json::object();
	message["error"] = exc.what();

	return message;
      }
  }

  bool glm_query::from_config(nlohmann::json config)
  {
    try
      {
	flow.from_config(config);
      }
    catch(std::exception& exc)
      {
	LOG_S(ERROR) << "could not parse config: "
		     << config.dump(2);
	LOG_S(ERROR) << "error: " << exc.what();
	return false;	
      }

    return true;
  }

  void glm_query::clear()
  {
    flow.clear();
  }

  nlohmann::json glm_query::validate()
  {
    nlohmann::json result = nlohmann::json::object();
    {
      result["success"] = true;
    }

    std::string error="";
    if(not flow.validate(error))
      {
	result["success"] = false;
	result["error"] = error;
      }

    return result;
  }

  std::set<typename glm_query::flow_id_type> glm_query::get_dependencies(nlohmann::json& params)
  {
    std::set<flow_id_type> deps={};
    
    if(params.count("sources"))
      {
	deps = params["sources"].get<std::set<flow_id_type> >();
      }    
    else if(params.count("source"))
      {
	flow_id_type sid = params["source"].get<flow_id_type>();
	deps.insert(sid);
      }
    else if(flow.back()!=NULL)
      {
	flow_id_type sid = flow.back()->get_flid();
	deps.insert(sid);
      }
    else
      {}

    params["sources"] = deps;		
    
    return deps;
  }
  
  glm_query& glm_query::select(nlohmann::json& params)
  {
    flow_id_type           flid = flow.size();
    std::set<flow_id_type> deps = get_dependencies(params);
    
    qry_baseop_ptr_type op = andromeda::glm::to_flowop(model, andromeda::glm::SELECT,
						       flid, deps, params);
    
    flow.push_back(op);
    
    return *this;
  }

  glm_query& glm_query::filter_by(nlohmann::json& params)
  {
    flow_id_type           flid = flow.size();
    std::set<flow_id_type> deps = get_dependencies(params);
    
    qry_baseop_ptr_type op = andromeda::glm::to_flowop(model, andromeda::glm::FILTER,
						       flid, deps, params);
    
    flow.push_back(op);
    
    return *this;
  }
  
  glm_query& glm_query::traverse(nlohmann::json& params)
  {
    flow_id_type           flid = flow.size();
    std::set<flow_id_type> deps = get_dependencies(params);
    
    qry_baseop_ptr_type op = andromeda::glm::to_flowop(model, andromeda::glm::TRAVERSE,
						       flid, deps, params);
    flow.push_back(op);
    
    return *this;    
  }
  
  glm_query& glm_query::join(nlohmann::json& params)
  {
    flow_id_type           flid = flow.size();
    std::set<flow_id_type> deps = get_dependencies(params);
    
    qry_baseop_ptr_type op = andromeda::glm::to_flowop(model, andromeda::glm::JOIN,
						       flid, deps, params);
    flow.push_back(op);
    
    return *this;
  }

  glm_query& glm_query::intersect(nlohmann::json& params)
  {
    flow_id_type           flid = flow.size();
    std::set<flow_id_type> deps = get_dependencies(params);
    
    qry_baseop_ptr_type op = andromeda::glm::to_flowop(model, andromeda::glm::INTERSECT,
						       flid, deps, params);
    flow.push_back(op);
    
    return *this;
  }

  glm_query& glm_query::subgraph(nlohmann::json& params)
  {
    flow_id_type           flid = flow.size();
    std::set<flow_id_type> deps = get_dependencies(params);
    
    qry_baseop_ptr_type op = andromeda::glm::to_flowop(model, andromeda::glm::SUBGRAPH,
						       flid, deps, params);    
    flow.push_back(op);
    
    return *this;
  }
  
}

#endif
