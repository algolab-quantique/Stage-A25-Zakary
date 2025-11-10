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


 //cz2m
 
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
//cz2m

//z2row
//z2r  = voids

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
    #pragma omp parallel for if (u64_per_elem >= BOPS_THRESHOLD_PARALLEL) schedule(static)                  \
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
    #pragma omp parallel for if (u64_per_elem >= BOPS_THRESHOLD_PARALLEL) schedule(static)                  \
        reduction(+ : count)
#endif
        for (size_t k = 0; k < u64_per_elem; ++k) {
            uint64_t w1, w2;
            std::memcpy(&w1, base1 + k * 8, 8);
            std::memcpy(&w2, base2 + k * 8, 8);
            count += std::popcount(w1 & w2);
        }
        for (size_t t = 0; t < tail_bytes; ++t) {
            count += std::popcount(static_cast<uint8_t>(base1[u64_per_elem * 8 + t] & base2[u64_per_elem * 8 + t]));
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
            count += std::popcount(static_cast<uint8_t>(base1[u64_per_elem * 8 + t] & base2[u64_per_elem * 8 + t]));
        }
        ptr_out[i] = count;
    }

    return result;
}

/**
 * @brief Transposes a technically 1D array of voids by interpreting it as a 2D array of bits.
 * In other words, if the input array has shape (M,) and each element has N bits, the output array
 * will have shape (N, M), where the rows correspond to the bits of the input elements.
 *
 * If input is:
 * [0110,
 *  1001,
 *  1111]
 *
 * then output will be:
 * [011,
 *  101,
 *  101,
 *  011]
 *
 * @param voids Input array
 * @return py::array Transposed array with minimal dtype
 */
inline py::array bitwise_transpose(py::array voids, int64_t num_bits = -1) {
    auto buf = voids.request();

    size_t M = buf.size;                // Number of elements
    size_t max_bits = buf.itemsize * 8; // Maximum bits per element

    // If num_bits not specified, use all bits in dtype
    // This will create trailing zero bits, but its better than nothing
    size_t N_bits = (num_bits > 0) ? static_cast<size_t>(num_bits) : max_bits;

    if (N_bits > max_bits) {
        throw std::runtime_error("num_bits cannot exceed itemsize * 8");
    }
    // Calculate output dtype size (smallest possible to hold M bits)
    size_t out_bytes = (M + 7) / 8; // Ceiling division

    // Create output array with shape (N_bits,) and dtype |V{out_bytes}
    std::string dtype_str =
        "|V" + std::to_string(out_bytes); // a revisiter, peux etre pas necessaire
    py::dtype out_dtype(dtype_str);
    std::vector<ssize_t> out_shape = {static_cast<ssize_t>(N_bits)};
    py::array z2r_out = py::array(out_dtype, out_shape);
    auto buf_out = z2r_out.request();

    const uint8_t *ptr_in = std::bit_cast<const uint8_t *>(buf.ptr);
    uint8_t *ptr_out = std::bit_cast<uint8_t *>(buf_out.ptr);

    // Initialize output to zero
    std::memset(ptr_out, 0, N_bits * out_bytes);

    // Transpose: bit j of element i becomes bit i of element j
#ifdef USE_OPENMP
    #pragma omp parallel for if (N_bits * M >= BOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t j = 0; j < N_bits; ++j) {
        size_t byte_idx_in = j / 8;
        size_t bit_idx_in = j % 8;

        for (size_t i = 0; i < M; ++i) {
            // Get bit j from element i
            uint8_t bit = (ptr_in[i * buf.itemsize + byte_idx_in] >> bit_idx_in) & 1;

            // Set bit i in element j of output
            size_t byte_idx_out = i / 8;
            size_t bit_idx_out = i % 8;

            if (bit) {
                ptr_out[j * out_bytes + byte_idx_out] |= (1 << bit_idx_out);
            }
        }
    }

    return z2r_out;
}

/**
 * @brief Matrix multiplication of two void arrays (technically 1D), interpreted as 2D bit matrices.
 *
 * @param z2r_a
 * @param z2r_b
 * @return py::array
 */
inline py::array bitwise_matmul(py::array z2r_a, py::array z2r_b, int a_num_qubits,
                                int b_num_qubits) {
    auto buf1 = z2r_a.request();
    auto buf2 = z2r_b.request();

    size_t a_rows = buf1.size;
    size_t a_cols = a_num_qubits; // Number of bits per element in void
    size_t b_rows = buf2.size;
    size_t b_cols = b_num_qubits; // Number of bits per element in void

    std::cout << "A shape: (" << a_rows << ", " << a_cols << ")\n";
    std::cout << "B shape: (" << b_rows << ", " << b_cols << ")\n";

    if (a_cols != b_rows) {
        throw std::runtime_error("Shape mismatch for matrix multiplication: A columns (" +
                                 std::to_string(a_cols) + ") must equal B rows (" +
                                 std::to_string(b_rows) + ").");
    }

    size_t out_bytes = (b_cols + 7) / 8;
    std::string dtype_str =
        "|V" + std::to_string(out_bytes); // a revisiter, peux etre pas necessaire
    py::dtype out_dtype(dtype_str);

    // output is 1-D array of a_rows elements, each a void of out_bytes
    std::vector<ssize_t> out_shape = {static_cast<ssize_t>(a_rows)};
    py::array z2r_out = py::array(out_dtype, out_shape);
    auto buf_out = z2r_out.request();

    std::memset(buf_out.ptr, 0, buf_out.size * buf_out.itemsize);
    // Output array with shape (a_rows, b_cols)

    const uint8_t *ptr_a = std::bit_cast<const uint8_t *>(buf1.ptr);
    const uint8_t *ptr_b = std::bit_cast<const uint8_t *>(buf2.ptr);
    int8_t *ptr_out = std::bit_cast<int8_t *>(buf_out.ptr);

    //TODO: Parallelize this whole block
    for (size_t i = 0; i < a_rows; i++) {
        for (size_t j = 0; j < b_cols; j++) {
            int8_t bit_sum = 0;
            for (size_t k = 0; k < a_cols; k++) {
                // Get bit k of row i in A
                size_t a_byte_idx = k / 8;
                size_t a_bit_idx = k % 8;
                uint8_t a_bit = (ptr_a[i * buf1.itemsize + a_byte_idx] >> a_bit_idx) & 1;

                // Get bit j of row k in B
                size_t b_byte_idx = j / 8;
                size_t b_bit_idx = j % 8;
                uint8_t b_bit = (ptr_b[k * buf2.itemsize + b_byte_idx] >> b_bit_idx) & 1;

                bit_sum += a_bit & b_bit;
            }
            // Set bit j of row i in output
            size_t out_byte_idx = j / 8;
            size_t out_bit_idx = j % 8;
            if (bit_sum % 2) { // Modulo 2 for bitwise addition
                ptr_out[i * buf_out.itemsize + out_byte_idx] |= (1 << out_bit_idx);
            }
        }
    }
    return z2r_out;
}



// def row_echelon(bit_matrix: "np.ndarray[np.bool]") -> "np.ndarray[np.bool]":
//     """
//     Applies Gauss-Jordan elimination on a binary matrix to produce row echelon form.

//     Args:
//         bit_matrix ("np.ndarray[np.bool]"): Input binary matrix.

//     Returns:
//         "np.ndarray[np.bool]": Row echelon form of the provided matrix.
//     """
//     re_bit_matrix = bit_matrix.copy().astype(bool)

//     n_rows = re_bit_matrix.shape[0]
//     n_cols = re_bit_matrix.shape[1]

//     row_range = np.arange(n_rows)

//     h_row = 0
//     k_col = 0

//     while h_row < n_rows and k_col < n_cols:
//         if np.all(re_bit_matrix[h_row:, k_col] == 0):
//             k_col += 1
//         else:
//             i_row = h_row + np.argmax(re_bit_matrix[h_row:, k_col])
//             if i_row != h_row:
//                 re_bit_matrix[[i_row, h_row], :] = re_bit_matrix[[h_row, i_row], :]

//             cond_rows = np.logical_and(re_bit_matrix[:, k_col], (row_range != h_row))

//             re_bit_matrix[cond_rows, :] = np.logical_xor(re_bit_matrix[cond_rows, :], re_bit_matrix[h_row, :][None, :])

//             h_row += 1
//             k_col += 1

//     return re_bit_matrix

