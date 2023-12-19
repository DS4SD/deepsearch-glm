//-*-C++-*-

#include <pybind11/pybind11.h>
#include <pybind/utils/pybind11_json.h>

#include <pybind/base_log.h>
#include <pybind/base_resources.h>
#include <pybind/nlp_interface.h>

PYBIND11_MODULE(andromeda_nlp, m) {
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
    .def("apply_on_doc", &andromeda_py::nlp_model::apply_on_doc);
}

