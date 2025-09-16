#pragma once
#include <iostream>
#include <cstdint> // uint8_t
#include <vector>
#include <numeric>
#include <complex>
#include <omp.h>
#include <random>
#include <map>
#include <cstring>
#include <bitset>

using namespace std;
using pauli_t = uint8_t;

#define I 0  // 0b00
#define X 1  // 0b01
#define Z 2  // 0b10
#define Y 3  // 0b11

class DensePauliArray {
    public:
        DensePauliArray();
        template <size_t N>
        DensePauliArray(bitset<N> z, bitset<N> x, complex<double> phase=1);
        ~DensePauliArray();
        int size() const;
        vector<uint8_t> get_x_bitset() const;
        vector<uint8_t> get_z_bitset() const;

    private:
        bitset<0> z_string;
        bitset<0> x_string;
        complex<double> phase;


};


DensePauliArray::DensePauliArray() {
    phase = 1;
}

template <size_t N>
DensePauliArray::DensePauliArray(bitset<N> z, bitset<N> x, complex<double> phase) {
    

    this->x_string = x;
    this->phase = phase;
}