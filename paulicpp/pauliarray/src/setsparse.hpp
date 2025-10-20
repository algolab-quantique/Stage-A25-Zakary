#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <cstdint> // uint8_t
#include <iostream>
#include <cstring>
#include <omp.h>
#include <bit>
#include <set>

using dpoint = std::pair<unsigned int, unsigned int>; // first = begin, second = end (exclusive)


#define THRESHOLD_PARALLEL 1'000


std::set<dpoint> make_dpoint_set(const std::vector<uint8_t>& vec){
std::set<dpoint> set;
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
                set.insert(p);
                in_run = false;
            }
        }
    }
    if (in_run) {
        dpoint p{run_start, static_cast<unsigned>(vec.size())}; // end exclusive = n_paulis
        set.insert(p);
    }
    return set;
}

std::set<dpoint> xor_set(const std::set<dpoint>& a, const std::set<dpoint>& b) {
    std::set<dpoint> res;
    std::set<dpoint>::iterator it_a = a.begin();
    std::set<dpoint>::iterator it_b = b.begin();


    return res;
}