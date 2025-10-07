#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <omp.h>
#include <cstring>
#include <algorithm>
#include <random>


#include <Eigen/Sparse>
// #include <pybind11/pybind11.h>
// #include <pybind11/numpy.h>

using pauli_t = uint8_t;

struct dpoint{
    unsigned int begin;
    unsigned int end;
};



std::vector<dpoint> make_dpoint(const std::vector<uint8_t>& vec){
    std::vector<dpoint> points;
    bool in_run = false;
    unsigned run_start = 0;
    for (size_t i = 0; i < vec.size(); ++i) {
        bool bit = (vec[i] != 0);
        if (bit) {
            if (!in_run) {
                in_run = true;
                run_start = static_cast<unsigned>(i);
            }
        } else {
            if (in_run) {
                dpoint p{run_start, static_cast<unsigned>(i)}; // end is exclusive
                points.push_back(p);
                in_run = false;
            }
        }
    }
    if (in_run) {
        dpoint p{run_start, static_cast<unsigned>(vec.size())}; // end exclusive = n_paulis
        points.push_back(p);
    }
    return points;
}



void show_dpoints(const std::vector<dpoint>& points) {
    for (const auto& p : points) {
        std::cout << "[" << p.begin << ", " << p.end << ") ";
    }
    std::cout << std::endl;
}

std::vector<uint8_t> generate_random_vec(int size, float density=0.1, float proximity=0.0) {
    if (size <= 0) return {};
    // clamp parameters to [0,1]
    double p = std::min(std::max(static_cast<double>(density), 0.0), 1.0);
    double q = std::min(std::max(static_cast<double>(proximity), 0.0), 1.0);

    std::vector<uint8_t> v;
    v.reserve(static_cast<size_t>(size));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    bool prev = false;
    for (int i = 0; i < size; ++i) {
        double prob = prev ? (p + q * (1.0 - p)) : (p * (1.0 - q));
        bool bit = dist(gen) < prob;
        v.push_back(bit ? 1 : 0);
        prev = bit;
    }
    return v;
    
}


class SparsePauliArray {
private:
    std::vector<dpoint> z_points;
    std::vector<dpoint> x_points;
    size_t n_qubits;
    bool sorted;
        
public:
    SparsePauliArray xor_runs(const SparsePauliArray& other) const;
    SparsePauliArray();
    // SparsePauliArray(py::array z_void, py::array x_void, size_t n_qubits);
    SparsePauliArray(std::vector<uint8_t> z_vec, std::vector<uint8_t> x_vec, size_t n_qubits);
    SparsePauliArray(std::vector<dpoint> z_pts, std::vector<dpoint> x_pts, size_t n_qubits)
        : z_points(z_pts), x_points(x_pts), n_qubits(n_qubits), sorted(false) {}
    void show() const;

    //getters
    const std::vector<dpoint>& get_z_points() const { return z_points; }
    const std::vector<dpoint>& get_x_points() const { return x_points; }
    size_t get_n_qubits() const { return n_qubits; }
    bool is_sorted() const { return sorted; }
    unsigned int get_z_popcount() const;
    unsigned int get_x_popcount() const;
    
    


};



SparsePauliArray::SparsePauliArray() : n_qubits(0), sorted(true) {}
SparsePauliArray::SparsePauliArray(std::vector<uint8_t> z_vec, std::vector<uint8_t> x_vec, size_t n_qbits)
    : n_qubits(n_qbits), sorted(false) {
    if (z_vec.size() != x_vec.size()) {
        throw std::runtime_error("Z and X vectors must have the same length.");
    }
    size_t n_paulis = z_vec.size();
    z_points.reserve(n_paulis);
    x_points.reserve(n_paulis);

    z_points = make_dpoint(z_vec);
    x_points = make_dpoint(x_vec);

    sorted = true; //? probablement
}

void SparsePauliArray::show() const {
    std::cout << "SparsePauliArray:\n";
    std::cout << "Number of qubits: " << n_qubits << "\n";
    std::cout << "Z points:\n";
    show_dpoints(z_points);
    std::cout << "X points:\n";
    show_dpoints(x_points); 
    std::cout << "Sorted: " << (sorted ? "true" : "false") << "\n\n";
}


unsigned int SparsePauliArray::get_z_popcount() const {
    unsigned int total = 0;
    for (const auto& p : z_points) {
        total += (p.end - p.begin);
    }
    return total;
}
unsigned int SparsePauliArray::get_x_popcount() const {
    unsigned int total = 0;
    for (const auto& p : x_points) {
        total += (p.end - p.begin);
    }
    return total;
}


std::vector<dpoint> xor_dpoint(const std::vector<dpoint>& a, const std::vector<dpoint>& b) {
    std::vector<dpoint> res;
    size_t i = 0, j = 0;
    if (a.empty() && b.empty()) return res;

    const unsigned INF = static_cast<unsigned>(-1);

    unsigned pos;
    if (i < a.size() && j < b.size()) pos = std::min(a[0].begin, b[0].begin);
    else if (i < a.size()) pos = a[0].begin;
    else pos = b[0].begin;

    while (i < a.size() || j < b.size()) {
        unsigned a_begin = (i < a.size()) ? a[i].begin : INF;
        unsigned a_end   = (i < a.size()) ? a[i].end   : INF;
        unsigned b_begin = (j < b.size()) ? b[j].begin : INF;
        unsigned b_end   = (j < b.size()) ? b[j].end   : INF;

        // determine membership at current position
        bool in_a = (i < a.size() && pos >= a_begin && pos < a_end);
        bool in_b = (j < b.size() && pos >= b_begin && pos < b_end);

        // next change can occur at an end OR at the next begin that is after pos
        unsigned next = INF;
        if (i < a.size()) next = std::min(next, a_end);
        if (j < b.size()) next = std::min(next, b_end);
        if (i < a.size() && a_begin > pos) next = std::min(next, a_begin);
        if (j < b.size() && b_begin > pos) next = std::min(next, b_begin);

        if (next == INF) break;

        // if exactly one has a run here, emit/extend a run on result
        if (in_a ^ in_b) {
            if (!res.empty() && res.back().end == pos) {
                res.back().end = next;
            } else {
                res.push_back({pos, next});
            }
        }

        pos = next;

        // advance iterators that ended at pos
        if (i < a.size() && pos >= a_end) ++i;
        if (j < b.size() && pos >= b_end) ++j;

        // if there's a gap before the next run begins, jump to the next begin
        unsigned next_begin = INF;
        if (i < a.size()) next_begin = std::min(next_begin, a[i].begin);
        if (j < b.size()) next_begin = std::min(next_begin, b[j].begin);
        if (pos < next_begin && next_begin != INF) pos = next_begin;
    }

    return res;
}

std::vector<dpoint> and_dpoint(const std::vector<dpoint>& a, const std::vector<dpoint>& b) {
    std::vector<dpoint> res;
    size_t i = 0, j = 0;
    if (a.empty() || b.empty()) return res;

    while (i < a.size() && j < b.size()) {
        unsigned a_begin = a[i].begin;
        unsigned a_end   = a[i].end;
        unsigned b_begin = b[j].begin;
        unsigned b_end   = b[j].end;

        // find overlap
        unsigned start = std::max(a_begin, b_begin);
        unsigned end   = std::min(a_end, b_end);

        if (start < end) {
            res.push_back({start, end});
        }

        // advance the interval that ends first
        if (a_end < b_end) {
            ++i;
        } else {
            ++j;
        }
    }

    return res;
}

std::vector<dpoint> not_dpoint(const std::vector<dpoint>& a, size_t n_qubits) {
    std::vector<dpoint> res;
    if (a.empty()) {
        res.push_back({0, static_cast<unsigned>(n_qubits)});
        return res;
    }

    unsigned prev_end = 0;
    for (const auto& p : a) {
        if (p.begin > prev_end) {
            res.push_back({prev_end, p.begin});
        }
        prev_end = p.end;
    }
    if (prev_end < n_qubits) {
        res.push_back({prev_end, static_cast<unsigned>(n_qubits)});
    }

    return res;
}

std::vector<dpoint> or_dpoint(const std::vector<dpoint>& a, const std::vector<dpoint>& b) {
    std::vector<dpoint> res;
    size_t i = 0, j = 0;
    if (a.empty()) return b;
    if (b.empty()) return a;
    while (i < a.size() || j < b.size()) {
        unsigned a_begin = (i < a.size()) ? a[i].begin : static_cast<unsigned>(-1);
        unsigned a_end   = (i < a.size()) ? a[i].end   : static_cast<unsigned>(-1);
        unsigned b_begin = (j < b.size()) ? b[j].begin : static_cast<unsigned>(-1);
        unsigned b_end   = (j < b.size()) ? b[j].end   : static_cast<unsigned>(-1);

        unsigned next_begin, next_end;
        if (a_begin < b_begin) {
            next_begin = a_begin;
            next_end = a_end;
            ++i;
        } else if (b_begin < a_begin) {
            next_begin = b_begin;
            next_end = b_end;
            ++j;
        } else { // equal
            next_begin = a_begin;
            next_end = std::max(a_end, b_end);
            ++i; ++j;
        }

        if (res.empty() || res.back().end < next_begin) {
            res.push_back({next_begin, next_end});
        } else {
            res.back().end = std::max(res.back().end, next_end);
        }
    }
    return res;
}

void overlap_dpoint(const std::vector<dpoint>& a, const std::vector<dpoint>& b) {
    size_t i = 0, j = 0;
    if (a.empty() || b.empty()) {
        std::cout << "No overlap (one or both arrays are empty)\n";
        return;
    }

    while (i < a.size() && j < b.size()) {
        unsigned a_begin = a[i].begin;
        unsigned a_end   = a[i].end;
        unsigned b_begin = b[j].begin;
        unsigned b_end   = b[j].end;

        // find overlap
        unsigned start = std::max(a_begin, b_begin);
        unsigned end   = std::min(a_end, b_end);

        if (start < end) {
            std::cout << "Overlap detected at [" << start << ", " << end << ") "
                      << "between a[" << i << "]=[" << a_begin << ", " << a_end << ") "
                      << "and b[" << j << "]=[" << b_begin << ", " << b_end << ")\n";
        }

        // advance the interval that ends first
        if (a_end < b_end) {
            ++i;
        } else {
            ++j;
        }
    }
}



