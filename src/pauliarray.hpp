#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;


#include <cstdint> // uint8_t
#include <iostream>
#include <cstring>
#include <omp.h>
#include <vector>
#include <random>

#include "voidops.hpp"


#define I 0  // 0b00
#define X 1  // 0b01  
#define Z 2  // 0b10
#define Y 3  // 0b11

static const std::complex<double> phase_lookup[4] = {
    {1.0, 0.0},      // 0 -> 1
    {0.0, -1.0},     // 1 -> -1j
    {-1.0, 0.0},     // 2 -> -1
    {0.0, 1.0}       // 3 -> 1j
};

py::tuple tensor(py::array z1, py::array x1, py::array z2, py::array x2) {
    // Request buffer info for input arrays
    auto buf_z1 = z1.request();
    auto buf_x1 = x1.request();
    auto buf_z2 = z2.request();
    auto buf_x2 = x2.request();

    // Get shapes of input arrays
    std::vector<size_t> shape1(buf_z1.shape.begin(), buf_z1.shape.end());
    std::vector<size_t> shape2(buf_z2.shape.begin(), buf_z2.shape.end());

    // Ensure the shapes are compatible for concatenation
    if (shape1.size() != shape2.size()) {
        throw std::invalid_argument("Input arrays must have the same number of dimensions for concatenation.");
    }
    for (size_t i = 0; i < shape1.size() - 1; ++i) {
        if (shape1[i] != shape2[i]) {
            throw std::invalid_argument("Input arrays must have matching dimensions (except the last one).");
        }
    }

    // Compute the concatenated shape
    std::vector<size_t> combined_shape = shape1;
    combined_shape.back() += shape2.back(); // Add the last dimension sizes

    // Create output arrays for concatenated z and x
    py::array new_z(z1.dtype(), combined_shape);
    py::array new_x(z1.dtype(), combined_shape);

    auto buf_new_z = new_z.request();
    auto buf_new_x = new_x.request();

    void* ptr_new_z = buf_new_z.ptr;
    void* ptr_new_x = buf_new_x.ptr;

    const void* ptr_z1 = buf_z1.ptr;
    const void* ptr_x1 = buf_x1.ptr;
    const void* ptr_z2 = buf_z2.ptr;
    const void* ptr_x2 = buf_x2.ptr;

    // Sizes of the last dimension
    size_t last_dim1 = shape1.back();
    size_t last_dim2 = shape2.back();
    size_t num_elements = buf_z1.size / last_dim1; // Number of elements excluding the last dimension

    // Concatenate z and x arrays
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < num_elements; ++i) {
        size_t offset1 = i * last_dim1;
        size_t offset2 = i * last_dim2;
        size_t offset_new = i * (last_dim1 + last_dim2);

        // Copy z1 and z2
        std::memcpy(static_cast<char*>(ptr_new_z) + offset_new * buf_z1.itemsize,
                    static_cast<const char*>(ptr_z1) + offset1 * buf_z1.itemsize,
                    last_dim1 * buf_z1.itemsize);
        std::memcpy(static_cast<char*>(ptr_new_z) + (offset_new + last_dim1) * buf_z1.itemsize,
                    static_cast<const char*>(ptr_z2) + offset2 * buf_z2.itemsize,
                    last_dim2 * buf_z2.itemsize);

        // Copy x1 and x2
        std::memcpy(static_cast<char*>(ptr_new_x) + offset_new * buf_x1.itemsize,
                    static_cast<const char*>(ptr_x1) + offset1 * buf_x1.itemsize,
                    last_dim1 * buf_x1.itemsize);
        std::memcpy(static_cast<char*>(ptr_new_x) + (offset_new + last_dim1) * buf_x1.itemsize,
                    static_cast<const char*>(ptr_x2) + offset2 * buf_x2.itemsize,
                    last_dim2 * buf_x2.itemsize);
    }

    // Return the concatenated arrays as a tuple
    return py::make_tuple(new_z, new_x);
}

py::tuple compose(py::array z1, py::array x1, py::array z2, py::array x2){
    
}


py::array_t<bool> bitwise_commute_with(py::array z1, py::array x1, py::array z2, py::array x2) {
    py::array ovlp_1 = bitwise_and(z1, x2);
    py::array ovlp_2 = bitwise_and(x1, z2);
    py::array ovlp_3 = bitwise_xor(ovlp_1, ovlp_2);


    size_t num_qubits = ovlp_3.request().size;
    py::array_t<bool> result = py::array_t<bool>(num_qubits);
    auto buf_result = result.request();
    auto ptr_result = static_cast<bool*>(buf_result.ptr);

    auto buf1 = ovlp_3.request();
    const uint8_t* ptr1 = static_cast<const uint8_t*>(buf1.ptr);
    size_t n = buf1.size;

    #pragma omp parallel for if (n >= THRESHOLD_PARALLEL) schedule(static)
    for (size_t i = 0; i < n; ++i) {
        ptr_result[i] = ptr1[i] == 0;
    }
    return result.reshape(z1.request().shape);


}


py::tuple random_zx_strings(const std::vector<size_t>& shape, size_t num_qubits) {
    // Calculate the total size of the array
    size_t total_size = 1;
    for (size_t dim : shape) {
        total_size *= dim;
    }
    total_size *= num_qubits;

    // Create the full shape vector
    std::vector<size_t> full_shape = shape;
    full_shape.push_back(num_qubits);

    // Create output arrays for z_strings and x_strings
    py::array_t<bool> z_strings(full_shape);
    py::array_t<bool> x_strings(full_shape);

    auto buf_z = z_strings.request();
    auto buf_x = x_strings.request();

    bool* ptr_z = static_cast<bool*>(buf_z.ptr);
    bool* ptr_x = static_cast<bool*>(buf_x.ptr);

    // Random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1);

    // Fill the arrays with random values
    for (size_t i = 0; i < total_size; ++i) {
        ptr_z[i] = dist(gen);
        ptr_x[i] = dist(gen);
    }

    // Return the z_strings and x_strings as a tuple
    return py::make_tuple(z_strings, x_strings);
}