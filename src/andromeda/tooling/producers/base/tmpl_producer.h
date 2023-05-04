//-*-C++-*-

#ifndef ANDROMEDA_PRODUCERS_TMPL_PRODUCERS_H_
#define ANDROMEDA_PRODUCERS_TMPL_PRODUCERS_H_

namespace andromeda
{

  template<subject_name name>
  class producer: public base_producer
  {
  public:

    producer();
    producer(std::vector<model_ptr_type> models);
    producer(nlohmann::json config, std::vector<model_ptr_type> models);
  };

  template<subject_name name>
  producer<name>::producer():
    base_producer()
  {}

  template<subject_name name>
  producer<name>::producer(std::vector<model_ptr_type> models):
    base_producer(models)
  {}

  template<subject_name name>
  producer<name>::producer(nlohmann::json config, std::vector<model_ptr_type> models):    
    base_producer(config, models)
  {}

}

#endif
