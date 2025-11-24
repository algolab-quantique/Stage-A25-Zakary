#pragma once
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <bit>
#include <cstdint> // uint8_t
#include <cstring>
#include <iostream>
#include <vector>

#ifdef USE_OPENMP
    #include <omp.h>
#else
    #warning "OpenMP is not enabled"
#endif

// bit_operations
// cz2m

// z2row
// z2r  = voids

// This threshold is completely arbitrary and can be tuned for performance depending on the
// hardware.
#define BOPS_THRESHOLD_PARALLEL 1'000'000

py::array bitwise_and(py::array z2r_1, py::array z2r_2);
py::array bitwise_xor(py::array z2r_1, py::array z2r_2);
py::array bitwise_or(py::array z2r_1, py::array z2r_2);
py::array bitwise_not(py::array voids);
py::array paded_bitwise_not(py::array voids, int num_qubits);
py::object bitwise_count(py::array z2r_1);
py::object bitwise_dot(py::array z2r_1, py::array z2r_2);

/**
 * @brief This templated function performs an element-wise, bitwise operation onto two NumPy
 * contiguous (C-like) arrays. Any other type of operators or type of arrays will lead to undefined
 * behavior. This function is the basis all two-array bitwise operations.
 *
 * @tparam Op The type of the bitwise operator (std::bit_and<uint64_t>, std::bit_xor<uint64_t>...)
 * @param z2r_1 The first input array from Python.
 * @param z2r_2 The second input array from Python
 * @param op The bitwise operator to apply
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing
 * the result of the operation
 */
template <typename Op> py::object bitwise_core(py::array z2r_1, py::array z2r_2, Op op) {
    auto buf1 = z2r_1.request();
    auto buf2 = z2r_2.request();

    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize (dtype compatibility).");
    }
    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size.");
    }

    size_t total_bytes = buf1.size * buf1.itemsize;
    py::array z2r_out = py::array(z2r_1.dtype(), buf1.shape);
    auto buf_out = z2r_out.request();

    // Cut the data into 64-bit chunks for faster proceCest ssing
    // ptr1_64 and ptr2_64 are the pointers to the input data, ptr_out_64 is the pointer to the
    // output data
    // TODO: With C++20, we could use std::assume_aligned, which could potentially lead to better
    // auto-vectorization by the compiler? ? Since NumPy cant guarantee alignement and we cant
    // afford to copy the data just to get better SIMD, I dont know if assume_aligned is worth
    // anything.
    const uint64_t *ptr1_64 = std::bit_cast<uint64_t *>(buf1.ptr);
    const uint64_t *ptr2_64 = std::bit_cast<uint64_t *>(buf2.ptr);
    int64_t *ptr_out_64 = std::bit_cast<int64_t *>(buf_out.ptr);

    size_t num_u64_chunks = total_bytes / 8;

#ifdef USE_OPENMP
    #pragma omp parallel for if (num_u64_chunks >= BOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t i = 0; i < num_u64_chunks; ++i) {
        // Applies the bitwise operation.
        ptr_out_64[i] = op(ptr1_64[i], ptr2_64[i]);
    }

    // Handle any bytes that don't fit into a 64-bit chunk (the tail)
    size_t tail_bytes = total_bytes % 8;
    if (tail_bytes > 0) {
        const uint8_t *ptr1_8 = std::bit_cast<uint8_t *>(buf1.ptr);
        const uint8_t *ptr2_8 = std::bit_cast<uint8_t *>(buf2.ptr);
        int8_t *ptr_out_8 = std::bit_cast<int8_t *>(buf_out.ptr);

        size_t start_byte = num_u64_chunks * 8;
        for (size_t i = 0; i < tail_bytes; ++i) {
            ptr_out_8[start_byte + i] = op(ptr1_8[start_byte + i], ptr2_8[start_byte + i]);
        }
    }

    // If the input arrays were actually scalars (shape == ()), return a scalar as well
    if (buf1.size == 1) {
        std::vector<ssize_t> shape0{};   // zero-dim
        std::vector<ssize_t> strides0{}; // must match ndim (0)
        py::array scalar(z2r_out.dtype(), shape0, strides0, buf_out.ptr,
                         z2r_out); // TODO: Check if this is zero-copy
        return scalar;
    }

    return z2r_out;
}
