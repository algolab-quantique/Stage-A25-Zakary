#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../src/voidops.hpp"

namespace py = pybind11;

PYBIND11_MODULE(voidops, m) {
    m.doc() = "Module for void operations on numpy arrays using pybind11";

    m.def("bitwise_and", &bitwise_and, "addwad");
    m.def("bitwise_xor", &bitwise_xor, "addwad");
    m.def("bitwise_not", &bitwise_not, "addwad");
    m.def("bitwise_count", &bitwise_count, "addwad");
    m.def("bitwise_dot", &bitwise_dot, "addwad");
}
    