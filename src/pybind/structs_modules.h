//-*-C++-*-

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind/utils/pybind11_json.h>

#include <pybind/base_log.h>
#include <pybind/base_resources.h>

#include <pybind/structs.h>
//#include <pybind/nlp_interface.h>

PYBIND11_MODULE(andromeda_structs, m) {

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
}
