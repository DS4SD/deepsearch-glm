//-*-C++-*-

#ifndef ANDROMEDA_MODELS_ENTITIES_UCMI_MATERIAL_H_
#define ANDROMEDA_MODELS_ENTITIES_UCMI_MATERIAL_H_

namespace andromeda
{
  template<>
  class nlp_model<ENT, MATERIAL>: public nlp_model<ENT, CUSTOM_CRF>
  {
  public:

    nlp_model();
    //~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return MATERIAL; }

    virtual std::string get_key() { return to_key(MATERIAL); }
    
    //virtual bool apply(std::string& text, nlohmann::json& annots);

    virtual bool apply(subject<TEXT>& subj);
    //virtual bool apply_on_table_data(subject<TABLE>& subj);

  private:

    //bool initialise();
    
  protected:

    const static inline std::set<model_name> dependencies = {};
  };

  nlp_model<ENT, MATERIAL>::nlp_model():
    nlp_model<ENT, CUSTOM_CRF>()
  {
    nlp_model<ENT, CUSTOM_CRF>::custom_name = "material";
    nlp_model<ENT, CUSTOM_CRF>::custom_file = "material.bin";

    nlp_model<ENT, CUSTOM_CRF>::model_file = get_crf_dir() / "ucmi/crf_material.bin";
    
    nlp_model<ENT, CUSTOM_CRF>::initialise();
  }

  bool nlp_model<ENT, MATERIAL>::apply(subject<TEXT>& subj)
  {
    if(not satisfies_dependencies(subj))
      {
        return false;
      }

    run_model(subj);

    post_process_bio(subj);

    // final filter ...
    {
      std::set<std::string> filters = {"...", "---"};
      
      auto& instances = subj.get_instances();
      for(auto itr=instances.begin(); itr!=instances.end(); )
	{
	  if(itr->is_model(MATERIAL))
	    {
	      std::string name = itr->get_name();

	      bool keep=true;
	      for(std::string filter:filters)
		{
		  if(utils::contains(name, filter))
		    {
		      keep=false;
		    }
		}

	      if(not keep)
		{
		  itr = instances.erase(itr);
		}
	      else
		{
		  itr++;
		}
	    }
	  else
	    {
	      itr++;
	    }
	}
    }
    
    return true;
  }

}

#endif
