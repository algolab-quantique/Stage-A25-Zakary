/**
 * @file cz2m.hpp
 * @author Zakary Romdhane (zakary.romdhane@usherbrooke.ca)
 * @brief
 *
 * @attention Your compiler must support at least C++20 standard to properly compile this file.
 *
 * @todo Add SIMD support,
 * to_matrix,
 * inserer des matrices dans dautres via indexes,
 * get_qubit_slices (?) - return toutes qubits pour un ensemble dindexes via vector<int>
 *
 * @note It is assumed that all of the input arrays are contiguous, of the dtype |V{N}, and the
 * exact same shape. No checks are performed to ensure this is the case. Any broadcasting and
 * contiguity checks MUST be performed in Python before calling any of these functions. Any
 * computers which are not 64-bit architectures will lead to undefined behavior due to the casting
 * to uint64_t*.
 *
 * @version 0.1.1
 * @date 2025-10-01
 *
 * @copyright Copyright 2025 Zakary Romdhane
 */

#include "cz2m.h"

/**
 * @brief This function performs a 'tensor product' between two arrays of Pauli operators
 * In practice this is just a concatenation of the underlying z and x arrays.
 * @deprecated Use concatenate() instead, as it is generic and not aimed only for Pauli ops.
 *
 * @param z2
 * @param x2
 * @param z1
 * @param x1
 * @return py::tuple
 */
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

    // TODO: ==================== Fix this - prob a pointer issue ====================
    // a = XY, b = ZZ
    // a.tensor(b) = IIXY... Should be ZZXY !!
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

/**
 * @brief Compose two arrays of Pauli operators.
 *
 * @warning Current implementation is very naive, error prone and probably slow.
 * Best to re-implement in a better way.
 *
 * @param z1
 * @param x1
 * @param z2
 * @param x2
 * @return py::tuple Returns a tuple of (new_z, new_x, phase_power). The new_z and new_x
 * are the composed Pauli operators, and phase_power is a complex array of dtype complex128
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

/**
 * @brief Operates on two arrays of Pauli operators and returns a boolean vector indicating which
 * qubits commute between the two arrays.
 *
 * @param z1
 * @param x1
 * @param z2
 * @param x2
 * @return py::array_t<bool>
 */
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
    #pragma omp parallel for if (n >= BOPS_THRESHOLD_PARALLEL) schedule(static)
#endif
    for (size_t i = 0; i < n; ++i) {
        ptr_result[i] = ptr1[i] == 0;
    }
    return result.reshape(z1.request().shape);
}

/**
 * @brief Generates random Z and X strings of given shape.
 * @attention This function exists mainly for testing purposes.
 *
 * @param shape
 * @return py::tuple Returns a tuple of (z_strings, x_strings), which are both
 * boolean arrays of the given shape (not Z2Rs)
 */
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

/**
 * @brief Finds the unique rows in a Z2R array. Functions similarly to numpy.unique, but with
 * no axis parameter.
 * @deprecated Use unordered_unique() instead for better performance (3x-15x) on the vast majority
 * of use cases.
 *
 * @param z2r
 * @param return_index
 * @param return_inverse
 * @param return_counts
 * @return py::object
 */
py::object unique(py::array z2r, bool return_index, bool return_inverse, bool return_counts) {
    auto buf = z2r.request();
    if (buf.ndim == 0) {
        if (return_index || return_inverse || return_counts) {
            py::tuple t(1 + (int)return_index + (int)return_inverse + (int)return_counts);
            t[0] = z2r;
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
        return z2r;
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
        int row = std::memcmp(pa, pb, itemsize);
        if (row != 0)
            return row < 0;
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
    py::array unique = py::array(z2r.dtype(), unique_shape);
    auto ubuf = unique.request();
    uint8_t *uptr = static_cast<uint8_t *>(ubuf.ptr);

    for (size_t gi = 0; gi < groups; ++gi) {
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

// TODO: See if xxhash is faster than std::hash for this use case
//  #include "xxhash/xxhash.h"
//  struct XXH3StringViewHash {
//      size_t operator()(const std::string_view& s) const noexcept {
//          return static_cast<size_t>(XXH3_64bits(s.data(), s.size()));
//      }
//  };

/**
 * @brief This function finds unique rows in a NumPy 2D array by mapping the z2r to a hashmap.
 * Thus, two identical rows will be encoded to the same key via the hashing function and ensures
 * a fast execution time.
 *
 * @attention Does not work for higher dimensions.
 * @param z2r Both Z and X voids stiched together
 * @return py::tuple Returns (indices, inverse).
 * Indices gives the index of each unique row from z2r.
 * Inverse is the indices to remake the input array from only its unique elements.
 */
py::tuple unordered_unique(py::array z2r) {

    auto buf = z2r.request();
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
        py::gil_scoped_release release;

#ifdef USE_OPENMP
    #pragma omp parallel for if (nrows >= FUNC_THRESHOLD_PARALLEL) schedule(static)
#endif
        for (size_t i = 0; i < nrows; ++i) {
            const char *ptr = reinterpret_cast<const char *>(base + i * row_bytes);
            keys[i] = std::string_view(ptr, row_bytes);
        }

        std::unordered_map<std::string_view, size_t> table;
        // std::unordered_map seems to be poor in performance (according to the internet)
        // Perhaps we should be using another container implementation?
        // See: https://martin.ankerl.com/2022/08/27/hashmap-bench-01/
        table.max_load_factor(0.5);
        // std::unordered_map<std::string_view, size_t, XXH3StringViewHash> table;
        table.reserve(nrows);

        for (size_t i = 0; i < nrows; ++i) {
            const auto &key = keys[i];
            auto it = table.find(key);
            if (it != table.end()) {
                inverses[i] = it->second;
            } else {
                size_t new_id = indices.size();
                table.try_emplace(key, new_id);
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

/**
 * @brief applies Gauss-Jordan elimination on a binary matrix to produce row echelon form.
 * @todo Optimize this function!
 *
 * @param voids
 * @param num_qubits
 * @return py::array
 */
py::array row_echelon(py::array voids, int num_qubits) {
    auto buf = voids.request();

    size_t n_rows = buf.size;
    size_t n_cols = num_qubits;

    py::array voids_out = py::array(voids.dtype(), buf.shape);
    auto buf_out = voids_out.request();

    const uint8_t *ptr_in = std::bit_cast<const uint8_t *>(buf.ptr);
    uint8_t *ptr_out = std::bit_cast<uint8_t *>(buf_out.ptr);

    // Copy input to output
    std::memcpy(ptr_out, ptr_in, buf.size * buf.itemsize);

    size_t h_row = 0;
    size_t k_col = 0;

    //     while h_row < n_rows and k_col < n_cols:
    //         if np.all(re_bit_matrix[h_row:, k_col] == 0):
    //             k_col += 1
    //         else:
    //             i_row = h_row + np.argmax(re_bit_matrix[h_row:, k_col])
    //             if i_row != h_row:
    //                 re_bit_matrix[[i_row, h_row], :] = re_bit_matrix[[h_row, i_row], :]

    //             cond_rows = np.logical_and(re_bit_matrix[:, k_col], (row_range != h_row))

    //             re_bit_matrix[cond_rows, :] = np.logical_xor(re_bit_matrix[cond_rows, :],
    //             re_bit_matrix[h_row, :][None, :])

    //             h_row += 1
    //             k_col += 1

    //     return re_bit_matrix

    // TODO: Optimize!
    while (h_row < n_rows && k_col < n_cols) {
        int found_nonzero = 0;
        for (size_t row = h_row; row < n_rows; row++) {
            size_t byte_idx = k_col / 8;
            size_t bit_idx = k_col % 8;
            uint8_t bit = (ptr_out[row * buf.itemsize + byte_idx] >> bit_idx) & 1;
            if (bit) {
                found_nonzero = 1;
                break;
            }
        }
        if (!found_nonzero) {
            k_col += 1;
        } else {
            // Find pivot row
            size_t i_row = h_row;
            for (size_t row = h_row; row < n_rows; row++) {
                size_t byte_idx = k_col / 8;
                size_t bit_idx = k_col % 8;
                uint8_t bit = (ptr_out[row * buf.itemsize + byte_idx] >> bit_idx) & 1;
                if (bit) {
                    i_row = row;
                    break;
                }
            }
            // Swap rows if needed
            if (i_row != h_row) {
                for (size_t b = 0; b < buf.itemsize; b++) {
                    std::swap(ptr_out[i_row * buf.itemsize + b], ptr_out[h_row * buf.itemsize + b]);
                }
            }

            // Eliminate other rows
            for (size_t row = 0; row < n_rows; row++) {
                if (row != h_row) {
                    size_t byte_idx = k_col / 8;
                    size_t bit_idx = k_col % 8;
                    uint8_t bit = (ptr_out[row * buf.itemsize + byte_idx] >> bit_idx) & 1;
                    if (bit) {
                        // XOR rows
                        for (size_t b = 0; b < buf.itemsize; b++) {
                            ptr_out[row * buf.itemsize + b] ^= ptr_out[h_row * buf.itemsize + b];
                        }
                    }
                }
            }
            h_row++;
            k_col++;
        }
    }

    return voids_out;
}

/**
 * @brief Generates a sparse matrix representation from two Z2Rs.
 * @attention This function is mainly for testing purposes. Errors are to be expected.
 *
 * @param z_voids
 * @param x_voids
 * @param num_qubits
 * @return std::tuple<std::vector<int>, std::vector<int>, std::vector<std::complex<double>>>
 * Returns (row_ind, col_ind, matrix_elements)
 */
std::tuple<std::vector<int>, std::vector<int>, std::vector<std::complex<double>>>
sparse_matrix_from_z2r(py::array z_voids, py::array x_voids, int num_qubits) {
    auto buf_z = z_voids.request();
    auto buf_x = x_voids.request();

    size_t dim = 1 << num_qubits; // this is equivalent to 2**num_qubits
    std::vector<int> row_ind(dim);
    std::vector<int> col_ind(dim);
    std::vector<std::complex<double>> matrix_elements(dim);

    size_t itemsize = buf_z.itemsize;

    uint8_t *z_ptr = std::bit_cast<uint8_t *>(buf_z.ptr);
    uint8_t *x_ptr = std::bit_cast<uint8_t *>(buf_x.ptr);

    for (size_t i = 0; i < dim; ++i) {
        row_ind[i] = i;
        // col_ind = row_ind XOR x_int
        int col = 0;
        for (size_t byte_idx = 0; byte_idx < itemsize; ++byte_idx) {
            col |= ((x_ptr[byte_idx] & (1 << (i % 8))) ? (1 << (byte_idx * 8 + (i % 8))) : 0);
        }
        col_ind[i] = i ^ col;

        // matrix_elements = 1 - 2 * (popcount(row_ind & z_int) % 2)
        int popcount = 0;
        for (size_t byte_idx = 0; byte_idx < itemsize; ++byte_idx) {
            uint8_t and_result = row_ind[i] & z_ptr[byte_idx];
            popcount += std::popcount(and_result);
        }
        matrix_elements[i] =
            (popcount % 2 == 0) ? std::complex<double>(1.0, 0.0) : std::complex<double>(-1.0, 0.0);
    }
    std::cout << "Col size: " << col_ind.size() << std::endl;
    std::cout << "Row size: " << row_ind.size() << std::endl;
    std::cout << "Matrix elements size: " << matrix_elements.size() << std::endl;

    return std::make_tuple(row_ind, col_ind, matrix_elements);
}

/**
 * @brief Get the phases from two Z2R arrays.
 *
 * @deprecated This function is not used anymore.
 *
 * @param z_voids
 * @param x_voids
 * @return std::vector<std::complex<double>>
 */
std::vector<std::complex<double>> get_phases(py::array z_voids, py::array x_voids) {
    auto buf_z = z_voids.request();
    auto buf_x = x_voids.request();

    size_t n = buf_z.size;

    std::vector<std::complex<double>> phases(n);

    // const uint8_t *z_ptr = std::bit_cast<const uint8_t *>(buf_z.ptr);
    // const uint8_t *x_ptr = std::bit_cast<const uint8_t *>(buf_x.ptr);

    return phases;
}

// def to_matrix(self) -> NDArray:
//         """
//         Converts the Operator into a (n**2, n**2) matrix.

//         Returns:
//             NDArray: The matrix representation of the Operator.
//         """
//         # This method does not use WeightedPauliArray.to_matrices() for performance.

//         mat_shape = (2**self.num_qubits, 2**self.num_qubits)

//         z_ints = bitops.strings_to_ints(self.paulis.z_strings)
//         x_ints = bitops.strings_to_ints(self.paulis.x_strings)

//? what is this type
//         phase_powers = np.mod(bitops.dot(self.paulis.z_strings, self.paulis.x_strings), 4)
//         phases = np.choose(phase_powers, [1, -1j, -1, 1j])

//         matrix = np.zeros(mat_shape, dtype=complex)
//         for idx in np.ndindex(self.wpaulis.shape):
//             row_ind, col_ind, matrix_elements = pa.PauliArray.sparse_matrix_from_zx_ints(
//                 z_ints[idx], x_ints[idx], self.num_qubits
//             )
//? self.weights?

//             matrix[row_ind, col_ind] += self.weights[idx] * phases[idx] * matrix_elements

//         return matrix
// todo: Check return type of operator::to_matrix()
/**
 * @brief Converts two arrays of Z2Rs into a dense matrix representation.
 *
 * @attention This function is mainly for testing purposes.
 *
 * @param z_voids
 * @param x_voids
 * @return py::array_t<std::complex<double>>
 */
py::array_t<std::complex<double>> to_matrix(py::array z_voids, py::array x_voids, int num_qubits) {
    auto buf_z = z_voids.request();
    auto buf_x = x_voids.request();

    size_t n_rows = buf_z.shape[0];
    size_t n_cols = num_qubits;

    py::array_t<std::complex<double>> matrix(
        {static_cast<ssize_t>(1 << n_cols), static_cast<ssize_t>(1 << n_cols)});
    auto buf_mat = matrix.request();
    auto ptr_mat = static_cast<std::complex<double> *>(buf_mat.ptr);

    std::memset(buf_mat.ptr, 0, buf_mat.size * buf_mat.itemsize);

    for (size_t idx = 0; idx < n_rows; ++idx) {
        py::array z_row = z_voids[py::int_(idx)];
        py::array x_row = x_voids[py::int_(idx)];

        auto [row_ind, col_ind, matrix_elements] = sparse_matrix_from_z2r(z_row, x_row, num_qubits);

        for (size_t k = 0; k < row_ind.size(); ++k) {
            size_t row = row_ind[k];
            size_t c = col_ind[k];
            ptr_mat[row * (1 << n_cols) + c] += matrix_elements[k];
        }
    }

    return matrix;
}

//  def sparse_matrix_from_zx_ints(z_int: int, x_int: int, num_qubits: int) -> Tuple[NDArray,
//  NDArray, NDArray]:
//         """
//         Builds the matrix representing the Pauli String encoded in a sparse notation.

//         Args:
//             z_int (int): Integer which binary representation defines the z part of a Pauli
//             String. x_int (int): Integer which binary representation defines the x part of a
//             Pauli String. num_qubits (int): Length of the Pauli String.

//         Returns:
//             row_ind (NDArray): The row indices of returned matrix elements.
//             col_ind (NDArray): The column indices of returned matrix elements.
//             matrix_elements (NDArray): The matrix elements.
//         """
//         dim = 2**num_qubits

//         row_ind = np.arange(dim, dtype=np.uint64)
//         col_ind = np.bitwise_xor(row_ind, x_int)

//         matrix_elements = 1 - 2 * (np.bitwise_count(np.bitwise_and(row_ind,
//         z_int)).astype(np.float64) % 2)

//         return row_ind, col_ind, matrix_elements

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
py::array transpose(py::array voids, int64_t num_bits) {
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
py::array matmul(py::array z2r_a, py::array z2r_b, int a_num_qubits, int b_num_qubits) {
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

    // TODO: Parallelize this whole block
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

/**
 * @brief Concatenates two numpy matrices along a specified axis.
 * If axis=0, performs row stacking. If axis=1, performs column concatenation.
 *
 * @param x1
 * @param x2
 * @param axis
 * @return py::array
 */
py::array concatenate(py::array x1, py::array x2, int axis) {
    auto buf1 = x1.request();
    auto buf2 = x2.request();

    if (buf1.ndim != buf2.ndim) {
        throw std::runtime_error("Input arrays must have same ndim.");
    }
    if (axis < 0)
        axis += buf1.ndim;
    if (axis < 0 || axis >= buf1.ndim) {
        throw std::runtime_error("Axis out of range.");
    }

    for (ssize_t d = 0; d < buf1.ndim; ++d) {
        if (d != axis && buf1.shape[d] != buf2.shape[d]) {
            throw std::runtime_error("Shapes differ on non-concat axis.");
        }
    }

    std::vector<ssize_t> new_shape(buf1.ndim);
    for (ssize_t d = 0; d < buf1.ndim; ++d) {
        new_shape[d] = (d == axis) ? buf1.shape[d] + buf2.shape[d] : buf1.shape[d];
    }

    py::array out(x1.dtype(), new_shape);
    auto buf_out = out.request();

    const uint8_t *p1 = static_cast<const uint8_t *>(buf1.ptr);
    const uint8_t *p2 = static_cast<const uint8_t *>(buf2.ptr);
    uint8_t *pout = static_cast<uint8_t *>(buf_out.ptr);
    size_t itemsize = buf1.itemsize;

    if (buf1.ndim == 1) {
        if (axis != 0)
            throw std::runtime_error("Invalid axis for 1D concat.");
        std::memcpy(pout, p1, buf1.shape[0] * itemsize);
        std::memcpy(pout + buf1.shape[0] * itemsize, p2, buf2.shape[0] * itemsize);
        return out;
    }

    // 2D
    if (buf1.ndim == 2) {
        if (axis == 0) {
            // row stacking
            ssize_t rows1 = buf1.shape[0];
            ssize_t rows2 = buf2.shape[0];
            ssize_t row_bytes = buf1.shape[1] * itemsize; // assumes same ncols for both
            std::memcpy(pout, p1, rows1 * row_bytes);
            std::memcpy(pout + rows1 * row_bytes, p2, rows2 * row_bytes);
            return out;
        } else if (axis == 1) {
            // column concatenation (stride-safe)
            ssize_t nrows = buf1.shape[0];
            ssize_t ncols1 = buf1.shape[1];
            ssize_t ncols2 = buf2.shape[1];

            ssize_t rstride1 = buf1.strides[0];
            ssize_t rstride2 = buf2.strides[0];
            ssize_t rstride_out = buf_out.strides[0];
            ssize_t cstride1 = buf1.strides[1];
            ssize_t cstride2 = buf2.strides[1];
            ssize_t cstride_out = buf_out.strides[1];

            bool simple = (cstride1 == (ssize_t)itemsize) && (cstride2 == (ssize_t)itemsize) &&
                          (cstride_out == (ssize_t)itemsize);

            for (ssize_t i = 0; i < nrows; ++i) {
                const uint8_t *row1 = p1 + i * rstride1;
                const uint8_t *row2 = p2 + i * rstride2;
                uint8_t *row_out = pout + i * rstride_out;

                if (simple) {
                    std::memcpy(row_out, row1, ncols1 * itemsize);
                    std::memcpy(row_out + ncols1 * itemsize, row2, ncols2 * itemsize);
                } else {
                    // copy first block
                    for (ssize_t j = 0; j < ncols1; ++j) {
                        std::memcpy(row_out + j * cstride_out, row1 + j * cstride1, itemsize);
                    }
                    // copy second block
                    for (ssize_t j = 0; j < ncols2; ++j) {
                        std::memcpy(row_out + (ncols1 + j) * cstride_out, row2 + j * cstride2,
                                    itemsize);
                    }
                }
            }
            return out;
        } else {
            throw std::runtime_error("Axis not implemented for 2D.");
        }
    }

    throw std::runtime_error("ndim > 2 not implemented.");
}

/**
 * @brief Converts a Z2R array to a uint8 array.
 *
 * @param z2r
 * @param num_bits
 * @return py::array_t<uint8_t> Returns an array of shape (rows, num_bits) with each uint8_t element
 * being either 0 or 1, representing the bits of the input Z2R array
 */
py::array_t<uint8_t> z2_to_uint8(py::array z2r, int num_bits) {
    auto buf = z2r.request();
    if (buf.ndim == 0 || buf.ndim > 2) {
        throw std::runtime_error("z2_to_uint8 supports only 1D or 2D arrays.");
    }

    ssize_t rows = buf.shape[0];
    ssize_t cols = (buf.ndim == 1) ? 1 : buf.shape[1]; // number of void elements per row
    size_t itemsize = buf.itemsize;                    // bytes per void element
    size_t bytes_per_row = cols * itemsize;

    // If 2D, num_bits must be divisible by cols (equal bits per void element)
    if (buf.ndim == 2 && (num_bits % cols) != 0) {
        throw std::runtime_error("num_bits must be a multiple of number of columns.");
    }

    int bits_per_void = (buf.ndim == 1) ? num_bits : (num_bits / cols);
    size_t max_bits_per_void = itemsize * 8;
    if (bits_per_void > (int)max_bits_per_void) {
        throw std::runtime_error("bits_per_void exceeds capacity of dtype.");
    }

    py::array_t<uint8_t> out({rows, (ssize_t)num_bits});
    auto bout = out.request();
    uint8_t *out_ptr = static_cast<uint8_t *>(bout.ptr);

    const uint8_t *base = static_cast<const uint8_t *>(buf.ptr);

    for (ssize_t row = 0; row < rows; ++row) {
        const uint8_t *row_ptr = base + row * bytes_per_row;
        int out_offset = row * num_bits;
        for (ssize_t c = 0; c < cols; ++c) {
            const uint8_t *void_ptr = row_ptr + c * itemsize;
            for (int b = 0; b < bits_per_void; ++b) {
                size_t byte_idx = b / 8;
                int bit_idx = b % 8;
                uint8_t bit = (void_ptr[byte_idx] >> bit_idx) & 0x1;
                out_ptr[out_offset + c * bits_per_void + b] = bit;
            }
        }
    }
    return out;
}

/**
 * @brief Calculates the inverse of a matrix (if possible) using Gauss-Jordan elimination.
 * This function will not work on a matrix if :
 * - It is not "2D", as in a 1D list of packed voids
 * - It is not "square" (num_qubits == number of rows)
 * - It is inconsistant with its dtype and num_qubits
 * - It is is singular, i.e inatly does not have a inverse
 * If an input is bad, a runtime error will be thrown and function exited.
 * TODO: rename function variables to something less ass
 *
 * @param z2r
 * @param num_bits
 * @return py::array
 */
py::array gauss_jordan_inverse(py::array z2r, int num_bits) {
    auto buf = z2r.request();
    if (buf.ndim != 1) {
        throw std::runtime_error("gauss_jordan_inverse expects 1D z2r array of voids.");
    }
    ssize_t n = buf.shape[0];
    if (num_bits != n) {
        // Require square (n x n) bit matrix
        throw std::runtime_error("num_bits must equal number of rows (square matrix).");
    }
    size_t itemsize = buf.itemsize;
    if ((size_t)num_bits > itemsize * 8) {
        throw std::runtime_error("num_bits exceeds bit capacity of row dtype.");
    }

    py::array A = py::array(z2r.dtype(), buf.shape);
    auto bufA = A.request();
    std::memcpy(bufA.ptr, buf.ptr, buf.size * buf.itemsize);
    uint8_t *Aptr = static_cast<uint8_t *>(bufA.ptr);

    // identity crisis
    py::array Inv = py::array(z2r.dtype(), buf.shape);
    auto bufInv = Inv.request();
    uint8_t *Iptr = static_cast<uint8_t *>(bufInv.ptr);
    std::memset(Iptr, 0, bufInv.size * bufInv.itemsize);
    for (ssize_t row = 0; row < n; ++row) {
        ssize_t byte_idx = row / 8;
        int bit_idx = row % 8;
        Iptr[row * itemsize + byte_idx] |= (1u << bit_idx);
    }

    // G-J here
    for (int col = 0; col < num_bits; ++col) {
        ssize_t pivot = -1;
        ssize_t byte_idx = col / 8;
        int bit_idx = col % 8;

        // Find pivot
        for (ssize_t row = col; row < n; ++row) {
            uint8_t bit = (Aptr[row * itemsize + byte_idx] >> bit_idx) & 1u;
            // uint8_t bit =
            if (bit) {
                pivot = row;
                break;
            }
        }
        if (pivot < 0) {
            throw std::runtime_error("Matrix is singular (no pivot).");
        }

        // Swap pivot row into position col
        if (pivot != col) {
            for (size_t b = 0; b < itemsize; ++b) {
                std::swap(Aptr[pivot * itemsize + b], Aptr[col * itemsize + b]);
                std::swap(Iptr[pivot * itemsize + b], Iptr[col * itemsize + b]);
            }
        }

        // Eliminate other rows
        for (ssize_t row = 0; row < n; ++row) {
            if (row == col)
                continue;
            uint8_t bit = (Aptr[row * itemsize + byte_idx] >> bit_idx) & 1u;
            if (bit) {
                //
                for (size_t b = 0; b < itemsize; ++b) {
                    Aptr[row * itemsize + b] ^= Aptr[col * itemsize + b];
                    //
                    Iptr[row * itemsize + b] ^= Iptr[col * itemsize + b];
                }
            }
        }
    }
    return Inv;
}
