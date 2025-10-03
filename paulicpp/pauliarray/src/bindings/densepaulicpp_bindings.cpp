#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../densepauliarray.hpp"

namespace py = pybind11;

PYBIND11_MODULE(densepaulicpp, m) {
    m.doc() = "Python bindings for the DensePauliArray class using Pybind11";
    py::class_<DensePauliArray>(m, "DensePauliArray")
        .def(py::init<int, int>(), py::arg("num_operators"), py::arg("num_qubits"))
        .def(py::init<uint8_t*, uint8_t*, int, int>(), 
             py::arg("z_data"), py::arg("x_data"), py::arg("num_operators"), py::arg("num_qubits"))
        .def("commutes_batch", &DensePauliArray::commutes_batch, 
             py::arg("other"), "Check commutation with another DensePauliArray")
        
             
        .def("compose_batch", &DensePauliArray::compose_batch, 
             py::arg("other"), "Compose with another DensePauliArray")
        .def("tensor", &DensePauliArray::tensor,
             py::arg("other"), "Tensor product with another DensePauliArray")
        .def_static("random", &DensePauliArray::random, 
                    py::arg("num_operators"), py::arg("num_qubits"), 
                    "Generate a random DensePauliArray")

        .def("to_string", &DensePauliArray::to_string, "String representation")
        .def("is_diagonal", &DensePauliArray::is_diagonal, "Check if the array is diagonal")
          .def("is_identity", &DensePauliArray::is_identity, "Check if the array is identity")
          .def_static("identities", &DensePauliArray::identities, 
               py::arg("num_operators"), py::arg("num_qubits"), "Generate identity DensePauliArray")
          .def("swap_zx", &DensePauliArray::swap_zx, "Swap Z and X components")
          .def("traces", &DensePauliArray::traces, "Compute the trace of the DensePauliArray");
}