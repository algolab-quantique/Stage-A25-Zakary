#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;


#include <cstdint> // uint8_t
#include <iostream>
#include <cstring>
#include <omp.h>

#include "voidops.hpp"

using namespace std;

#define I 0  // 0b00
#define X 1  // 0b01  
#define Z 2  // 0b10
#define Y 3  // 0b11


py::tuple tensor(py::array z1, py::array x1, py::array z2, py::array x2) {
    // Check dimensions
    if (z1.ndim() != z2.ndim()) throw std::runtime_error("z1 and z2 must have same ndim");
    if (x1.ndim() != x2.ndim()) throw std::runtime_error("x1 and x2 must have same ndim");
    for (ssize_t i = 0; i < z1.ndim() - 1; ++i) {
        if (z1.shape(i) != z2.shape(i)) throw std::runtime_error("Shapes must match except last axis");
        if (x1.shape(i) != x2.shape(i)) throw std::runtime_error("Shapes must match except last axis");
    }

    // Build new shape
    std::vector<ssize_t> new_shape(z1.ndim());
    for (ssize_t i = 0; i < z1.ndim() - 1; ++i) new_shape[i] = z1.shape(i);
    new_shape[z1.ndim() - 1] = z1.shape(z1.ndim() - 1) + z2.shape(z2.ndim() - 1);

    // Allocate output arrays
    py::array new_z(z1.dtype(), new_shape);
    py::array new_x(x1.dtype(), new_shape);

    // Copy data (using numpy for simplicity)
    py::object np = py::module_::import("numpy");
    py::object concat_z = np.attr("concatenate")(py::make_tuple(z1, z2), z1.ndim() - 1);
    py::object concat_x = np.attr("concatenate")(py::make_tuple(x1, x2), x1.ndim() - 1);

    // Convert to py::array
    new_z = concat_z.cast<py::array>();
    new_x = concat_x.cast<py::array>();

    return py::make_tuple(new_z, new_x);
}

py::tuple compose(py::array z1, py::array x1, py::array z2, py::array x2){
    auto buf_z1 = z1.request();
    auto buf_x1 = x1.request();
    auto buf_z2 = z2.request();
    auto buf_x2 = x2.request();

    auto z1_ptr = static_cast<uint64_t *>(buf_z1.ptr);
    auto x1_ptr = static_cast<uint64_t *>(buf_x1.ptr);
    auto z2_ptr = static_cast<uint64_t *>(buf_z2.ptr);
    auto x2_ptr = static_cast<uint64_t *>(buf_x2.ptr);

    py::array new_z = bitwise_xor(z1, z2); 
    py::array new_x = bitwise_xor(x1, x2);

    py::array self_phase_power = bitwise_dot(z1, x1);
    py::array other_phase_power = bitwise_dot(z2, x2);
    py::array new_phase_power = bitwise_dot(new_z, new_x);
    py::array commutation_phase_power = bitwise_dot(x1, z2);

    // phase_power = commutation_phase_power + self_phase_power + other_phase_power - new_phase_power
    py::array phase_power = py::array(self_phase_power.dtype(), self_phase_power.shape());
    auto phase_power_ptr = static_cast<uint64_t *>(phase_power.request().ptr);
    auto comm_ptr = static_cast<uint64_t *>(commutation_phase_power.request().ptr);
    auto self_ptr = static_cast<uint64_t *>(self_phase_power.request().ptr);
    auto other_ptr = static_cast<uint64_t *>(other_phase_power.request().ptr);
    auto new_ptr = static_cast<uint64_t *>(new_phase_power.request().ptr);

    size_t n = self_phase_power.size();
    #pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
        phase_power_ptr[i] = 2*comm_ptr[i] + self_ptr[i] + other_ptr[i] - new_ptr[i];
    }

    return py::make_tuple(new_z, new_x, phase_power);
}
