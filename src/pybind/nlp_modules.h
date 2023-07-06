//-*-C++-*-

#include <pybind11/pybind11.h>
#include <pybind/utils/pybind11_json.h>

#include <pybind/base_log.h>
#include <pybind/nlp_interface.h>

PYBIND11_MODULE(andromeda_nlp, m) {
  pybind11::class_<andromeda_py::nlp_model>(m, "nlp_model")
    .def(pybind11::init())
    .def("initialise", &andromeda_py::nlp_model::initialise)

    .def("get_apply_configs", &andromeda_py::nlp_model::get_apply_configs)
    .def("get_train_configs", &andromeda_py::nlp_model::get_train_configs)
    
    .def("apply", &andromeda_py::nlp_model::apply)
    .def("train", &andromeda_py::nlp_model::train)
    
    .def("apply_on_text", &andromeda_py::nlp_model::apply_on_text)
    .def("apply_on_doc", &andromeda_py::nlp_model::apply_on_doc);
}

