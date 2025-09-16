#pragma once
#include <vector>
#include <complex>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <numeric>
#include <omp.h>
#include <random>
#include <map>
#include <cstring>

using namespace std;

class PSet {
    public:
        PSet(size_t n=0);
        ~PSet();
        size_t size() const { return this->m_size; }
    private:
        size_t m_size = 0;
}; 