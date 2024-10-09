//-*-C++-*-

#ifndef PYBIND_ANDROMEDA_BASE_RESOURCES_H
#define PYBIND_ANDROMEDA_BASE_RESOURCES_H

#include <Python.h>

namespace andromeda_py
{
  
  class base_resources
  {
  public:

    base_resources();

    std::string get_resources_path();

  private:

    bool set_resources_path();
  };
  
  base_resources::base_resources()
  {
    set_resources_path();
  }
  
  std::string base_resources::get_resources_path()
  {
    //return andromeda::glm_variables::get_resources_dir(true);
    return andromeda::glm_variables::get_resources_dir(true).string();
  }
  
  bool base_resources::set_resources_path()
  {
    //LOG_S(INFO) << __FUNCTION__;

    /*
    auto default_path = andromeda::glm_variables::get_resources_dir(false);
    if(std::filesystem::exists(default_path))
      {
	return true;
      }
    */
    
    // Get the module object of your package
    PyObject* myPackageModule = PyImport_ImportModule("deepsearch_glm");
    
    // Get the filename object of the module
    PyObject* filenameObj = PyModule_GetFilenameObject(myPackageModule);
    
    // Extract the string value of the filename
    const char* filename = PyUnicode_AsUTF8(filenameObj);
	
    std::filesystem::path __init__path(filename);

    std::filesystem::path package_path = __init__path.parent_path();
    //std::filesystem::path resources_path = package_path / "resources";
    std::filesystem::path resources_path = package_path / andromeda::glm_variables::resources_relative_path;

    return andromeda::glm_variables::set_resources_dir(resources_path);
  }
  
}

#endif
