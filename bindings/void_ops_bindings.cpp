#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../src/densepauliarray.hpp"

namespace py = pybind11;

PYBIND11_MODULE(void_ops, m) {
    