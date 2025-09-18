#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <iostream>
#include <cstdint>
#include <vector>
#include <complex>
#include <omp.h>
#include <random>
#include <string>
#include <cstring>

using namespace std;
using pauli_t = uint8_t;

#define I 0
#define X 1
#define Z 2
#define Y 3



class DensePauliArray {
public:
    //constructors/destructors
    DensePauliArray(int num_operators, int num_qubits);
    DensePauliArray(uint8_t* z_data, uint8_t* x_data, int num_operators, int num_qubits);
    DensePauliArray(const DensePauliArray& other);
    DensePauliArray(DensePauliArray&& other) noexcept;
    DensePauliArray identities(int num_operators, int num_qubits) const;
    ~DensePauliArray();
    
    //operators and friends
    DensePauliArray& operator=(const DensePauliArray& other);
    DensePauliArray& operator=(DensePauliArray&& other) noexcept;
    
    //computational methods
    static DensePauliArray random(int num_operators, int num_qubits);
    vector<bool> commutes_batch(const DensePauliArray& other) const;
    DensePauliArray compose_batch(const DensePauliArray& other) const;
    DensePauliArray tensor(const DensePauliArray& other) const;
    bool is_diagonal() const;
    bool is_identity() const;
    
    //affichage
    string to_string() const;
    
    //helpers
    uint32_t bitwise_dot(const uint8_t* a, const uint8_t* b, size_t length) const;
    
    // Data layout: [operator][qubit]
    //inlines 
    inline uint8_t get_z(int op, int qubit) const { return z_data[op * num_qubits + qubit]; }
    inline uint8_t get_x(int op, int qubit) const { return x_data[op * num_qubits + qubit]; }
    inline void set_z(int op, int qubit, uint8_t val) { z_data[op * num_qubits + qubit] = val; }
    inline void set_x(int op, int qubit, uint8_t val) { x_data[op * num_qubits + qubit] = val; }
    
    int get_num_operators() const { return num_operators; }
    int get_num_qubits() const { return num_qubits; }


private:
    // Flat array: [op0_q0, op0_q1, ..., op0_qN, op1_q0, op1_q1, ...]
    uint8_t* z_data; //z_strings/voids
    uint8_t* x_data;  //x_strings/voids
    int num_operators; //first dimension
    int num_qubits; // second dimension
    size_t data_size;
    bool owns_data;
    
    //helpers
    void allocate_data();
    void deallocate_data();
    void copy_from(const DensePauliArray& other);
};

inline DensePauliArray::DensePauliArray(int num_operators, int num_qubits) 
    : num_operators(num_operators), num_qubits(num_qubits), owns_data(true) {
    allocate_data();
}

inline DensePauliArray::DensePauliArray(uint8_t* z_data, uint8_t* x_data, int num_operators, int num_qubits)
    : z_data(z_data), x_data(x_data), num_operators(num_operators), num_qubits(num_qubits), owns_data(false) {
    data_size = static_cast<size_t>(num_operators) * num_qubits;
}

DensePauliArray::DensePauliArray(const DensePauliArray& other) 
    : num_operators(other.num_operators), num_qubits(other.num_qubits), owns_data(true) {
    allocate_data();
    copy_from(other);
}

DensePauliArray::DensePauliArray(DensePauliArray&& other) noexcept
    : z_data(other.z_data), x_data(other.x_data), 
      num_operators(other.num_operators), num_qubits(other.num_qubits),
      data_size(other.data_size), owns_data(other.owns_data) {
    other.z_data = nullptr;
    other.x_data = nullptr;
    other.owns_data = false;
}

DensePauliArray& DensePauliArray::operator=(const DensePauliArray& other) {
    if (this != &other) {
        if (owns_data) deallocate_data();
        num_operators = other.num_operators;
        num_qubits = other.num_qubits;
        owns_data = true;
        allocate_data();
        copy_from(other);
    }
    return *this;
}

DensePauliArray& DensePauliArray::operator=(DensePauliArray&& other) noexcept {
    if (this != &other) {
        if (owns_data) deallocate_data();
        z_data = other.z_data;
        x_data = other.x_data;
        num_operators = other.num_operators;
        num_qubits = other.num_qubits;
        data_size = other.data_size;
        owns_data = other.owns_data;
        
        other.z_data = nullptr;
        other.x_data = nullptr;
        other.owns_data = false;
    }
    return *this;
}

inline DensePauliArray::~DensePauliArray() {
    if (owns_data) {
        deallocate_data();
    }
}

inline void DensePauliArray::allocate_data() {
    data_size = static_cast<size_t>(num_operators) * num_qubits;
    
    size_t aligned_size = (data_size + 63) & ~63;
    
    if (posix_memalign(reinterpret_cast<void**>(&z_data), 64, aligned_size) != 0) { //a tester sur mac
        z_data = nullptr;
    }
    if (posix_memalign(reinterpret_cast<void**>(&x_data), 64, aligned_size) != 0) {
        x_data = nullptr;
    }
    
    
    if (!z_data || !x_data) {
        throw std::bad_alloc();
    }
    
    memset(z_data, 0, data_size);
    memset(x_data, 0, data_size);
}

inline void DensePauliArray::deallocate_data() {
    if (z_data) { free(z_data); z_data = nullptr; }
    if (x_data) { free(x_data); x_data = nullptr; }
}

inline void DensePauliArray::copy_from(const DensePauliArray& other) {
    memcpy(z_data, other.z_data, data_size);
    memcpy(x_data, other.x_data, data_size);
}

// OK! Envrion 4x plus rapide que PA
DensePauliArray DensePauliArray::random(int num_operators, int num_qubits) {
    DensePauliArray result(num_operators, num_qubits);
    
    #pragma omp parallel
    {
        thread_local static random_device rd;
        thread_local static mt19937 gen(rd());
        thread_local static uniform_int_distribution<uint8_t> dis(0, 1);
        
        #pragma omp for schedule(static)
        for (size_t i = 0; i < result.data_size; i++) {
            result.z_data[i] = dis(gen);
            result.x_data[i] = dis(gen);
        }
    }
    
    return result;
}

/**
 * * OK? ~1.1x plus rapide que PA
 * TODO: optimiser davantage
 * Fait un test element-wise de commutation entre this et other
 * @param other test
 * @return vector<bool> de taille num_operators, true si commute, false sinon
 */
vector<bool> DensePauliArray::commutes_batch(const DensePauliArray& other) const {
    if (num_operators != other.num_operators || num_qubits != other.num_qubits) {
        throw invalid_argument("Dimension mismatch");
    }
    
    vector<bool> results(num_operators, true);
    
    #pragma omp parallel for schedule(static)
    for (int op = 0; op < num_operators; op++) {
        uint32_t anti_commute_count = 0;
        
        // Sequential memory access
        const size_t base_idx = op * num_qubits;
        const uint8_t* z1_ptr = z_data + base_idx;
        const uint8_t* x1_ptr = x_data + base_idx;
        const uint8_t* z2_ptr = other.z_data + base_idx;
        const uint8_t* x2_ptr = other.x_data + base_idx;
        
        for (int qubit = 0; qubit < num_qubits; qubit++) {
            anti_commute_count += (x1_ptr[qubit] & z2_ptr[qubit]) ^ (z1_ptr[qubit] & x2_ptr[qubit]);
        }
        
        results[op] = (anti_commute_count % 2) == 0;
    }
    
    return results;
}


/**
 * ! NON! TROP LENT
 * TODO: rendre ca pas dogwater (jsp)
 * 
 * Fait la composition entre this et other
 */
DensePauliArray DensePauliArray::compose_batch(const DensePauliArray& other) const {

    if (num_operators != other.num_operators || num_qubits != other.num_qubits) {
        throw invalid_argument("Dimension mismatch");
    }
    uint8_t* new_z = (uint8_t*)aligned_alloc(64, data_size);
    uint8_t* new_x = (uint8_t*)aligned_alloc(64, data_size);
    
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < data_size; i++) {
        new_z[i] = z_data[i] ^ other.z_data[i];
        new_x[i] = x_data[i] ^ other.x_data[i];
    }
    
    uint32_t this_phase_power = bitwise_dot(z_data, x_data, data_size);
    uint32_t other_phase_power = bitwise_dot(other.z_data, other.x_data, data_size);
    uint32_t new_phase_power = bitwise_dot(new_z, new_x, data_size);
    uint32_t commutation_phase_power = 2 * bitwise_dot(x_data, other.z_data, data_size);
    uint32_t total_phase_power = (this_phase_power + other_phase_power + commutation_phase_power - new_phase_power) % 4;
    cout << "Phase power: " << total_phase_power << endl;


    DensePauliArray result(new_z, new_x, num_operators, num_qubits);
    
    return result;
}

//fonction pourrite ca vaut rien
uint32_t DensePauliArray::bitwise_dot(const uint8_t* data1, const uint8_t* data2, size_t size) const {
    uint32_t result = 0;

    #pragma omp parallel for reduction(+:result) schedule(static)
    for (size_t i = 0; i < size; i++) {
        uint8_t and_result = data1[i] & data2[i];
        result += __builtin_popcount(and_result); //jsp a tester si ca marche ou pas
    }

    return result;
}

string DensePauliArray::to_string() const {
    static const string pauli_table[2][2] = {
        {"I", "X"},
        {"Z", "Y"}
    };

    string res;
    res.reserve(num_operators * (num_qubits + 10)); 

    for (int op = 0; op < num_operators; op++) {
        res += "P" + std::to_string(op) + ": ";
        for (int qubit = 0; qubit < num_qubits; qubit++) {
            uint8_t z = get_z(op, qubit);
            uint8_t x = get_x(op, qubit);
            res += pauli_table[z][x];
        }
        res += "\n";
    }

    return res;
}


/**
 * Fait le produit tensoriel entre this et other. En realite, c'est juste une concatenation
 */
// * OK! Pour petits tableaux, ~4x plus rapide que PA
// * Pour grands tableaux, ~6x plus rapide que PA
DensePauliArray DensePauliArray::tensor(const DensePauliArray& other) const {
    
    int new_num_qubits = num_qubits + other.num_qubits;
    int new_num_operators = num_operators;
    size_t new_data_size = static_cast<size_t>(new_num_operators) * new_num_qubits;

    cout << "Tensor C++: new_num_operators = " << new_num_operators << ", new_num_qubits = " << new_num_qubits << ", new_data_size = " << new_data_size << endl;
    
    uint8_t* new_z = (uint8_t*)aligned_alloc(64, new_data_size);
    uint8_t* new_x = (uint8_t*)aligned_alloc(64, new_data_size);

    #pragma omp parallel for schedule(static)
    for (int op = 0; op < new_num_operators; op++) {
        memcpy(new_z + op * new_num_qubits, z_data + op * num_qubits, num_qubits);
        memcpy(new_x + op * new_num_qubits, x_data + op * num_qubits, num_qubits);

        memcpy(new_z + op * new_num_qubits + num_qubits, other.z_data + op * other.num_qubits, other.num_qubits);
        memcpy(new_x + op * new_num_qubits + num_qubits, other.x_data + op * other.num_qubits, other.num_qubits);
    }


    return DensePauliArray(new_z, new_x, new_num_operators, new_num_qubits);
}

bool DensePauliArray::is_diagonal() const {
    for (size_t i = 0; i < data_size; i++) {
        if (x_data[i] != 0) {
            return false;
        }
    }
    return true;
}

bool DensePauliArray::is_identity() const {
    for (size_t i = 0; i < data_size; i++) {
        if (x_data[i] != 0 || z_data[i] != 0) {
            return false;
        }
    }
    return true;
}

DensePauliArray DensePauliArray::identities(int num_operators, int num_qubits) const {
    DensePauliArray result(num_operators, num_qubits);
    return result;
}



