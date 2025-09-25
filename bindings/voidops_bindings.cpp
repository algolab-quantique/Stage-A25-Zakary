#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../src/voidops.hpp"

namespace py = pybind11;

PYBIND11_MODULE(voidops, m) {
    m.doc() = "Module for void operations on numpy arrays using pybind11";

    m.def("bitwise_and", &bitwise_and, "jhgfd");
    m.def("bitwise_count", &bitwise_count, "Count the number of set bits in a uint64 numpy array");
    m.def("bitwise_dot", &bitwise_dot, "dwadwa");
    m.def("bitwise_not", &bitwise_not, "dwadwa");
    m.def("bitwise_xor", &bitwise_xor, "dwadwa");
}
    