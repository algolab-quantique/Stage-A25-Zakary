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
        DensePauliArray(vector<bool> z, vector<bool> x, complex<double> phase=1);
        DensePauliArray(vector<pauli_t> paulis, complex<double> phase=1);
        ~DensePauliArray();
        int size() const;
        vector<bool> get_x_string() const;
        vector<bool> get_z_string() const;

        static DensePauliArray random(int n);
        vector<bool> commutes(const DensePauliArray &other) const;
    private:
        vector<pauli_t> paulis;
        vector<bool> z_string;
        vector<bool> x_string;
        complex<double> phase;


};


DensePauliArray::DensePauliArray() {
    phase = 1;
}

DensePauliArray::DensePauliArray(vector<bool> z, vector<bool> x, complex<double> phase) {
    
    this->paulis.resize(z.size());
    #pragma omp parallel for
    for (int i = 0; i < z.size(); i++) {
        if (z[i] && x[i]) {
            paulis[i] = Y;
        } else if (z[i]) {
            paulis[i] = Z;
        } else if (x[i]) {
            paulis[i] = X;
        } else {
            paulis[i] = I;
        }
    }
    this->z_string = z;
    this->x_string = x;
    this->phase = phase;
}

DensePauliArray::DensePauliArray(vector<pauli_t> paulis, complex<double> phase) {
    this->paulis = paulis;
    this->z_string.resize(paulis.size());
    this->x_string.resize(paulis.size());
    #pragma omp parallel for
    for (int i = 0; i < paulis.size(); i++) {
        if (paulis[i] == Y) {
            z_string[i] = 1;
            x_string[i] = 1;
        } else if (paulis[i] == Z) {
            z_string[i] = 1;
            x_string[i] = 0;
        } else if (paulis[i] == X) {
            z_string[i] = 0;
            x_string[i] = 1;
        } else {
            z_string[i] = 0;
            x_string[i] = 0;
        }
    }
    this->phase = phase;
}

DensePauliArray::~DensePauliArray() {
    paulis.clear();
    z_string.clear();
    x_string.clear();
}

int DensePauliArray::size() const {
    return paulis.size();
}

DensePauliArray DensePauliArray::random(int n) {
    vector<pauli_t> paulis(n);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 3);
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        paulis[i] = dis(gen);
    }
    return DensePauliArray(move(paulis));
}

vector<bool> DensePauliArray::commutes(const DensePauliArray &other) const {
    if (this->size() != other.size()) {
        throw invalid_argument("Pauli arrays size");
    }
    vector<bool> result(this->size());

    static const pauli_t product_table[4][4] = {
        {true, true, true, true},   // I x {I,X,Z,Y}
        {true, true, false, false}, // X x {I,X,Z,Y}
        {true, false, true, false}, // Z x {I,X,Z,Y}
        {true, false, false, true}  // Y x {I,X,Z,Y
    };

    #pragma omp parallel for
    for (int i = 0; i < this->size(); i++) {
        // result[i] = product_table[this->paulis[i]][other.paulis[i]] == product_table[other.paulis[i]][this->paulis[i]];
        result[i] = product_table[this->paulis[i]][other.paulis[i]];
    }
    return result;
}




