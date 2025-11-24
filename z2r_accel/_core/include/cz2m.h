#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <complex>
#include <cstdint> // uint8_t
#include <cstring>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

#include "bitops.h"

#ifdef USE_OPENMP
    #include <omp.h>
#else
    #warning "OpenMP is not enabled"
#endif

#define FUNC_THRESHOLD_PARALLEL 100000

// Function declarations
py::tuple tensor(py::array z2, py::array x2, py::array z1, py::array x1);

py::tuple compose(py::array z1, py::array x1, py::array z2, py::array x2);

py::array_t<bool> bitwise_commute_with(py::array z1, py::array x1, py::array z2, py::array x2);

py::tuple random_zx_strings(const std::vector<size_t> &shape);

py::object unique(py::array zx_voids, bool return_index = false, bool return_inverse = false,
                  bool return_counts = false);

py::tuple unordered_unique(py::array zx_voids);

py::array row_echelon(py::array voids, int num_qubits);

std::tuple<std::vector<int>, std::vector<int>, std::vector<std::complex<double>>>
sparse_matrix_from_zx_voids(py::array z_voids, py::array x_voids, int num_qubits);

std::vector<std::complex<double>> get_phases(py::array z_voids, py::array x_voids);

py::array_t<std::complex<double>> to_matrix(py::array z_voids, py::array x_voids, int num_qubits);

py::array transpose(py::array voids, int64_t num_bits = -1);

py::array matmul(py::array z2r_a, py::array z2r_b, int a_num_qubits, int b_num_qubits);

py::array concatenate(py::array x1, py::array x2, int axis = 0);

py::array_t<uint8_t> z2_to_uint8(py::array z2r, int num_qubits);

py::array gauss_jordan_inverse(py::array matrix, int num_bits);