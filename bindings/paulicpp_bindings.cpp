#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../src/pauliarray.hpp"

namespace py = pybind11;

PYBIND11_MODULE(paulicpp, m) {
    m.doc() = "Python bindings for the PauliArray class using Pybind11";

    py::class_<PauliArray>(m, "PauliArray")
        .def(py::init<>()) 
        .def(py::init<std::vector<pauli_t>>())  
        .def(py::init<std::vector<pauli_t>, int>()) 
        .def(py::init<std::vector<uint8_t>, std::vector<uint8_t>, std::complex<double>>()) 
        .def("zx_to_paulis", &PauliArray::zx_to_paulis, "Convert ZX strings to Pauli representation")
        .def("commutes", &PauliArray::commutes, "Check commutation with another PauliArray")
        .def("commutes_numpy", &PauliArray::commutes_numpy, "Check commutation with another PauliArray and return a Numpy array")
        .def("size", &PauliArray::size, "Get the size of the PauliArray")
        .def("get_x_string", &PauliArray::get_x_string, "Get the X string")
        .def("get_z_string", &PauliArray::get_z_string, "Get the Z string")
        .def("get_paulis", &PauliArray::get_paulis, "Get the Pauli representation")
        .def("compose", &PauliArray::compose, "Compose with another PauliArray")
        .def("tensor", &PauliArray::tensor, "Tensor product with another PauliArray")
        .def_static("random", &PauliArray::random, "Generate a random PauliArray")
        .def("concatenate", &PauliArray::concatenate, "Concatenate with another PauliArray")
        .def("__repr__", [](const PauliArray &pa) {
            std::ostringstream oss;
            oss << pa;
            return oss.str();
        });
}