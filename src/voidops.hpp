#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <cstdint> // uint8_t
#include <iostream>
#include <cstring> // For memcpy
#include <omp.h>
#include <immintrin.h>


template <typename Op>
py::array bitwise_core(py::array voids_1, py::array voids_2, Op op) {
    auto buf1 = voids_1.request();
    auto buf2 = voids_2.request();

    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize (dtype compatibility).");
    }
    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size.");
    }

    size_t total_bytes = buf1.size * buf1.itemsize;
    py::array voids_3 = py::array(voids_1.dtype(), buf1.shape);
    auto buf3 = voids_3.request();

    const uint64_t* __restrict__ ptr1_64 = static_cast<const uint64_t*>(buf1.ptr);
    const uint64_t* __restrict__ ptr2_64 = static_cast<const uint64_t*>(buf2.ptr);
    uint64_t* __restrict__ ptr3_64 = static_cast<uint64_t*>(buf3.ptr);

    size_t num_64bit_chunks = total_bytes / 8;

    // if (num_64bit_chunks < 10000000) {
    //     #pragma omp simd
    //     for (size_t i = 0; i < num_64bit_chunks; ++i) {
    //         ptr3_64[i] = op(ptr1_64[i], ptr2_64[i]);
    //     }
    // } else {
        #pragma omp parallel for if(num_64bit_chunks >= 10000000) schedule(static)
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            ptr3_64[i] = op(ptr1_64[i], ptr2_64[i]);
        }
    // }

    size_t remaining_bytes = total_bytes % 8;
    if (remaining_bytes > 0) {
        const uint8_t* __restrict__ ptr1_8 = static_cast<const uint8_t*>(buf1.ptr);
        const uint8_t* __restrict__ ptr2_8 = static_cast<const uint8_t*>(buf2.ptr);
        uint8_t* __restrict__ ptr3_8 = static_cast<uint8_t*>(buf3.ptr);

        size_t start_byte = num_64bit_chunks * 8;
        for (size_t i = 0; i < remaining_bytes; ++i) {
            ptr3_8[start_byte + i] = op(ptr1_8[start_byte + i], ptr2_8[start_byte + i]);
        }
    }

    return voids_3;
}

// * Bon resultats
//TODO: rendre ca vite
py::array bitwise_and(py::array voids_1, py::array voids_2) {
    return bitwise_core(voids_1, voids_2, std::bit_and<uint64_t>());
}


py::array bitwise_xor(py::array voids_1, py::array voids_2) {
    return bitwise_core(voids_1, voids_2, std::bit_xor<uint64_t>());
}


py::array bitwise_or(py::array voids_1, py::array voids_2) {
    return bitwise_core(voids_1, voids_2, std::bit_or<uint64_t>());
}


py::array bitwise_not(py::array voids) {
    auto buf = voids.request();
    size_t total_bytes = buf.size * buf.itemsize;
    py::array new_voids = py::array(voids.dtype(), buf.shape);
    auto new_buf = new_voids.request();

    const uint64_t* ptr_64 = static_cast<const uint64_t*>(buf.ptr);
    uint64_t* new_ptr_64 = static_cast<uint64_t*>(new_buf.ptr);

    size_t num_64bit_chunks = total_bytes / 8;

    if (num_64bit_chunks < 10000000) { 
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            new_ptr_64[i] = ~ptr_64[i];
        }
    } else {
        #pragma omp parallel for 
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            new_ptr_64[i] = ~ptr_64[i];
        }
    }

    size_t remaining_bytes = total_bytes % 8;
    if (remaining_bytes > 0) {
        const uint8_t* ptr_8 = static_cast<const uint8_t*>(buf.ptr);
        uint8_t* new_ptr_8 = static_cast<uint8_t*>(new_buf.ptr);
        
        size_t start_byte = num_64bit_chunks * 8;
        for (size_t i = 0; i < remaining_bytes; ++i) {
            new_ptr_8[start_byte + i] = ~ptr_8[start_byte + i];
        }
    }

    return new_voids;
}


// * OK!
// Bon resultats
// uint64_t bitwise_count(py::array voids) {
//     auto buf = voids.request();
//     uint64_t total_count = 0;
//     size_t total_bytes = buf.size * buf.itemsize;
//     size_t num_64bit_chunks = total_bytes / 8;
//     const uint64_t* ptr_64 = static_cast<const uint64_t*>(buf.ptr);

  
//     if (num_64bit_chunks < 10000000) { 
//         for (size_t i = 0; i < num_64bit_chunks; ++i) {
//             // total_count += std::__popcount(ptr_64[i]);
//             total_count += __builtin_popcountll(ptr_64[i]);

//         }
//     } else {
//         #pragma omp parallel for reduction(+:total_count)
//         for (size_t i = 0; i < num_64bit_chunks; ++i) {
//             total_count += __builtin_popcountll(ptr_64[i]);
//         }
//     }

//     // pour les bytes restants, pratiquement juste utilisé pour des voids de taille < 64 bits
//     size_t remaining_bytes = total_bytes % 8;
//     if (remaining_bytes > 0) {
//         const uint8_t* ptr_8 = static_cast<const uint8_t*>(buf.ptr);
//         size_t start_byte = num_64bit_chunks * 8;
//         for (size_t i = 0; i < remaining_bytes; ++i) {
//             total_count += __builtin_popcount(ptr_8[start_byte + i]);
//         }
//     }
//     return total_count;
// }

py::array bitwise_count(py::array voids_1) {
    auto buf1 = voids_1.request();

    py::array_t<int64_t> result(buf1.size);
    auto buf_result = result.request();

    const uint8_t* ptr1 = static_cast<const uint8_t*>(buf1.ptr);
    int64_t* ptr_out = static_cast<int64_t*>(buf_result.ptr);

    size_t itemsize = buf1.itemsize;
    size_t n = buf1.size;
    size_t n64 = itemsize / 8;
    size_t tail = itemsize % 8;

    size_t total_64_chunks = n * n64;

    // if (total_64_chunks < 10000000) {
    //     for (size_t i = 0; i < n; ++i) {
    //         const uint8_t* base = ptr1 + i * itemsize;
    //         int64_t count = 0;
    //         for (size_t k = 0; k < n64; ++k) {
    //             uint64_t word;
    //             std::memcpy(&word, base + k * 8, 8);
    //             count += __builtin_popcountll(word);
    //         }
    //         for (size_t t = 0; t < tail; ++t) {
    //             count += __builtin_popcount(base[n64 * 8 + t]);
    //         }
    //         ptr_out[i] = count;
    //     }
    // } else {
        #pragma omp parallel for if (total_64_chunks >= 10000000) schedule(static)
        for (size_t i = 0; i < n; ++i) {
            const uint8_t* base = ptr1 + i * itemsize;
            int64_t count = 0;
            for (size_t k = 0; k < n64; ++k) {
                uint64_t word;
                std::memcpy(&word, base + k * 8, 8);
                count += __builtin_popcountll(word);
            }
            for (size_t t = 0; t < tail; ++t) {
                count += __builtin_popcount(base[n64 * 8 + t]);
            }
            ptr_out[i] = count;
        }
    // }

    result.resize(buf1.shape);

    return result;
}


py::array bitwise_dot(py::array voids_1, py::array voids_2) {
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

    py::array_t<int64_t> result(buf1.size);
    auto buf_result = result.request();

    const uint8_t* ptr1 = static_cast<const uint8_t*>(buf1.ptr);
    const uint8_t* ptr2 = static_cast<const uint8_t*>(buf2.ptr);
    int64_t* ptr_out = static_cast<int64_t*>(buf_result.ptr);

    size_t itemsize = buf1.itemsize;
    size_t n = buf1.size;
    size_t n64 = itemsize / 8;
    size_t tail = itemsize % 8;
    size_t total_64_chunks = n * n64;

    // if (total_64_chunks < 10000000) {
    //     // #pragma omp simd
    //     for (size_t i = 0; i < n; ++i) {
    //         const uint8_t* base1 = ptr1 + i * itemsize;
    //         const uint8_t* base2 = ptr2 + i * itemsize;
    //         int64_t count = 0;
    //         for (size_t k = 0; k < n64; ++k) {
    //             uint64_t w1, w2;
    //             std::memcpy(&w1, base1 + k * 8, 8);
    //             std::memcpy(&w2, base2 + k * 8, 8);
    //             count += __builtin_popcountll(w1 & w2);
    //         }
    //         for (size_t t = 0; t < tail; ++t) {
    //             count += __builtin_popcount( (base1[n64 * 8 + t] & base2[n64 * 8 + t]) );
    //         }
    //         ptr_out[i] = count;
    //     }
    // } else {
        #pragma omp parallel for if (total_64_chunks >= 10000000) schedule(static)
        for (size_t i = 0; i < n; ++i) {
            const uint8_t* base1 = ptr1 + i * itemsize;
            const uint8_t* base2 = ptr2 + i * itemsize;
            int64_t count = 0;
            for (size_t k = 0; k < n64; ++k) {
                uint64_t w1, w2;
                std::memcpy(&w1, base1 + k * 8, 8);
                std::memcpy(&w2, base2 + k * 8, 8);
                count += __builtin_popcountll(w1 & w2);
            }
            for (size_t t = 0; t < tail; ++t) {
                count += __builtin_popcount( (base1[n64 * 8 + t] & base2[n64 * 8 + t]) );
            }
            ptr_out[i] = count;
        }
    // }

    result.resize(buf1.shape);

    return result;
}

