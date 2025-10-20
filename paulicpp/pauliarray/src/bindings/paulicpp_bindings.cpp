#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../paulicpp.hpp"
// declare registration function from voidops_bindings.cpp
void register_voidops(pybind11::module_ &m);

namespace py = pybind11;

PYBIND11_MODULE(paulicpp, m) {
    m.doc() = "Python bindings for the PauliArray class using Pybind11";

    m.def("tensor", &tensor, "awdwa");
    m.def("compose", &compose, "Compose two Pauli arrays");
    m.def("bitwise_commute_with", &bitwise_commute_with, "Check commutation between two Pauli arrays");
    m.def("random_zx_strings", &random_zx_strings, "Gfddy");
    m.def("unique", &unique, "Unique arrays 1", 
        py::arg("zx_voids"), py::arg("return_index") = false, py::arg("return_inverse") = false, py::arg("return_counts") = false);
    m.def("unordered_unique", &unordered_unique, "Returns unordered unique rows of the input array");

    // register voidops bindings into this module
    register_voidops(m);
}