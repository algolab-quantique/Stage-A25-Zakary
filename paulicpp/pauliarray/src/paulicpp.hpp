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

#ifdef USE_OPENMP
#include <omp.h>
#else
#warning "OpenMP is not enabled"
#endif

#include "voidops.hpp"

#define FUNC_THRESHOLD_PARALLEL 100000

//  new_z_strings = np.concatenate((self.z_strings, other.z_strings), axis=-1)
// new_x_strings = np.concatenate((self.x_strings, other.x_strings), axis=-1)
// todo: this aint working
py::tuple tensor(py::array z2, py::array x2, py::array z1, py::array x1) {
    auto buf_z1 = z1.request();
    auto buf_x1 = x1.request();
    auto buf_z2 = z2.request();
    auto buf_x2 = x2.request();

    std::vector<ssize_t> new_shape = buf_z1.shape;
    new_shape.back() += buf_z2.shape.back();
    py::array new_z = py::array(z1.dtype(), new_shape);
    py::array new_x = py::array(x1.dtype(), new_shape);
    auto buf_new_z = new_z.request();
    auto buf_new_x = new_x.request();

    const uint8_t *z1_ptr = static_cast<const uint8_t *>(buf_z1.ptr);
    const uint8_t *z2_ptr = static_cast<const uint8_t *>(buf_z2.ptr);
    const uint8_t *x1_ptr = static_cast<const uint8_t *>(buf_x1.ptr);
    const uint8_t *x2_ptr = static_cast<const uint8_t *>(buf_x2.ptr);
    uint8_t *new_z_ptr = static_cast<uint8_t *>(buf_new_z.ptr);
    uint8_t *new_x_ptr = static_cast<uint8_t *>(buf_new_x.ptr);

    ssize_t n1 = buf_z1.shape.back();
    ssize_t n2 = buf_z2.shape.back();
    ssize_t outer = buf_z1.size / n1;
    size_t itemsize = buf_z1.itemsize;

    // use byte offsets to be safe when itemsize != 1
    size_t row_bytes_new = static_cast<size_t>(n1 + n2) * itemsize;
    size_t row_bytes1 = static_cast<size_t>(n1) * itemsize;
    size_t row_bytes2 = static_cast<size_t>(n2) * itemsize;

    for (ssize_t j = 0; j < outer; ++j) {
        size_t dst_row_byte = static_cast<size_t>(j) * row_bytes_new;
        size_t src1_row_byte = static_cast<size_t>(j) * row_bytes1;
        size_t src2_row_byte = static_cast<size_t>(j) * row_bytes2;

        std::memcpy(new_z_ptr + dst_row_byte, z1_ptr + src1_row_byte, row_bytes1);
        std::memcpy(new_z_ptr + dst_row_byte + row_bytes1, z2_ptr + src2_row_byte, row_bytes2);
        std::memcpy(new_x_ptr + dst_row_byte, x1_ptr + src1_row_byte, row_bytes1);
        std::memcpy(new_x_ptr + dst_row_byte + row_bytes1, x2_ptr + src2_row_byte, row_bytes2);
    }

    return py::make_tuple(new_z, new_x);
}

/*
    assert self.num_qubits == other.num_qubits
    assert is_broadcastable(self.shape, other.shape)

    new_z_voids = vops.bitwise_xor(self.z_voids, other.z_voids)
    new_x_voids = vops.bitwise_xor(self.x_voids, other.x_voids)

    self_phase_power = vops.bitwise_dot(self.z_voids, self.x_voids)
    other_phase_power = vops.bitwise_dot(other.z_voids, other.x_voids)
    new_phase_power = vops.bitwise_dot(new_z_voids, new_x_voids)
    commutation_phase_power = 2 * vops.bitwise_dot(self.x_voids, other.z_voids)

    phase_power = commutation_phase_power + self_phase_power + other_phase_power - new_phase_power

    phases = np.choose(phase_power, [1, -1j, -1, 1j], mode="wrap")

    return PauliArray(new_z_voids, new_x_voids, self.num_qubits), phases
*/
py::tuple compose(py::array z1, py::array x1, py::array z2, py::array x2) {
    py::array new_z = bitwise_xor(z1, z2);
    py::array new_x = bitwise_xor(x1, x2);

    py::array self_phase_power = bitwise_dot(z1, x1);
    py::array other_phase_power = bitwise_dot(z2, x2);
    py::array new_phase_power = bitwise_dot(new_z, new_x);
    py::array commutation_phase_power = bitwise_dot(x1, z2);

    // phase_power = commutation_phase_power + self_phase_power + other_phase_power -
    // new_phase_power
    py::array_t<std::complex<double>> phase_power =
        py::array_t<std::complex<double>>(new_z.request().shape);

    auto buf_comm = commutation_phase_power.request();
    auto buf_self = self_phase_power.request();
    auto buf_other = other_phase_power.request();
    auto buf_new = new_phase_power.request();
    auto buf_phase = phase_power.request();

    auto n = buf_phase.size;
    const int64_t *ptr_comm = static_cast<const int64_t *>(buf_comm.ptr);
    const int64_t *ptr_self = static_cast<const int64_t *>(buf_self.ptr);
    const int64_t *ptr_other = static_cast<const int64_t *>(buf_other.ptr);
    const int64_t *ptr_new = static_cast<const int64_t *>(buf_new.ptr);
    std::complex<double> *ptr_phase = static_cast<std::complex<double> *>(buf_phase.ptr);

#ifdef USE_OPENMP
#pragma omp parallel for if (n >= FUNC_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (ssize_t i = 0; i < n; ++i) {
        uint8_t tmp = (ptr_comm[i] * 2 + ptr_self[i] + ptr_other[i] - ptr_new[i]) % 4;
        switch (tmp) {
        case 0:
            ptr_phase[i] = std::complex<double>(1.0, 0.0);
            break;
        case 1:
            ptr_phase[i] = std::complex<double>(0.0, -1.0);
            break;
        case 2:
            ptr_phase[i] = std::complex<double>(-1.0, 0.0);
            break;
        case 3:
            ptr_phase[i] = std::complex<double>(0.0, 1.0);
            break;
        }
    }
    return py::make_tuple(new_z, new_x, phase_power);
}

py::array_t<bool> bitwise_commute_with(py::array z1, py::array x1, py::array z2, py::array x2) {
    py::array ovlp_1 = bitwise_and(z1, x2);
    py::array ovlp_2 = bitwise_and(x1, z2);
    py::array ovlp_3 = bitwise_xor(ovlp_1, ovlp_2);

    size_t num_qubits = ovlp_3.request().size;
    py::array_t<bool> result = py::array_t<bool>(num_qubits);
    auto buf_result = result.request();
    auto ptr_result = static_cast<bool *>(buf_result.ptr);

    auto buf1 = ovlp_3.request();
    const uint8_t *ptr1 = static_cast<const uint8_t *>(buf1.ptr);
    size_t n = buf1.size;

#ifdef USE_OPENMP
#pragma omp parallel for if (n >= VOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t i = 0; i < n; ++i) {
        ptr_result[i] = ptr1[i] == 0;
    }
    return result.reshape(z1.request().shape);
}

py::tuple random_zx_strings(const std::vector<size_t> &shape) { //, size_t num_qubits) {
    size_t total_size = 1;
    for (size_t dim : shape) {
        total_size *= dim;
    }
    // total_size *= num_qubits;

    std::vector<size_t> full_shape = shape;
    // full_shape.push_back(num_qubits);

    py::array_t<bool> z_strings(full_shape);
    py::array_t<bool> x_strings(full_shape);

    auto buf_z = z_strings.request();
    auto buf_x = x_strings.request();

    bool *ptr_z = static_cast<bool *>(buf_z.ptr);
    bool *ptr_x = static_cast<bool *>(buf_x.ptr);

    // Random number generation
    std::random_device rd;
    // std::mt19937 gen(rd());
    std::minstd_rand gen(rd());
    std::uniform_int_distribution<int> dist(0, 1);

    for (size_t i = 0; i < total_size; ++i) {
        ptr_z[i] = dist(gen);
        ptr_x[i] = dist(gen);
    }

    return py::make_tuple(z_strings, x_strings);
}

py::object unique(py::array zx_voids, bool return_index = false, bool return_inverse = false,
                  bool return_counts = false) {
    auto buf = zx_voids.request();
    if (buf.ndim == 0) {
        if (return_index || return_inverse || return_counts) {
            py::tuple t(1 + (int)return_index + (int)return_inverse + (int)return_counts);
            t[0] = zx_voids;
            int pos = 1;
            if (return_index) {
                py::array_t<int64_t> arr_idx(1);
                auto ib = arr_idx.request();
                int64_t *iptr = static_cast<int64_t *>(ib.ptr);
                iptr[0] = 0;
                t[pos++] = arr_idx;
            }
            if (return_inverse) {
                py::array_t<int64_t> arr_inv(1);
                auto rb = arr_inv.request();
                int64_t *rptr = static_cast<int64_t *>(rb.ptr);
                rptr[0] = 0;
                t[pos++] = arr_inv;
            }
            if (return_counts) {
                py::array_t<int64_t> arr_cnt(1);
                auto cb = arr_cnt.request();
                int64_t *cptr = static_cast<int64_t *>(cb.ptr);
                cptr[0] = 1;
                t[pos++] = arr_cnt;
            }
            return t;
        }
        return zx_voids;
    }

    size_t n = buf.size;            // number of elements (
    size_t itemsize = buf.itemsize; // bytes per element
    auto ptr = static_cast<uint8_t *>(buf.ptr);

    std::vector<size_t> idx(n);
    std::iota(idx.begin(), idx.end(), 0);

    // comparator: lexicographic bytes
    auto cmp = [&](size_t a, size_t b) {
        const void *pa = ptr + a * itemsize;
        const void *pb = ptr + b * itemsize;
        int r = std::memcmp(pa, pb, itemsize);
        if (r != 0)
            return r < 0;
        return a < b;
    };

    std::sort(idx.begin(), idx.end(), cmp);

    std::vector<size_t> group_of(n);
    std::vector<size_t> representatives;
    representatives.reserve(n);
    std::vector<int64_t> counts;
    counts.reserve(n);

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
                if (cur < representatives.back())
                    representatives.back() = cur;
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
    uint8_t *uptr = static_cast<uint8_t *>(ubuf.ptr);

    for (size_t gi = 0; gi < groups; ++gi) {
        size_t src = idx[gi == 0 ? 0 : 0]; // placeholder to avoid compiler warning
        // find any element belonging to group gi -> we can use representatives[gi]
        // (original index), but we must copy the representative's bytes (they equal
        // the group's bytes)
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
        int64_t *iptr = static_cast<int64_t *>(ib.ptr);
        for (size_t gi = 0; gi < groups; ++gi)
            iptr[gi] = (int64_t)representatives[gi];
    }

    if (return_inverse) {
        py_inverse = py::array_t<int64_t>(n);
        auto rb = py_inverse.request();
        int64_t *rptr = static_cast<int64_t *>(rb.ptr);
        for (size_t orig = 0; orig < n; ++orig)
            rptr[orig] = (int64_t)group_of[orig];
    }

    if (return_counts) {
        py_counts = py::array_t<int64_t>(groups);
        auto cb = py_counts.request();
        int64_t *cptr = static_cast<int64_t *>(cb.ptr);
        for (size_t gi = 0; gi < groups; ++gi)
            cptr[gi] = counts[gi];
    }

    if (return_index || return_inverse || return_counts) {
        int ntuple = 1 + (int)return_index + (int)return_inverse + (int)return_counts;
        py::tuple ret(ntuple);
        int pos = 0;
        ret[pos++] = unique;
        if (return_index)
            ret[pos++] = py_indices;
        if (return_inverse)
            ret[pos++] = py_inverse;
        if (return_counts)
            ret[pos++] = py_counts;
        return ret;
    }
    return unique;
}

// #include "xxhash/xxhash.h"
// #include "xxhash.h"

// struct XXH3StringHash {
//     size_t operator()(const std::string& s) const noexcept {
//         return static_cast<size_t>(XXH3_64bits(s.data(), s.size()));
//     }
// };

/**
 * @brief This is a very early test implementation of unordered unique.
 * It finds unique rows in a NumPy 2D array by mapping the zx_voids to a hashmap.
 * Thus, two identical rows will be encoded to the same key via the hashing function and ensures
 * a fast execution time.
 *
 * This function is heavily inspired by Qiskit's Rust function of the same name.
 * https://github.com/Qiskit/qiskit/blob/main/crates/quantum_info/src/sparse_pauli_op.rs#L54
 *
 * @attention Does not work for higher dimensions.
 * @param zx_voids Both Z and X voids stiched together
 * @return py::tuple Returns (indices, inverse).
 * Indices gives the index of each unique row from zx_voids.
 * Inverse is the indices to remake the input array from only its unique elements.
 */
py::tuple unordered_unique(py::array zx_voids) {
    // TODO: Make this work for higher dimensions
    // TODO: Use a better hash function like XXH3 from xxhash

    auto buf = zx_voids.request();
    // TODO: Fix this cause its not working ! Ahah!!
    if (buf.ndim == 0) {
        py::array_t<int64_t> idx(1);
        py::array_t<int64_t> inv(1);
        auto ib = idx.request();
        auto rb = inv.request();
        int64_t *ip = static_cast<int64_t *>(ib.ptr);
        int64_t *rp = static_cast<int64_t *>(rb.ptr);
        ip[0] = 0;
        rp[0] = 0;
        return py::make_tuple(idx, inv);
    }

    const uint8_t *base = static_cast<const uint8_t *>(buf.ptr);
    const size_t nrows = static_cast<size_t>(buf.shape[0]);
    const size_t row_bytes = (buf.ndim > 1) ? static_cast<size_t>(std::llabs(buf.strides[0]))
                                            : static_cast<size_t>(buf.itemsize);

    std::vector<std::string_view> keys;
    keys.resize(nrows);

    // these need to be out of the GIL scope to survive the release
    std::vector<size_t> indices;
    indices.reserve(nrows);
    std::vector<size_t> inverses(nrows);

    {
        // dont really know if releasing the GIL helps here but whatever
        py::gil_scoped_release release;

#ifdef USE_OPENMP
#pragma omp parallel for if (nrows >= FUNC_THRESHOLD_PARALLEL) schedule(static)
#endif
        for (size_t i = 0; i < nrows; ++i) {
            const char *ptr = reinterpret_cast<const char *>(base + i * row_bytes);
            keys[i] = std::string_view(ptr, row_bytes);
        }

        std::unordered_map<std::string_view, size_t> table;
        // std::unordered_map<std::string, size_t, XXH3StringHash> table;
        table.reserve(nrows);

        for (size_t i = 0; i < nrows; ++i) {
            const auto &key = keys[i];
            auto it = table.find(key);
            if (it != table.end()) {
                inverses[i] = it->second;
            } else {
                size_t new_id = indices.size();
                table.emplace(key, new_id);
                indices.push_back(i);
                inverses[i] = new_id;
            }
        }
    } // GIL reacquired here

    // Build numpy outputs
    py::array_t<int64_t> py_indices(indices.size());
    auto ib = py_indices.request();
    int64_t *iptr = static_cast<int64_t *>(ib.ptr);
    for (size_t k = 0; k < indices.size(); ++k)
        iptr[k] = static_cast<int64_t>(indices[k]);

    py::array_t<int64_t> py_inverse(nrows);
    auto rb = py_inverse.request();
    int64_t *rptr = static_cast<int64_t *>(rb.ptr);
    for (size_t i = 0; i < nrows; ++i)
        rptr[i] = static_cast<int64_t>(inverses[i]);

    return py::make_tuple(py_indices, py_inverse);
}
