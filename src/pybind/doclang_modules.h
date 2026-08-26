//-*-C++-*-

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind/utils/pybind11_json.h>

#include <pybind/doclang.h>

PYBIND11_MODULE(andromeda_doclang, m) {

  pybind11::class_<andromeda_py::DocLangXDocument>(m, "DocLangXDocument")
    .def(pybind11::init())

    .def("read", &andromeda_py::DocLangXDocument::read,
         pybind11::arg("path"))
    .def("read_xml", &andromeda_py::DocLangXDocument::read_xml,
         pybind11::arg("xml"))
    .def("write", &andromeda_py::DocLangXDocument::write,
         pybind11::arg("path"))
    .def("apply_nlp", &andromeda_py::DocLangXDocument::apply_nlp,
         pybind11::arg("models"),
         pybind11::arg("progress_every") = 25)

    .def("valid", &andromeda_py::DocLangXDocument::valid)
    .def("has_archive", &andromeda_py::DocLangXDocument::has_archive)
    .def("has_annotations", &andromeda_py::DocLangXDocument::has_annotations)

    .def("xml", &andromeda_py::DocLangXDocument::xml)
    .def("source_path", &andromeda_py::DocLangXDocument::source_path)
    .def("last_error", &andromeda_py::DocLangXDocument::last_error)

    .def("archive_paths", &andromeda_py::DocLangXDocument::archive_paths)
    .def("annotation_paths", &andromeda_py::DocLangXDocument::annotation_paths)

    .def("summary", &andromeda_py::DocLangXDocument::summary)
    .def("properties", &andromeda_py::DocLangXDocument::properties)
    .def("instances", &andromeda_py::DocLangXDocument::instances)
    .def("relations", &andromeda_py::DocLangXDocument::relations)

    .def("query_properties", &andromeda_py::DocLangXDocument::query_properties,
         pybind11::arg("type") = "",
         pybind11::arg("label") = "",
         pybind11::arg("subj_path") = "",
         pybind11::arg("min_conf") = 0.0)
    .def("query_instances", &andromeda_py::DocLangXDocument::query_instances,
         pybind11::arg("type") = "",
         pybind11::arg("subtype") = "",
         pybind11::arg("name") = "",
         pybind11::arg("name_contains") = "",
         pybind11::arg("subj_path") = "",
         pybind11::arg("min_conf") = 0.0)
    .def("query_relations", &andromeda_py::DocLangXDocument::query_relations,
         pybind11::arg("name") = "",
         pybind11::arg("name_i") = "",
         pybind11::arg("name_j") = "",
         pybind11::arg("name_contains") = "",
         pybind11::arg("min_conf") = 0.0);
}
