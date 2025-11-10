#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../setsparse.hpp"

namespace py = pybind11;

void register_setsparse(py::module_ &m) {
    m.def("make_dpoint_set", &make_dpoint_set, "Create dpoint set from binary vector");
    m.def("xor_set", &xor_set, "XOR operation on dpoint sets");
}