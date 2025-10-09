#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../sparsepaulicpp.hpp"

namespace py = pybind11;

PYBIND11_MODULE(sparsepaulicpp, m) {
    m.doc() = "Python bindings for the SparsePauliArray class using Pybind11";
    m.def("generate_random_vec", &generate_random_vec, "awdwadwadwddd");
    m.def("make_dpoint", &make_dpoint, "awdwadwadwddd");
    m.def("show_dpoints", &show_dpoints, "awdwadwadwddd");
    m.def("xor_dpoint", &xor_dpoint, "awdwadwadwddd");
    m.def("and_dpoint", &and_dpoint, "awdwadwadwddd");
    m.def("or_dpoint", &or_dpoint, "awdwadwadwddd");
    m.def("not_dpoint", &not_dpoint, "awdwadwadwddd");
    m.def("overlap_dpoint", &overlap_dpoint, "awdwadwadwddd");
    m.def("count_dpoint", &count_dpoint, "awdwadwadwddd");
}