//-*-C++-*-

#ifndef ANDROMEDA_MODELS_RECORDS_METADATA_H_
#define ANDROMEDA_MODELS_RECORDS_METADATA_H_

namespace andromeda
{

  template<>
  class nlp_model<REC, METADATA>:
    public fasttext_supervised_model//,
  //public base_crf_model
  {
    typedef typename word_token::range_type range_type;

  public:

    nlp_model();
    ~nlp_model();

    virtual std::set<model_name> get_dependencies() { return dependencies; }

    virtual model_type get_type() { return ENT; }
    virtual model_name get_name() { return METADATA; }

    // you can only run metadata on full documents
    virtual bool apply(subject<TEXT>& subj) { return false; }
    virtual bool apply(subject<TABLE>& subj) { return false; }

    virtual bool apply(subject<DOCUMENT>& subj);

  private:

    bool initialise();

    bool is_scientific_paper(subject<DOCUMENT>& subj);

    bool find_title(subject<DOCUMENT>& subj);

    bool find_authors(subject<DOCUMENT>& subj);

    bool find_affiliations(subject<DOCUMENT>& subj);

    bool find_abstract(subject<DOCUMENT>& subj);

  private:

    const static std::set<model_name> dependencies;

    std::filesystem::path model_file;

    int title_ind, abstract_ind, introduction_ind;

    std::set<int> metadata_inds;
    
    std::string title;
    std::vector<std::string> abstract;

    std::vector<std::string> authors;
    std::vector<std::string> affiliations;
  };

  const std::set<model_name> nlp_model<REC, METADATA>::dependencies = {NAME/*, DATE*/, GEOLOC, SEMANTIC};

  nlp_model<REC, METADATA>::nlp_model():
    fasttext_supervised_model(),
    model_file(get_fst_dir() / "metadata/fst_author.bin"),

    title_ind(-1),
    abstract_ind(-1),
    introduction_ind(-1),

    metadata_inds({})
  {
    initialise();
  }

  nlp_model<REC, METADATA>::~nlp_model()
  {}

  bool nlp_model<REC, METADATA>::initialise()
  {
    if(not fasttext_supervised_model::load(model_file))
      {
        LOG_S(ERROR) << "could not load `authors` classifier model for `metadata` ...";
        return false;
      }

    return true;
  }

  bool nlp_model<REC, METADATA>::apply(subject<DOCUMENT>& subj)
  {
    if(not is_scientific_paper(subj))
      {
        LOG_S(INFO) << "document is NOT scientific report!";
        return false;
      }

    LOG_S(INFO) << "document is scientific report!";

    find_title(subj);

    find_authors(subj);

    find_affiliations(subj);

    find_abstract(subj);

    return update_applied_models(subj);
  }

  bool nlp_model<REC, METADATA>::is_scientific_paper(subject<DOCUMENT>& subj)
  {
    bool detected_abstract=false;
    bool detected_introduction=false;

    for(int tind=0; tind<subj.texts.size(); tind++)
      {
        auto& tsubj = subj.texts.at(tind);

        auto& provs = tsubj->get_provs();
        if(provs.size()>0 and provs.at(0)->get_page()>=2)
          {
            break;
          }

        std::string text = tsubj->get_text();

        text = utils::replace(text, " ", "");
        text = utils::to_lower(text);

        if(text.starts_with("abstract") or
           text.ends_with("abstract"))
          {
            abstract_ind = tind;
            detected_abstract=true;
          }

        if(text.starts_with("introduction") or
           text.ends_with("introduction"))
          {
            introduction_ind = tind;
            detected_introduction=true;
          }
      }

    //LOG_S(INFO) << abstract_ind << ", " << introduction_ind << "\n";

    return (detected_abstract or detected_introduction);
  }

  bool nlp_model<REC, METADATA>::find_title(subject<DOCUMENT>& subj)
  {
    // We first see if we have detected a title bounding-box on the first two pages ...

    std::string iref="";
    for(int pind=0; pind<subj.provs.size(); pind++)
      {
        auto& prov = subj.provs.at(pind);

        int         page = prov->get_page();
        std::string type = prov->get_type();

        if(page<=2 and type=="title")
          {
            iref = prov->get_item_ref();
            //LOG_S(INFO) << "found title at " << iref;
          }
      }

    title_ind=-1;

    // currently, the title-box is not found with great enough accuracy
    // to rely on ML to distinguish title from header ...
    
    /*
    if(iref!="") // found bbox with type `title`
      {
	//LOG_S(WARNING) << "found bbox with type `title`";
	
	for(int tind=0; tind<subj.texts.size(); tind++)
	  {
	    if(subj.texts.at(tind)->get_self_ref()==iref)
	      {
		title_ind = tind;
		break;
	      }
	  }
      }
    */

    if(title_ind==-1) // fall-back: assume title is before first metadata
      {
        int cut_off=0;
        if(abstract_ind!=-1)
          {
            cut_off = abstract_ind;
          }
        else if(introduction_ind!=-1)
          {
            cut_off = introduction_ind;
          }
        else
          {}
	
        for(int tind=1; tind<cut_off; tind++)
          {
            std::vector<std::string> semlabels = subj.texts.at(tind)->get_property_labels(SEMANTIC);
            
	    if(std::find(semlabels.begin(), semlabels.end(), "meta-data")!=semlabels.end())
              {
		metadata_inds.insert(tind);

		if(title_ind==-1)
		  {
		    title_ind = tind-1;
		  }
              }
          }
      }
    
    if(0<=title_ind and title_ind<subj.texts.size())
      {
        auto& title_subj = subj.texts.at(title_ind);
        title = title_subj->get_text();

        subj.properties.emplace_back(title_subj->get_hash(), DOCUMENT, title_subj->get_self_ref(),
                                     get_name(), "title", 1.0);
	
	subj.set_title(title);
      }

    return true;
  }

  bool nlp_model<REC, METADATA>::find_authors(subject<DOCUMENT>& subj)
  {
    //LOG_S(INFO) << __FUNCTION__;

    authors = {};
    
    int cut_off=0;
    if(abstract_ind!=-1)
      {
        cut_off = abstract_ind;
      }
    else if(introduction_ind!=-1)
      {
        cut_off = introduction_ind;
      }
    else
      {}

    for(int tind=title_ind+1; tind<cut_off; tind++)
      {
        auto& tsubj = subj.texts.at(tind);
        //LOG_S(INFO) << tind << "\t" << tsubj->get_text();

        auto& insts = tsubj->get_instances();

        for(auto& inst:insts)
          {
            //LOG_S(INFO) << inst.get_subtype() << "\t" << inst.get_name() << "\t";
            if(inst.is_model(NAME))// and inst.is_subtype("person-name"))
              {
                std::string name = inst.get_name();
                std::string orig = inst.get_orig();

                //LOG_S(INFO) << " --> author: " << name;
		std::string label="";
		double conf = 0.0;

		bool success = this->classify(name, label, conf);
		
		if(success and label=="person-name")
		  {
		    authors.push_back(name);
		    
		    subj.instances.emplace_back(subj.get_hash(), DOCUMENT, tsubj->get_self_ref(), inst.get_conf(),
						METADATA, "author",
						name, orig,
						inst.get_char_range(),
						inst.get_ctok_range(),
						inst.get_wtok_range());
		  }
		
		//LOG_S(INFO) << success << " name: " << name << " => " << label << " (" << conf << ")";
	      }
          }
      }
    
    subj.set_authors(authors);
    
    return true;
  }

  bool nlp_model<REC, METADATA>::find_affiliations(subject<DOCUMENT>& subj)
  {
    affiliations = {};
    
    int cut_off=0;
    if(abstract_ind!=-1)
      {
        cut_off = abstract_ind;
      }
    else if(introduction_ind!=-1)
      {
        cut_off = introduction_ind;
      }
    else
      {}

    //LOG_S(INFO) << title_ind << "\t" << cut_off;

    for(int tind=title_ind+1; tind<cut_off; tind++)
      {
        auto& tsubj = subj.texts.at(tind);
        //LOG_S(INFO) << tind << "\t" << tsubj->get_text();

        auto& insts = tsubj->get_instances();

        for(auto& inst:insts)
          {
            //LOG_S(INFO) << inst.get_subtype() << "\t" << inst.get_name() << "\t";
            if(inst.is_model(NAME) and inst.is_subtype("specialised-name"))
              {
                std::string name = inst.get_name();
                std::string orig = inst.get_orig();

                //LOG_S(INFO) << " --> author: " << name;

		affiliations.push_back(name);
		
                subj.instances.emplace_back(subj.get_hash(), DOCUMENT, tsubj->get_self_ref(), inst.get_conf(),
                                            METADATA, "affiliation",
                                            name, orig,
                                            inst.get_char_range(),
                                            inst.get_ctok_range(),
                                            inst.get_wtok_range());
              }
          }
      }

    subj.set_affiliations(affiliations);
    
    return true;
  }
  
  bool nlp_model<REC, METADATA>::find_abstract(subject<DOCUMENT>& subj)
  {
    abstract={};
    
    if(abstract_ind!=-1 and introduction_ind==-1) // only found abstract header
      {
        auto& abstract_subj = subj.texts.at(abstract_ind);
	abstract.push_back(abstract_subj->get_text());

	auto tmp = utils::to_lower(abstract.back());
	tmp = utils::strip(tmp);

	if(tmp.ends_with("abstract"))
	  {
	    if(abstract_ind+1<subj.texts.size())
	      {
		subj.properties.emplace_back(abstract_subj->get_hash(), DOCUMENT, abstract_subj->get_self_ref(),
					     get_name(), "abstract", 1.0);

		auto& abstract_subj = subj.texts.at(abstract_ind+1);
		abstract.push_back(abstract_subj->get_text());
	      }
	  }
	else
	  {
	    subj.properties.emplace_back(abstract_subj->get_hash(), DOCUMENT, abstract_subj->get_self_ref(),
					 get_name(), "abstract", 1.0);
	  }
      }
    else if(abstract_ind!=-1 and introduction_ind!=-1) // found abstract and introduction header
      {
        for(int tind=abstract_ind; tind<introduction_ind; tind++)
          {
            auto& abstract_subj = subj.texts.at(tind);
	    //abstract += " " + abstract_subj->get_text();
	    abstract.push_back(abstract_subj->get_text());
	    
            subj.properties.emplace_back(abstract_subj->get_hash(), DOCUMENT, abstract_subj->get_self_ref(),
                                         get_name(), "abstract", 1.0);
          }
      }
    else if(abstract_ind==-1 and introduction_ind!=-1) // only found introduction header
      {
	
      }
    else
      {}

    //abstract = utils::strip(abstract);
    
    //if(abstract.size()>0)
    //{
    subj.set_abstract(abstract);
    //}
    
    return true;
  }

}

#endif
