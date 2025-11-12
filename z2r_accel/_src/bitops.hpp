/**
 * @file voidops.hpp
 * @author Zakary Romdhane (zakaryromdhane@gmail.com)
 * @brief This is a C++ implementation of PauliArray's bitwise operations (found in
 * pauliarray/binary/void_operations.py) The main goal of this file is to provide fast
 * implementations of bitwise operations on NumPy void arrays.
 *
 * @attention Your compiler must support at least C++20 standard to properly compile this file.
 *
 * @todo Add SIMD support
 *
 * It is assumed that both of the input arrays are contiguous, of the dtype |V{N}, and the exacte
 * same shape. No checks are performed to ensure this is the case. Any broadcasting and contiguity
 * checks MUST be performed in Python before calling any of these functions. Any computers which are
 * not 64-bit architectures will lead to undefined behavior due to the casting to uint64_t*.
 *
 * @version 0.1.1
 * @date 2025-10-01
 *
 * @copyright Copyright (c) 2025
 */

// meme nbr de chaines de pauli, mais entre 64 et 32 bnits

// cz2m

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
template <typename Op> inline py::object bitwise_core(py::array z2r_1, py::array z2r_2, Op op) {
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

/**
 * @brief Performs an element-wise bitwise AND operation on two NumPy contiguous (C-like) arrays
 * element-wise.
 *
 * @param z2r_1 the first input array
 * @param z2r_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing
 * the result of the bitwise AND operation
 */
inline py::array bitwise_and(py::array z2r_1, py::array z2r_2) {
    return bitwise_core(z2r_1, z2r_2, std::bit_and<uint64_t>());
}

/**
 * @brief Performs an element-wise bitwise XOR operation on two NumPy contiguous (C-like) arrays.
 *
 * @param z2r_1 the first input array
 * @param z2r_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing
 * the result of the bitwise AND operation
 */
inline py::array bitwise_xor(py::array z2r_1, py::array z2r_2) {
    return bitwise_core(z2r_1, z2r_2, std::bit_xor<uint64_t>());
}

/**
 * @brief Performs an element-wise bitwise OR operation on two NumPy contiguous (C-like) arrays.
 *
 * @param z2r_1 the first input array
 * @param z2r_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing
 * the result of the bitwise AND operation
 */
inline py::array bitwise_or(py::array z2r_1, py::array z2r_2) {
    return bitwise_core(z2r_1, z2r_2, std::bit_or<uint64_t>());
}

/**
 * @brief Performs an element-wise bitwise NOT operation on a NumPy contiguous (C-like) array.
 * In other words, it flips all bits in the array.
 *
 * @param voids the input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the input, containing
 * the result of the bitwise NOT operation
 */
// TODO: update to reflect new changes in bitwise_core for scalars
inline py::array bitwise_not(py::array voids) {
    auto buf = voids.request();
    size_t total_bytes = buf.size * buf.itemsize;
    py::array res_voids = py::array(voids.dtype(), buf.shape);
    auto buf_out = res_voids.request();

    const uint64_t *ptr_64 = std::bit_cast<uint64_t *>(buf.ptr);
    int64_t *ptr_out_64 = std::bit_cast<int64_t *>(buf_out.ptr);

    size_t num_u64_chunks = total_bytes / 8;

#ifdef USE_OPENMP
    #pragma omp parallel for if (num_u64_chunks >= BOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t i = 0; i < num_u64_chunks; ++i) {
        ptr_out_64[i] = ~ptr_64[i];
    }

    size_t tail_bytes = total_bytes % 8;
    if (tail_bytes > 0) {
        const uint8_t *ptr_8 = std::bit_cast<const uint8_t *>(buf.ptr);
        int8_t *ptr_out_8 = std::bit_cast<int8_t *>(buf_out.ptr);

        size_t start_byte = num_u64_chunks * 8;
        for (size_t i = 0; i < tail_bytes; ++i) {
            ptr_out_8[start_byte + i] = ~ptr_8[start_byte + i];
        }
    }

    return res_voids;
}

// ! THIS DOES NOT WORK AT ALL!!! DO NOT USE!
// Todo: fix this
/**
 * @brief Performs an element-wise bitwise NOT operation on only the first `num_qubits` bits of a
 * NumPy contiguous (C-like) array. In other words, it flips the first `num_qubits` bits in the
 * array, with the remaining bits left unchanged.
 *
 * @param voids the input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the input, containing
 * the result of the bitwise NOT operation
 */
inline py::array paded_bitwise_not(py::array voids, int num_qubits) {
    auto buf = voids.request();
    size_t total_bytes = num_qubits * 8;
    py::array res_voids = py::array(voids.dtype(), buf.shape);
    auto buf_out = res_voids.request();

    const uint64_t *ptr_64 = std::bit_cast<uint64_t *>(buf.ptr);
    int64_t *ptr_out_64 = std::bit_cast<int64_t *>(buf_out.ptr);

    size_t num_u64_chunks = total_bytes / 8;

#ifdef USE_OPENMP
    #pragma omp parallel for if (num_u64_chunks >= BOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t i = 0; i < num_u64_chunks; ++i) {
        ptr_out_64[i] = ~ptr_64[i];
    }

    size_t tail_bytes = total_bytes % 8;
    if (tail_bytes > 0) {
        const uint8_t *ptr_8 = std::bit_cast<const uint8_t *>(buf.ptr);
        //
        int8_t *ptr_out_8 = std::bit_cast<int8_t *>(buf_out.ptr);

        size_t start_byte = num_u64_chunks * 8;
        for (size_t i = 0; i < tail_bytes; ++i) {
            ptr_out_8[start_byte + i] = ~ptr_8[start_byte + i];
        }
    }

    return res_voids;
}

/**
 * @brief Counts the number of set bits in each element of a NumPy contiguous (C-like) array.
 * In other words, it returns the number of 1s found inside each element of the array.
 *
 * @param z2r_1 the input array
 * @return py::array An array of the number of set bits in each element of the input array
 */
inline py::object bitwise_count(py::array z2r_1) {
    auto buf_in = z2r_1.request();

    py::array_t<int64_t> result(buf_in.size);
    auto buf_out = result.request();

    const uint8_t *ptr1 = std::bit_cast<const uint8_t *>(buf_in.ptr);
    int64_t *ptr_out = std::bit_cast<int64_t *>(buf_out.ptr);

    size_t itemsize = buf_in.itemsize;
    size_t num_elem = buf_in.size;
    size_t u64_per_elem = itemsize / 8;
    size_t tail_bytes = itemsize % 8;

    size_t total_64_chunks = num_elem * u64_per_elem;

    if (num_elem == 1) {
        // Specila case for when the NumPy array is one-dimensional.
        // This is necessary in order to return the exact same output as the Python version of this
        // function. i.e. a single integer instead of a one-element array.
        const uint8_t *base = ptr1;
        int64_t count = 0;

#ifdef USE_OPENMP
    #pragma omp parallel for if (u64_per_elem >= BOPS_THRESHOLD_PARALLEL) schedule(static)         \
        reduction(+ : count)
#endif
        for (size_t k = 0; k < u64_per_elem; ++k) {
            uint64_t word;
            std::memcpy(&word, base + k * 8, 8);
            count += std::popcount(word);
        }
        for (size_t t = 0; t < tail_bytes; ++t) {
            count += std::popcount(static_cast<uint8_t>(base[u64_per_elem * 8 + t]));
        }
        return py::int_(count);
    }

#ifdef USE_OPENMP
    #pragma omp parallel for if (total_64_chunks >= BOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t i = 0; i < num_elem; ++i) {
        const uint8_t *base = ptr1 + i * itemsize;
        int64_t count = 0;
        for (size_t k = 0; k < u64_per_elem; ++k) {
            uint64_t word;
            std::memcpy(&word, base + k * 8, 8);
            count += std::popcount(word);
        }
        for (size_t t = 0; t < tail_bytes; ++t) {
            count += std::popcount(static_cast<uint8_t>(base[u64_per_elem * 8 + t]));
        }
        ptr_out[i] = count;
    }

    result.resize(buf_in.shape);

    return result;
}

/**
 * @brief Computes the bitwise dot product between corresponding elements of two NumPy contiguous
 * (C-like) arrays.
 *
 * @param z2r_1 the first input array
 * @param z2r_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape as the inputs, containing the
 * bitwise dot product.
 */
inline py::object bitwise_dot(py::array z2r_1, py::array z2r_2) {
    auto buf1 = z2r_1.request();
    auto buf2 = z2r_2.request();

    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize. Got " +
                                 std::to_string(buf1.itemsize) + " and " +
                                 std::to_string(buf2.itemsize));
    }
    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size. Got " +
                                 std::to_string(buf1.size) + " and " + std::to_string(buf2.size));
    }

    py::array_t<int64_t> result(buf1.shape);
    auto buf_out = result.request();

    const uint8_t *ptr1 = std::bit_cast<const uint8_t *>(buf1.ptr);
    const uint8_t *ptr2 = std::bit_cast<const uint8_t *>(buf2.ptr);
    int64_t *ptr_out = std::bit_cast<int64_t *>(buf_out.ptr);

    size_t itemsize = buf1.itemsize;
    size_t num_elem = buf1.size;
    size_t u64_per_elem = itemsize / 8;
    size_t tail_bytes = itemsize % 8;
    size_t total_64_chunks = num_elem * u64_per_elem;

    if (num_elem == 1) {
        // Specila case for when the NumPy array is one-dimensional.
        // This is necessary in order to return the exact same output as the Python version of this
        // function. i.e. a single integer instead of a one-element array.
        const uint8_t *base1 = ptr1;
        const uint8_t *base2 = ptr2;
        int64_t count = 0;

#ifdef USE_OPENMP
    #pragma omp parallel for if (u64_per_elem >= BOPS_THRESHOLD_PARALLEL) schedule(static)         \
        reduction(+ : count)
#endif
        for (size_t k = 0; k < u64_per_elem; ++k) {
            uint64_t w1, w2;
            std::memcpy(&w1, base1 + k * 8, 8);
            std::memcpy(&w2, base2 + k * 8, 8);
            count += std::popcount(w1 & w2);
        }
        for (size_t t = 0; t < tail_bytes; ++t) {
            count += std::popcount(
                static_cast<uint8_t>(base1[u64_per_elem * 8 + t] & base2[u64_per_elem * 8 + t]));
        }
        return py::int_(count);
    }

#ifdef USE_OPENMP
    #pragma omp parallel for if (total_64_chunks >= BOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t i = 0; i < num_elem; ++i) {
        const uint8_t *base1 = ptr1 + i * itemsize;
        const uint8_t *base2 = ptr2 + i * itemsize;
        int64_t count = 0;
        for (size_t k = 0; k < u64_per_elem; ++k) {
            uint64_t w1, w2;
            std::memcpy(&w1, base1 + k * 8, 8);
            std::memcpy(&w2, base2 + k * 8, 8);
            count += std::popcount(w1 & w2);
        }
        for (size_t t = 0; t < tail_bytes; ++t) {
            count += std::popcount(
                static_cast<uint8_t>(base1[u64_per_elem * 8 + t] & base2[u64_per_elem * 8 + t]));
        }
        ptr_out[i] = count;
    }

    return result;
}

// test