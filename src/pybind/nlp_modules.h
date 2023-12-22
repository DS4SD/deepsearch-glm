//-*-C++-*-

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind/utils/pybind11_json.h>

#include <pybind/base_log.h>
#include <pybind/base_resources.h>

#include <pybind/nlp_structs.h>
#include <pybind/nlp_interface.h>

PYBIND11_MODULE(andromeda_nlp, m) {

  pybind11::class_<andromeda_py::nlp_text>(m, "nlp_text")
    .def(pybind11::init())
    .def("to_json", &andromeda_py::nlp_text::to_json)
    .def("from_json", &andromeda_py::nlp_text::from_json)
    .def("clear", &andromeda_py::nlp_text::clear)
    .def("set_text", &andromeda_py::nlp_text::set_text);

  pybind11::class_<andromeda_py::nlp_table>(m, "nlp_table")
    .def(pybind11::init())
    .def("to_json", &andromeda_py::nlp_table::to_json)
    .def("from_json", &andromeda_py::nlp_table::from_json)
    .def("clear", &andromeda_py::nlp_table::clear);
  
  pybind11::class_<andromeda_py::nlp_document>(m, "nlp_document")
    .def(pybind11::init())
    .def("to_json", &andromeda_py::nlp_document::to_json)
    .def("from_json", &andromeda_py::nlp_document::from_json)
    .def("clear", &andromeda_py::nlp_document::clear)
    .def("append_text", &andromeda_py::nlp_document::append_text);
  
  pybind11::class_<andromeda_py::nlp_model>(m, "nlp_model")
    .def(pybind11::init())
    .def("get_resources_path", &andromeda_py::nlp_model::get_resources_path)

    .def("initialise", &andromeda_py::nlp_model::initialise)
    .def("initialise_models", &andromeda_py::nlp_model::initialise_models)

    .def("get_apply_configs", &andromeda_py::nlp_model::get_apply_configs)
    .def("get_train_configs", &andromeda_py::nlp_model::get_train_configs)
    
    .def("prepare_data_for_train", &andromeda_py::nlp_model::prepare_data_for_train)
    .def("apply", &andromeda_py::nlp_model::apply)
    .def("train", &andromeda_py::nlp_model::train)
    .def("evaluate", &andromeda_py::nlp_model::evaluate)
    
    .def("apply_on_text", &andromeda_py::nlp_model::apply_on_text)
    .def("apply_on_doc", &andromeda_py::nlp_model::apply_on_doc)

    .def("apply_on_subj", &andromeda_py::nlp_model::apply_on_subj);
}

