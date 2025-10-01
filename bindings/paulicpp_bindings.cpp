#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../src/pauliarray.hpp"

namespace py = pybind11;

PYBIND11_MODULE(paulicpp, m) {
    m.doc() = "Python bindings for the PauliArray class using Pybind11";

    m.def("tensor", &tensor, "awdwa");
    m.def("compose", &compose, "Compose two Pauli arrays");
}