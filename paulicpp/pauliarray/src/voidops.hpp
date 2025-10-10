/**
 * @file voidops.hpp
 * @author Zakary Romdhane (zakaryromdhane@gmail.com)
 * @brief This is a C++ implementation of PauliArray's bitwise operations (found in pauliarray/binary/void_operations.py)
 * The main goal of this file is to provide fast implementations of bitwise operations on NumPy void arrays.
 * 
 * @attention It is NECESSARY to have gcc/clang in order to compile this file, as it uses OpenMP and some gcc/clang intrinsics.
 * It is assumed that both of the input arrays are contiguous, of the dtype |V{N}, and the exacte same shape. No checks are performed to ensure this is the case.
 * Any broadcasting and contiguity checks must be performed in Python before calling any of these functions.
 * Any computers which are not 64-bit architectures will lead to undefined behavior due to the casting to uint64_t*.
 * 
 * @todo Convert from C++17 to C++20 in order to use std::popcount from <bit> instead of compiler intrinsic __builtin_popcountll. 
 * This would make the code actually portable to MSVC.
 * 
 * @version 0.1.0
 * @date 2025-10-01
 * 
 * @copyright Copyright (c) 2025
 */


#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <cstdint> // uint8_t
#include <iostream>
#include <cstring>
#include <omp.h>
#include <bit>

// #ifdef _MSC_VER
//     #include <intrin.h>
//     #define __builtin_popcount __popcnt
//     #define __builtin_popcountll __popcnt64
// #endif

// This threshold is completely arbitrary and can be tuned for performance depending on the hardware.
#define VOPS_THRESHOLD_PARALLEL 10'000'000
\
/**
 * @brief This templated function performs an element-wise, bitwise operation onto two NumPy contiguous (C-like) arrays.
 * Any other type of operators or type of arrays will lead to undefined behavior.
 * This function is the basis all two-array bitwise operations.
 * 
 * @tparam Op The type of the bitwise operator (std::bit_and<uint64_t>, std::bit_xor<uint64_t>...)
 * @param voids_1 The first input array from Python.
 * @param voids_2 The second input array from Python
 * @param op The bitwise operator to apply
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing the result of the operation
 */
template <typename Op>
inline py::array bitwise_core(py::array voids_1, py::array voids_2, Op op) {
    auto buf1 = voids_1.request();
    auto buf2 = voids_2.request();

    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize (dtype compatibility).");
    }
    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size.");
    }

    size_t total_bytes = buf1.size * buf1.itemsize;
    py::array voids_out = py::array(voids_1.dtype(), buf1.shape);
    auto buf_out = voids_out.request();

    // Cut the data into 64-bit chunks for faster processing 
    // ptr1_64 and ptr2_64 are the pointers to the input data, ptr_out_64 is the pointer to the output data
    const uint64_t*  ptr1_64 = static_cast<const uint64_t*>(buf1.ptr);
    const uint64_t*  ptr2_64 = static_cast<const uint64_t*>(buf2.ptr);
    uint64_t*  ptr_out_64 = static_cast<uint64_t*>(buf_out.ptr);

    size_t num_64bit_chunks = total_bytes / 8;

    #pragma omp parallel for if(num_64bit_chunks >= VOPS_THRESHOLD_PARALLEL) schedule(static)
    for (size_t i = 0; i < num_64bit_chunks; ++i) {
        // Applies the bitwise operation.
        ptr_out_64[i] = op(ptr1_64[i], ptr2_64[i]);
    }

    // Handle any bytes that don't fit into a 64-bit chunk (the tail)
    size_t remaining_bytes = total_bytes % 8;
    if (remaining_bytes > 0) {
        const uint8_t*  ptr1_8 = static_cast<const uint8_t*>(buf1.ptr);
        const uint8_t*  ptr2_8 = static_cast<const uint8_t*>(buf2.ptr);
        uint8_t*  ptr_out_8 = static_cast<uint8_t*>(buf_out.ptr);

        size_t start_byte = num_64bit_chunks * 8;
        for (size_t i = 0; i < remaining_bytes; ++i) {
            ptr_out_8[start_byte + i] = op(ptr1_8[start_byte + i], ptr2_8[start_byte + i]);
        }
    }

    return voids_out;
}

// * Bon resultats
//TODO: rendre ca vite
/**
 * @brief Performs an element-wise bitwise AND operation on two NumPy contiguous (C-like) arrays element-wise.
 * 
 * @param voids_1 the first input array
 * @param voids_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing the result of the bitwise AND operation
 */
inline py::array bitwise_and(py::array voids_1, py::array voids_2) {
    return bitwise_core(voids_1, voids_2, std::bit_and<uint64_t>());
}

/**
 * @brief Performs an element-wise bitwise XOR operation on two NumPy contiguous (C-like) arrays.
 * 
 * @param voids_1 the first input array
 * @param voids_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing the result of the bitwise AND operation
 */
inline py::array bitwise_xor(py::array voids_1, py::array voids_2) {
    return bitwise_core(voids_1, voids_2, std::bit_xor<uint64_t>());
}

/**
 * @brief Performs an element-wise bitwise OR operation on two NumPy contiguous (C-like) arrays.
 * 
 * @param voids_1 the first input array
 * @param voids_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the inputs, containing the result of the bitwise AND operation
 */
inline py::array bitwise_or(py::array voids_1, py::array voids_2) {
    return bitwise_core(voids_1, voids_2, std::bit_or<uint64_t>());
}


/**
 * @brief Performs an element-wise bitwise NOT operation on a NumPy contiguous (C-like) array.
 * In other words, it flips all bits in the array.
 * 
 * @param voids the input array
 * @return py::array A NumPy contiguous array of the same shape and dtype as the input, containing the result of the bitwise NOT operation
 */
inline py::array bitwise_not(py::array voids) {
    auto buf = voids.request();
    size_t total_bytes = buf.size * buf.itemsize;
    py::array res_voids = py::array(voids.dtype(), buf.shape);
    auto buf_out = res_voids.request();

    const uint64_t*  ptr_64 = static_cast<const uint64_t*>(buf.ptr);
    uint64_t*  ptr_out_64 = static_cast<uint64_t*>(buf_out.ptr);

    size_t num_64bit_chunks = total_bytes / 8;

    #pragma omp parallel for if (num_64bit_chunks >= VOPS_THRESHOLD_PARALLEL) schedule(static)
    for (size_t i = 0; i < num_64bit_chunks; ++i) {
        ptr_out_64[i] = ~ptr_64[i];
    }

    size_t remaining_bytes = total_bytes % 8;
    if (remaining_bytes > 0) {
        const uint8_t*  ptr_8 = static_cast<const uint8_t*>(buf.ptr);
        uint8_t*  ptr_out_8 = static_cast<uint8_t*>(buf_out.ptr);
        
        size_t start_byte = num_64bit_chunks * 8;
        for (size_t i = 0; i < remaining_bytes; ++i) {
            ptr_out_8[start_byte + i] = ~ptr_8[start_byte + i];
        }
    }

    return res_voids;
}

/** 
 * @brief Counts the number of set bits in each element of a NumPy contiguous (C-like) array.
 * In other words, it returns the number of 1s found inside each element of the array.
 * 
 * @attention 
 * This function uses the gcc/clang intrinsic __builtin_popcount() as its core operation.
 * Thus, whilst very fast, it is NOT PORTABLE to non-gcc/clang compilers
 * 
 * @param voids_1 the input array
 * @return py::array An array of the number of set bits in each element of the input array
 */
inline py::array bitwise_count(py::array voids_1) {
    auto buf1 = voids_1.request();

    py::array_t<int64_t> result(buf1.size);
    auto buf_out = result.request();

    const uint8_t* ptr1 = static_cast<const uint8_t*>(buf1.ptr);
    int64_t*  ptr_out = static_cast<int64_t*>(buf_out.ptr);

    size_t itemsize = buf1.itemsize;
    size_t n = buf1.size;
    size_t n64 = itemsize / 8;
    size_t tail = itemsize % 8;

    size_t total_64_chunks = n * n64;

    #pragma omp parallel for if (total_64_chunks >= VOPS_THRESHOLD_PARALLEL) schedule(static)
    for (size_t i = 0; i < n; ++i) {
        const uint8_t* base = ptr1 + i * itemsize;
        int64_t count = 0;
        for (size_t k = 0; k < n64; ++k) {
            uint64_t word;
            std::memcpy(&word, base + k * 8, 8);
            count += std::popcount(word);
        }
        for (size_t t = 0; t < tail; ++t) {
            count += std::popcount(static_cast<uint8_t>(base[n64 * 8 + t]));
        }
        ptr_out[i] = count;
    }

    result.resize(buf1.shape);

    return result;
}


/**
 * @brief Computes the bitwise dot product between corresponding elements of two NumPy contiguous (C-like) arrays.
 * 
 * @attention 
 * This function uses the gcc/clang intrinsic __builtin_popcount() as its core operation.
 * Thus, whilst very fast, it is NOT PORTABLE to non-gcc/clang compilers
 * 
 * @param voids_1 the first input array
 * @param voids_2 the second input array
 * @return py::array A NumPy contiguous array of the same shape as the inputs, containing the bitwise dot product.
 */
inline py::array bitwise_dot(py::array voids_1, py::array voids_2) {
    auto buf1 = voids_1.request();
    auto buf2 = voids_2.request();

    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize. Got " +
            std::to_string(buf1.itemsize) + " and " + std::to_string(buf2.itemsize));
    }
    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size. Got " +
            std::to_string(buf1.size) + " and " + std::to_string(buf2.size));
    }

    py::array_t<int64_t> result(buf1.shape);
    auto buf_out = result.request();

    const uint8_t* ptr1 = static_cast<const uint8_t*>(buf1.ptr);
    const uint8_t* ptr2 = static_cast<const uint8_t*>(buf2.ptr);
    int64_t*  ptr_out = static_cast<int64_t*>(buf_out.ptr);

    size_t itemsize = buf1.itemsize;
    size_t n = buf1.size;
    size_t n64 = itemsize / 8;
    size_t tail = itemsize % 8;
    size_t total_64_chunks = n * n64;

    #pragma omp parallel for if (total_64_chunks >= VOPS_THRESHOLD_PARALLEL) schedule(static)
    for (size_t i = 0; i < n; ++i) {
        const uint8_t* base1 = ptr1 + i * itemsize;
        const uint8_t* base2 = ptr2 + i * itemsize;
        int64_t count = 0;
        for (size_t k = 0; k < n64; ++k) {
            uint64_t w1, w2;
            std::memcpy(&w1, base1 + k * 8, 8);
            std::memcpy(&w2, base2 + k * 8, 8);
            count += std::popcount(w1 & w2);
        }
        for (size_t t = 0; t < tail; ++t) {
            count += std::popcount(static_cast<uint8_t>(base1[n64 * 8 + t] & base2[n64 * 8 + t]));
        }
        ptr_out[i] = count;
    }

    return result;
}

