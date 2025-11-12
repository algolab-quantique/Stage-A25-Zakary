#include "../bitops.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(_bitops, m) {
    m.def("bitwise_and", &bitwise_and, "addwad", py::arg("voids_1"), py::arg("voids_2"));
    m.def("bitwise_xor", &bitwise_xor, "addwad");
    m.def("bitwise_not", &bitwise_not, "addwad");
    m.def("paded_bitwise_not", &paded_bitwise_not, "addwad", py::arg("voids"),
          py::arg("num_qubits"));
    m.def("bitwise_count", &bitwise_count, "addwad");
    m.def("bitwise_dot", &bitwise_dot, "addwad");
    m.def("bitwise_or", &bitwise_or, "addwad");
}
