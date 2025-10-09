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
#include <numeric>

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
    // auto buf_z1 = z1.request();
    // auto buf_x1 = x1.request();
    // auto buf_z2 = z2.request();
    // auto buf_x2 = x2.request();

    // std::vector<size_t> shape1(buf_z1.shape.begin(), buf_z1.shape.end());
    // std::vector<size_t> shape2(buf_z2.shape.begin(), buf_z2.shape.end());

    // if (shape1.size() != shape2.size()) {
    //     throw std::invalid_argument("Input arrays must have the same number of dimensions for concatenation.");
    // }
    // for (size_t i = 0; i < shape1.size() - 1; ++i) {
    //     if (shape1[i] != shape2[i]) {
    //         throw std::invalid_argument("Input arrays must have matching dimensions (except the last one).");
    //     }
    // }


    // std::vector<size_t> combined_shape = shape1;
    // combined_shape.back() += shape2.back();

    // py::array new_z(z1.dtype(), combined_shape);
    // py::array new_x(z1.dtype(), combined_shape);

    // auto buf_new_z = new_z.request();
    // auto buf_new_x = new_x.request();

    // void* ptr_new_z = buf_new_z.ptr;
    // void* ptr_new_x = buf_new_x.ptr;

    // const void* ptr_z1 = buf_z1.ptr;
    // const void* ptr_x1 = buf_x1.ptr;
    // const void* ptr_z2 = buf_z2.ptr;
    // const void* ptr_x2 = buf_x2.ptr;

    return py::make_tuple(1, 2);
}

py::tuple compose(py::array z1, py::array x1, py::array z2, py::array x2){
    return py::make_tuple(1, 2);
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

    #pragma omp parallel for if (n >= VOPS_THRESHOLD_PARALLEL) schedule(static)
    for (size_t i = 0; i < n; ++i) {
        ptr_result[i] = ptr1[i] == 0;
    }
    return result.reshape(z1.request().shape);


}


py::tuple random_zx_strings(const std::vector<size_t>& shape, size_t num_qubits) {
    size_t total_size = 1;
    for (size_t dim : shape) {
        total_size *= dim;
    }
    total_size *= num_qubits;

    std::vector<size_t> full_shape = shape;
    full_shape.push_back(num_qubits);

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

    for (size_t i = 0; i < total_size; ++i) {
        ptr_z[i] = dist(gen);
        ptr_x[i] = dist(gen);
    }

    return py::make_tuple(z_strings, x_strings);
}


py::object unique(py::array zx_voids, bool return_index = false, bool return_inverse = false, bool return_counts = false) {
    auto buf = zx_voids.request();
    if (buf.ndim == 0) {
        if (return_index || return_inverse || return_counts) {
            py::tuple t(1 + (int)return_index + (int)return_inverse + (int)return_counts);
            t[0] = zx_voids;
            int pos = 1;
            if (return_index) {
                py::array_t<int64_t> arr_idx(1);
                auto ib = arr_idx.request();
                int64_t* iptr = static_cast<int64_t*>(ib.ptr);
                iptr[0] = 0;
                t[pos++] = arr_idx;
            }
            if (return_inverse) {
                py::array_t<int64_t> arr_inv(1);
                auto rb = arr_inv.request();
                int64_t* rptr = static_cast<int64_t*>(rb.ptr);
                rptr[0] = 0;
                t[pos++] = arr_inv;
            }
            if (return_counts) {
                py::array_t<int64_t> arr_cnt(1);
                auto cb = arr_cnt.request();
                int64_t* cptr = static_cast<int64_t*>(cb.ptr);
                cptr[0] = 1;
                t[pos++] = arr_cnt;
            }
            return t;
        }
        return zx_voids;
    }

    size_t n = buf.size; // number of elements (
    size_t itemsize = buf.itemsize; // bytes per element
    auto ptr = static_cast<uint8_t*>(buf.ptr);

    std::vector<size_t> idx(n);
    std::iota(idx.begin(), idx.end(), 0);

    // comparator: lexicographic bytes
    auto cmp = [&](size_t a, size_t b) {
        const void* pa = ptr + a * itemsize;
        const void* pb = ptr + b * itemsize;
        int r = std::memcmp(pa, pb, itemsize);
        if (r != 0) return r < 0;
        return a < b;
    };

    std::sort(idx.begin(), idx.end(), cmp);

    std::vector<size_t> group_of(n);
    std::vector<size_t> representatives; representatives.reserve(n);
    std::vector<int64_t> counts; counts.reserve(n);

    size_t groups = 0;
    for (size_t sorted_pos = 0; sorted_pos < n; ++sorted_pos) {
        size_t cur = idx[sorted_pos];
        if (sorted_pos == 0) {
            representatives.push_back(cur);
            counts.push_back(1);
            group_of[cur] = 0;
            groups = 1;
        } else {
            size_t prev = idx[sorted_pos - 1];
            if (std::memcmp(ptr + prev * itemsize, ptr + cur * itemsize, itemsize) == 0) {
                // same group
                counts.back() += 1;
                group_of[cur] = groups - 1;
                if (cur < representatives.back()) representatives.back() = cur; 
            } else {
                // new group
                representatives.push_back(cur);
                counts.push_back(1);
                group_of[cur] = groups;
                groups += 1;
            }
        }
    }

    // Build unique array (sorted order = order of groups as encountered)
    std::vector<ssize_t> unique_shape = {(ssize_t)groups};
    py::array unique = py::array(zx_voids.dtype(), unique_shape);
    auto ubuf = unique.request();
    uint8_t* uptr = static_cast<uint8_t*>(ubuf.ptr);

    for (size_t gi = 0; gi < groups; ++gi) {
        size_t src = idx[ gi == 0 ? 0 : 0 ]; // placeholder to avoid compiler warning
        // find any element belonging to group gi -> we can use representatives[gi] (original index),
        // but we must copy the representative's bytes (they equal the group's bytes)
        size_t rep_orig_index = representatives[gi];
        std::memcpy(uptr + gi * itemsize, ptr + rep_orig_index * itemsize, itemsize);
    }

    // build optional outputs
    py::array_t<int64_t> py_indices;
    py::array_t<int64_t> py_inverse;
    py::array_t<int64_t> py_counts;

    if (return_index) {
        py_indices = py::array_t<int64_t>(groups);
        auto ib = py_indices.request();
        int64_t* iptr = static_cast<int64_t*>(ib.ptr);
        for (size_t gi = 0; gi < groups; ++gi) iptr[gi] = (int64_t)representatives[gi];
    }

    if (return_inverse) {
        py_inverse = py::array_t<int64_t>(n);
        auto rb = py_inverse.request();
        int64_t* rptr = static_cast<int64_t*>(rb.ptr);
        for (size_t orig = 0; orig < n; ++orig) rptr[orig] = (int64_t)group_of[orig];
    }

    if (return_counts) {
        py_counts = py::array_t<int64_t>(groups);
        auto cb = py_counts.request();
        int64_t* cptr = static_cast<int64_t*>(cb.ptr);
        for (size_t gi = 0; gi < groups; ++gi) cptr[gi] = counts[gi];
    }

    if (return_index || return_inverse || return_counts) {
        int ntuple = 1 + (int)return_index + (int)return_inverse + (int)return_counts;
        py::tuple ret(ntuple);
        int pos = 0;
        ret[pos++] = unique;
        if (return_index) ret[pos++] = py_indices;
        if (return_inverse) ret[pos++] = py_inverse;
        if (return_counts) ret[pos++] = py_counts;
        return ret;
    }
    return unique;
}