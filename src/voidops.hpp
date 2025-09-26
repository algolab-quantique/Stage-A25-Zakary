#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <cstdint> // uint8_t
#include <iostream>
#include <cstring> // For memcpy
#include <omp.h>

//Bon resultats
//TODO: rendre ca vite
py::array bitwise_and(py::array voids_1, py::array voids_2) {
    auto buf1 = voids_1.request();
    auto buf2 = voids_2.request();

    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize. Got " + std::to_string(buf1.itemsize) + " and " + std::to_string(buf2.itemsize));
    }

    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size. Got " + std::to_string(buf1.size) + " and " + std::to_string(buf2.size));
    }
    

    size_t total_bytes = buf1.size * buf1.itemsize;
    py::array voids_3 = py::array(voids_1.dtype(), buf1.shape);
    auto buf3 = voids_3.request();

    const uint64_t* ptr1_64 = static_cast<const uint64_t*>(buf1.ptr);
    const uint64_t* ptr2_64 = static_cast<const uint64_t*>(buf2.ptr);
    uint64_t* ptr3_64 = static_cast<uint64_t*>(buf3.ptr);


    size_t num_64bit_chunks = total_bytes / 8;

    if (num_64bit_chunks < 10000000) { 
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            ptr3_64[i] = ptr1_64[i] & ptr2_64[i];
        }
    } else {
        #pragma omp parallel for
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            ptr3_64[i] = ptr1_64[i] & ptr2_64[i];
        }
    }

    // for (size_t i = 0; i < num_64bit_chunks; ++i) {
    //     ptr3_64[i] = ptr1_64[i] & ptr2_64[i];
    // }


    size_t remaining_bytes = total_bytes % 8;
    if (remaining_bytes > 0) {
        const uint8_t* ptr1_8 = static_cast<const uint8_t*>(buf1.ptr);
        const uint8_t* ptr2_8 = static_cast<const uint8_t*>(buf2.ptr);
        uint8_t* ptr3_8 = static_cast<uint8_t*>(buf3.ptr);
        
        size_t start_byte = num_64bit_chunks * 8;
        for (size_t i = 0; i < remaining_bytes; ++i) {
            ptr3_8[start_byte + i] = ptr1_8[start_byte + i] & ptr2_8[start_byte + i];
        }
    }

    return voids_3;
}


// def bitwise_not(voids: NDArray) -> NDArray:

//     int_strings = voids_to_int_strings(voids)
//     new_int_strings = np.invert(int_strings)
//     new_voids = int_strings_to_voids(new_int_strings)

//     return new_voids
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


py::array bitwise_xor(py::array voids_1, py::array voids_2) {
    auto buf1 = voids_1.request();
    auto buf2 = voids_2.request();

    // Check for matching itemsize and shape
    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize. Got " +
            std::to_string(buf1.itemsize) + " and " + std::to_string(buf2.itemsize));
    }
    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size. Got " +
            std::to_string(buf1.size) + " and " + std::to_string(buf2.size));
    }

    size_t total_bytes = buf1.size * buf1.itemsize;
    py::array result = py::array(voids_1.dtype(), buf1.shape);
    auto buf3 = result.request();

    const uint64_t* ptr1_64 = static_cast<const uint64_t*>(buf1.ptr);
    const uint64_t* ptr2_64 = static_cast<const uint64_t*>(buf2.ptr);
    uint64_t* ptr3_64 = static_cast<uint64_t*>(buf3.ptr);

    size_t num_64bit_chunks = total_bytes / 8;

    if (num_64bit_chunks < 10000000) { 
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            ptr3_64[i] = ptr1_64[i] ^ ptr2_64[i];
        }
    } else {
        #pragma omp parallel for
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            ptr3_64[i] = ptr1_64[i] ^ ptr2_64[i];
        }
    }

    size_t remaining_bytes = total_bytes % 8;
    if (remaining_bytes > 0) {
        const uint8_t* ptr1_8 = static_cast<const uint8_t*>(buf1.ptr);
        const uint8_t* ptr2_8 = static_cast<const uint8_t*>(buf2.ptr);
        uint8_t* ptr3_8 = static_cast<uint8_t*>(buf3.ptr);
        
        size_t start_byte = num_64bit_chunks * 8;
        for (size_t i = 0; i < remaining_bytes; ++i) {
            ptr3_8[start_byte + i] = ptr1_8[start_byte + i] ^ ptr2_8[start_byte + i];
        }
    }

    return result;
}


py::array bitwise_or(py::array voids_1, py::array voids_2) {
    auto buf1 = voids_1.request();
    auto buf2 = voids_2.request();

    if (buf1.itemsize != buf2.itemsize) {
        throw std::runtime_error("Input arrays must have the same itemsize. Got " + std::to_string(buf1.itemsize) + " and " + std::to_string(buf2.itemsize));
    }

    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size. Got " + std::to_string(buf1.size) + " and " + std::to_string(buf2.size));
    }
    

    size_t total_bytes = buf1.size * buf1.itemsize;
    py::array voids_3 = py::array(voids_1.dtype(), buf1.shape);
    auto buf3 = voids_3.request();

    const uint64_t* ptr1_64 = static_cast<const uint64_t*>(buf1.ptr);
    const uint64_t* ptr2_64 = static_cast<const uint64_t*>(buf2.ptr);
    uint64_t* ptr3_64 = static_cast<uint64_t*>(buf3.ptr);


    size_t num_64bit_chunks = total_bytes / 8;

    if (num_64bit_chunks < 10000000) { 
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            ptr3_64[i] = ptr1_64[i] | ptr2_64[i];
        }
    } else {
        #pragma omp parallel for
        for (size_t i = 0; i < num_64bit_chunks; ++i) {
            ptr3_64[i] = ptr1_64[i] | ptr2_64[i];
        }
    }

    size_t remaining_bytes = total_bytes % 8;
    if (remaining_bytes > 0) {
        const uint8_t* ptr1_8 = static_cast<const uint8_t*>(buf1.ptr);
        const uint8_t* ptr2_8 = static_cast<const uint8_t*>(buf2.ptr);
        uint8_t* ptr3_8 = static_cast<uint8_t*>(buf3.ptr);
        
        size_t start_byte = num_64bit_chunks * 8;
        for (size_t i = 0; i < remaining_bytes; ++i) {
            ptr3_8[start_byte + i] = ptr1_8[start_byte + i] | ptr2_8[start_byte + i];
        }
    }
    

    return voids_3;
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

    #pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
        int64_t count = 0;
        for (size_t j = 0; j < itemsize; ++j) {
            count += __builtin_popcount(ptr1[i * itemsize + j]);
        }
        ptr_out[i] = count;
    }

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

    #pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
        int64_t count = 0;
        for (size_t j = 0; j < itemsize; ++j) {
            uint8_t v = ptr1[i * itemsize + j] & ptr2[i * itemsize + j];
            count += __builtin_popcount(v);
        }
        ptr_out[i] = count;
    }

    result.resize(buf1.shape);

    return result;
}