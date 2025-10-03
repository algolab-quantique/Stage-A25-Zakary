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
using pauli_t = uint8_t;

#define I 0  // 0b00
#define X 1  // 0b01  
#define Z 2  // 0b10
#define Y 3  // 0b11


class SparsePauliArray {
public:
    SparsePauliArray(size_t n=0, bool random=false, float density=0.1);
    SparsePauliArray(size_t n, vector<size_t> indices, vector<pauli_t> values, complex<double> ph = {1,0});

    size_t size() const { return nbr_gates; }
    size_t size_nonzero() const { return idx.size(); }

    vector<bool> commutes(const SparsePauliArray& other);

    friend std::ostream &operator<<(std::ostream &os, SparsePauliArray const &pa);

private:
    size_t nbr_gates = 0;
    vector<size_t> idx; 
    vector<pauli_t> vals;  
    complex<double> phase;
};

SparsePauliArray::SparsePauliArray(size_t n, bool random, float density) {
    nbr_gates = n;
    phase = {1,0};
    if (random) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 3);
        std::uniform_real_distribution<> dis_real(0.0, 1.0);

        for (size_t i = 0; i < n; i++) {
            if (dis_real(gen) < density) {
                idx.push_back(i);
                vals.push_back(static_cast<pauli_t>(dis(gen)));
            }
        }
     
    }
} 

SparsePauliArray::SparsePauliArray(size_t n, vector<size_t> indices, vector<pauli_t> values, complex<double> ph){
    
}

vector<bool> SparsePauliArray::commutes(const SparsePauliArray& other) {


    // // PAS BON! ITERE 2x SUR n!!!!!!!!!
    // int n = static_cast<int>(this->size());
    // vector<bool> results(n, true);

    // size_t ia = 0, ib = 0;
    // while (ia < this->idx.size() && ib < other.idx.size()) {
    //     if (this->idx[ia] == other.idx[ib]) {
    //         uint8_t z1 = (this->vals[ia] >> 1) & 0x01;
    //         uint8_t x1 = this->vals[ia] & 0x01;
    //         uint8_t z2 = (other.vals[ib] >> 1) & 0x01;
    //         uint8_t x2 = other.vals[ib] & 0x01;
    //         bool comm = ((z1 * x2 + x1 * z2) % 2 == 0);
    //         results[this->idx[ia]] = comm;
    //         ++ia; 
    //         ++ib;
    //     } else if (this->idx[ia] < other.idx[ib]) {
    //         ++ia;
    //     } else {
    //         ++ib;
    //     }
 
    // }

    // return results;
}



std::ostream &operator<<(std::ostream &os, SparsePauliArray const &pa) { 
    extern std::map<pauli_t, std::string> pauli_to_str;
    os << "sparsepauliarray[size: " << pa.size() << ", nonzero: " << pa.size_nonzero() << ", indices: [";
    for (size_t i=0; i< pa.size_nonzero(); i++) {
        os << pa.idx[i] << ":" << pauli_to_str.at(pa.vals[i]) << " ";
    }
    os << "], phase: " << pa.phase << "]";

    return os;
}