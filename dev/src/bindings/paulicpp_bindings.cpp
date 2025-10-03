#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../pauliarray.hpp"

namespace py = pybind11;

PYBIND11_MODULE(paulicpp, m) {
    m.doc() = "Python bindings for the PauliArray class using Pybind11";

    m.def("tensor", &tensor, "awdwa");
    m.def("compose", &compose, "Compose two Pauli arrays");
    m.def("commute_with", &bitwise_commute_with, "Check commutation between two Pauli arrays");
    m.def("random_zx_strings", &random_zx_strings, "Gfddy");
}