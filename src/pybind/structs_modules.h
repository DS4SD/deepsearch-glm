//-*-C++-*-

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind/utils/pybind11_json.h>

#include <pybind/base_log.h>
#include <pybind/base_resources.h>

#include <pybind/structs.h>

PYBIND11_MODULE(andromeda_structs, m) {

  pybind11::class_<andromeda_py::ds_text>(m, "ds_text")
    .def(pybind11::init())
    .def("to_json", &andromeda_py::ds_text::to_json)
    .def("from_json", &andromeda_py::ds_text::from_json)
    .def("clear", &andromeda_py::ds_text::clear)
    .def("set_text", &andromeda_py::ds_text::set_text);

  pybind11::class_<andromeda_py::ds_table>(m, "ds_table")
    .def(pybind11::init())
    .def("to_json", &andromeda_py::ds_table::to_json)
    .def("from_json", &andromeda_py::ds_table::from_json)
    .def("clear", &andromeda_py::ds_table::clear)
    .def("set_data", &andromeda_py::ds_table::set_data);
  
  pybind11::class_<andromeda_py::ds_document>(m, "ds_document")
    .def(pybind11::init())
    .def("to_json", &andromeda_py::ds_document::to_json)
    .def("from_json", &andromeda_py::ds_document::from_json)

    .def("clear", &andromeda_py::ds_document::clear)

    .def("set_title", &andromeda_py::ds_document::set_title)
    .def("set_date", &andromeda_py::ds_document::set_date)
    .def("set_abstract", &andromeda_py::ds_document::set_abstract)
    .def("set_authors", &andromeda_py::ds_document::set_authors)
    .def("set_affiliations", &andromeda_py::ds_document::set_affiliations)
    .def("set_advanced", &andromeda_py::ds_document::set_advanced)

    .def("append_text", &andromeda_py::ds_document::append_text)
    .def("append_table", &andromeda_py::ds_document::append_table);
}
