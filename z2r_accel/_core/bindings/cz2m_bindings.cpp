#include "cz2m.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
// declare registration function from voidops_bindings.cpp
// void register_bitops(pybind11::module_ &m);

namespace py = pybind11;

PYBIND11_MODULE(_cz2m, m) {
    m.doc() = "Python bindings for the PauliArray class using Pybind11";

    m.def("tensor", &tensor, "awdwa");
    m.def("compose", &compose, "Compose two Pauli arrays");
    m.def("bitwise_commute_with", &bitwise_commute_with,
          "Check commutation between two Pauli arrays");
    m.def("random_zx_strings", &random_zx_strings, "Gfddy");
    m.def("unique", &unique, "Unique arrays 1", py::arg("zx_voids"),
          py::arg("return_index") = false, py::arg("return_inverse") = false,
          py::arg("return_counts") = false);
    m.def("unordered_unique", &unordered_unique,
          "Returns unordered unique rows of the input array");
    m.def("to_matrix", &to_matrix, "addwad");
    m.def("transpose", &transpose, "addwad");
    m.def("matmul", &matmul, "addwad");
    m.def("row_echelon", &row_echelon, "addwad");
    m.def("concatenate", &concatenate, "addwad");
    m.def("z2_to_uint8", &z2_to_uint8, "Convert z2r array to uint8 representation", py::arg("z2r"),
          py::arg("num_qubits"));
    m.def("gauss_jordan_inverse", &gauss_jordan_inverse,
          "Compute the Gauss-Jordan inverse of a binary matrix", py::arg("matrix"),
          py::arg("num_qubits"));

    // register_bitops(m);
}