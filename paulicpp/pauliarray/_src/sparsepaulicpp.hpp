#pragma once
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
// #include <algorithm>
#include <random>
#include <utility> // std::pair

// #include <Eigen/Sparse>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

using pauli_t = uint8_t;

using dpoint = std::pair<unsigned int, unsigned int>; // first = begin, second = end (exclusive)

#ifdef USE_OPENMP
#    include <omp.h>
#endif

#define THRESHOLD_PARALLEL 1'000

inline std::vector<dpoint> make_dpoint(const std::vector<uint8_t> &vec) {
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
                dpoint p{run_start, static_cast<unsigned>(i)}; // end is exclsusive
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

inline std::vector<uint8_t> generate_random_vec(int size, float density = 0.1,
                                                float proximity = 0.0) {
    if (size <= 0)
        return {};
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

void show_dpoints(const std::vector<dpoint> &points) {
    for (const auto &p : points) {
        std::cout << "[" << p.first << ", " << p.second << ") ";
    }
    std::cout << std::endl;
}

inline std::vector<dpoint> xor_dpoint(const std::vector<dpoint> &a, const std::vector<dpoint> &b) {
    std::vector<dpoint> res;
    size_t i = 0, j = 0;
    if (a.empty() && b.empty())
        return res;

    const unsigned INF = static_cast<unsigned>(-1);

    unsigned pos;
    if (i < a.size() && j < b.size())
        pos = std::min(a[0].first, b[0].first);
    else if (i < a.size())
        pos = a[0].first;
    else
        pos = b[0].first;

    while (i < a.size() || j < b.size()) {
        unsigned a_begin = (i < a.size()) ? a[i].first : INF;
        unsigned a_end = (i < a.size()) ? a[i].second : INF;
        unsigned b_begin = (j < b.size()) ? b[j].first : INF;
        unsigned b_end = (j < b.size()) ? b[j].second : INF;

        bool in_a = (i < a.size() && pos >= a_begin && pos < a_end);
        bool in_b = (j < b.size() && pos >= b_begin && pos < b_end);

        unsigned next = INF;
        if (i < a.size())
            next = std::min(next, a_end);
        if (j < b.size())
            next = std::min(next, b_end);
        if (i < a.size() && a_begin > pos)
            next = std::min(next, a_begin);
        if (j < b.size() && b_begin > pos)
            next = std::min(next, b_begin);

        if (next == INF)
            break;

        if (in_a ^ in_b) {
            if (!res.empty() && res.back().second == pos) {
                res.back().second = next;
            } else {
                res.push_back({pos, next});
            }
        }

        pos = next;

        if (i < a.size() && pos >= a_end)
            ++i;
        if (j < b.size() && pos >= b_end)
            ++j;

        unsigned next_begin = INF;
        if (i < a.size())
            next_begin = std::min(next_begin, a[i].first);
        if (j < b.size())
            next_begin = std::min(next_begin, b[j].first);
        if (pos < next_begin && next_begin != INF)
            pos = next_begin;
    }

    return res;
}
#include <algorithm>

inline std::vector<dpoint> and_dpoint(const std::vector<dpoint> &a, const std::vector<dpoint> &b) {
    std::vector<dpoint> res;
    const size_t na = a.size(), nb = b.size();
    if (na == 0 || nb == 0)
        return res;

    res.reserve(std::min(na, nb)); // heuristic

    size_t i = 0, j = 0;
    while (i < na && j < nb) {
        const unsigned a_begin = a[i].first;
        const unsigned a_end = a[i].second;
        const unsigned b_begin = b[j].first;
        const unsigned b_end = b[j].second;

        // no overlap and a finishes before b starts -> skip forward in a
        if (a_end <= b_begin) {
            // find first interval in 'a' with end > b_begin (galloping via binary search)
            auto it =
                std::upper_bound(a.begin() + i + 1, a.end(), b_begin,
                                 [](unsigned val, const dpoint &p) { return val < p.second; });
            i = (it == a.end()) ? na : static_cast<size_t>(std::distance(a.begin(), it));
            continue;
        }

        // no overlap and b finishes before a starts -> skip forward in b
        if (b_end <= a_begin) {
            auto it =
                std::upper_bound(b.begin() + j + 1, b.end(), a_begin,
                                 [](unsigned val, const dpoint &p) { return val < p.second; });
            j = (it == b.end()) ? nb : static_cast<size_t>(std::distance(b.begin(), it));
            continue;
        }

        // there is overlap
        const unsigned start = (a_begin > b_begin) ? a_begin : b_begin;
        const unsigned end = (a_end < b_end) ? a_end : b_end;
        res.emplace_back(start, end);

        // advance the interval that finishes first
        if (a_end < b_end)
            ++i;
        else
            ++j;
    }

    return res;
}

inline std::vector<dpoint> not_dpoint(const std::vector<dpoint> &a, size_t n_qubits) {
    std::vector<dpoint> res;
    if (a.empty()) {
        res.push_back({0, static_cast<unsigned>(n_qubits)});
        return res;
    }

    unsigned prev_end = 0;
    for (const auto &p : a) {
        if (p.first > prev_end) {
            res.push_back({prev_end, p.first});
        }
        prev_end = p.second;
    }
    if (prev_end < n_qubits) {
        res.push_back({prev_end, static_cast<unsigned>(n_qubits)});
    }

    return res;
}

inline std::vector<dpoint> or_dpoint(const std::vector<dpoint> &a, const std::vector<dpoint> &b) {
    std::vector<dpoint> res;
    size_t i = 0, j = 0;
    if (a.empty())
        return b;
    if (b.empty())
        return a;
    while (i < a.size() || j < b.size()) {
        unsigned a_begin = (i < a.size()) ? a[i].first : static_cast<unsigned>(-1);
        unsigned a_end = (i < a.size()) ? a[i].second : static_cast<unsigned>(-1);
        unsigned b_begin = (j < b.size()) ? b[j].first : static_cast<unsigned>(-1);
        unsigned b_end = (j < b.size()) ? b[j].second : static_cast<unsigned>(-1);

        unsigned next_begin, next_end;
        if (a_begin < b_begin) {
            next_begin = a_begin;
            next_end = a_end;
            ++i;
        } else if (b_begin < a_begin) {
            next_begin = b_begin;
            next_end = b_end;
            ++j;
        } else {
            next_begin = a_begin;
            next_end = std::max(a_end, b_end);
            ++i;
            ++j;
        }

        if (res.empty() || res.back().second < next_begin) {
            res.push_back({next_begin, next_end});
        } else {
            res.back().second = std::max(res.back().second, next_end);
        }
    }
    return res;
}

unsigned int count_dpoint(const std::vector<dpoint> &a) {
    unsigned int count = 0;
    for (const auto &p : a) {
        count += (p.second - p.first);
    }
    return count;
}

inline uint8_t dot_dpoint(const std::vector<dpoint> &a, const std::vector<dpoint> &b) {
    return (count_dpoint(and_dpoint(a, b)));
}

inline std::vector<uint8_t> dpoint_to_vec(const std::vector<dpoint> &points, size_t n_qubits) {
    std::vector<uint8_t> vec(n_qubits, 0);
    for (const auto &p : points) {
        for (unsigned int i = p.first; i < p.second; ++i) {
            vec[i] = 1;
        }
    }
    return vec;
}

void overlap_dpoint(const std::vector<dpoint> &a, const std::vector<dpoint> &b) {
    size_t i = 0, j = 0;
    if (a.empty() || b.empty()) {
        std::cout << "No overlap (one or both arrays are empty)\n";
        return;
    }

    while (i < a.size() && j < b.size()) {
        unsigned a_begin = a[i].first;
        unsigned a_end = a[i].second;
        unsigned b_begin = b[j].first;
        unsigned b_end = b[j].second;

        unsigned start = std::max(a_begin, b_begin);
        unsigned end = std::min(a_end, b_end);

        if (start < end) {
            std::cout << "Overlap detected at [" << start << ", " << end << ") " << "between a["
                      << i << "]=[" << a_begin << ", " << a_end << ") " << "and b[" << j << "]=["
                      << b_begin << ", " << b_end << ")\n";
        }

        if (a_end < b_end) {
            ++i;
        } else {
            ++j;
        }
    }
}