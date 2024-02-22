//-*-C++-*-

#include <pybind11/pybind11.h>
#include <pybind/utils/pybind11_json.h>

#include <pybind/glm_interface.h>

PYBIND11_MODULE(andromeda_glm, m) {
  pybind11::class_<andromeda_py::glm_model>(m, "glm_model")
    .def(pybind11::init())

    .def("set_loglevel", &andromeda_py::glm_model::set_loglevel)
    .def("get_resources_path", &andromeda_py::glm_model::get_resources_path)
    
    .def("from_dir", &andromeda_py::glm_model::load_dir)
    .def("to_dir", &andromeda_py::glm_model::save_dir)

    .def("load", &andromeda_py::glm_model::load)
    .def("save", &andromeda_py::glm_model::save)

    .def("get_topology", &andromeda_py::glm_model::get_topology)
    .def("get_configurations", &andromeda_py::glm_model::get_configurations)
    
    .def("create", &andromeda_py::glm_model::create)
    .def("distill", &andromeda_py::glm_model::distill)
    .def("explore", &andromeda_py::glm_model::explore)
    .def("query", &andromeda_py::glm_model::query)

    .def("apply_on_text", &andromeda_py::glm_model::apply_on_text);

  pybind11::class_<andromeda_py::glm_query>(m, "glm_query")
    .def(pybind11::init())

    .def("from_config", &andromeda_py::glm_query::from_config)
    .def("to_config", &andromeda_py::glm_query::to_config)

    .def("clear", &andromeda_py::glm_query::clear)
    .def("get_last_flid", &andromeda_py::glm_query::get_last_flid)
    
    .def("select", &andromeda_py::glm_query::select)
    .def("traverse", &andromeda_py::glm_query::traverse)
    .def("filter_by", &andromeda_py::glm_query::filter_by)

    .def("join", &andromeda_py::glm_query::join)
    .def("intersect", &andromeda_py::glm_query::intersect)

    .def("subgraph", &andromeda_py::glm_query::subgraph);
}
