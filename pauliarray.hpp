#pragma once

// #ifdef USE_PYBIND11
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;
// #endif


#include <iostream>
#include <cstdint> // uint8_t
#include <vector>
#include <numeric>
#include <complex>
#include <omp.h>
#include <random>
#include <map>
#include <cstring>
#include <chrono>


using namespace std;
using pauli_t = uint8_t;

#define I 0  // 0b00
#define X 1  // 0b01  
#define Z 2  // 0b10
#define Y 3  // 0b11

map<pauli_t, string> pauli_to_str = {
    {I, "I"},
    {X, "X"},
    {Y, "Y"},
    {Z, "Z"}
};

// static const pauli_t product_table[4][4] = {
//         {I, X, Z, Y},  // I x {I,X,Z,Y}
//         {X, I, Y, Z},  // X x {I,X,Z,Y}  
//         {Z, Y, I, X},  // Z x {I,X,Z,Y}
//         {Y, Z, X, I}   // Y x {I,X,Z,Y}
// };

static const pauli_t product_table[4][4] = {
        {true, true, true, true},   // I x {I,X,Z,Y}
        {true, true, false, false}, // X x {I,X,Z,Y}
        {true, false, true, false}, // Z x {I,X,Z,Y}
        {true, false, false, true}  // Y x {I,X,Z,Y
    };

// static const pauli_t product_table[4][4] = {
//         {1, 1, 1, 1},   // I x {I,X,Z,Y}
//         {1, 1, 0, 0}, // X x {I,X,Z,Y}
//         {1, 0, 1, 0}, // Z x {I,X,Z,Y}
//         {1, 0, 0, 1}  // Y x {I,X,Z,Y
//     };

static const std::complex<double> phase_table[4][4] = {
        {1,  1,  1,  1},   // I x {I,X,Z,Y}
        {1,  1,  1i, -1i}, // X x {I,X,Z,Y}
        {1,  -1i, 1,  1i}, // Z x {I,X,Z,Y} 
        {1,  1i,  -1i, 1}  // Y x {I,X,Z,Y}
    
};


class PauliArray {


    public:
        PauliArray();
        PauliArray(vector<pauli_t> p);
        PauliArray(vector<pauli_t> p, int flag);
        PauliArray(vector<uint8_t> z_string, vector<uint8_t> x_string, complex<double> phase=1);
        ~PauliArray();
        vector<pauli_t> zx_to_paulis(const vector<uint8_t>& z_string, const vector<uint8_t>& x_string) const;
        vector<bool> commutes(const PauliArray& other);
    // #ifdef USE_PYBIND11
        py::array_t<uint8_t> commutes_numpy(const PauliArray& other);
    // #endif
        int size() const;
        vector<uint8_t> get_x_string() const;
        vector<uint8_t> get_z_string() const;
        vector<pauli_t> get_paulis() const;
        friend std::ostream &operator<<(std::ostream &os, PauliArray const &pa);
        PauliArray compose(const PauliArray& other) const;
        PauliArray tensor(const PauliArray& other) const;
        static PauliArray random(int n);
        PauliArray concatenate(const PauliArray& other) const;

    private:
        vector<pauli_t> paulis;
        vector<uint8_t> x_string;
        vector<uint8_t> z_string;
        complex<double> phase;
        void create_xz_strings();



};



PauliArray::PauliArray() {
    paulis = {};
    x_string = {};
    z_string = {};
    phase = 1;
}

PauliArray::PauliArray(vector<pauli_t> p) : paulis(p) {
    if (paulis.empty()) {
        throw std::runtime_error("vide");
    }
    create_xz_strings();
    phase = 1;
}

PauliArray::PauliArray(vector<pauli_t> p, int flag) : paulis(p) {
    if (paulis.empty()) {
        throw std::runtime_error("vide");
    }
    // create_xz_strings();
    phase = 1;
}

PauliArray::PauliArray(vector<uint8_t> z_string, vector<uint8_t> x_string, complex<double> phase){
    this->z_string = z_string;
    this->x_string = x_string;
    this->paulis = zx_to_paulis(z_string, x_string);
    this->phase = phase;
}

PauliArray::~PauliArray() {

}


vector<pauli_t> PauliArray::zx_to_paulis(const vector<uint8_t>& z_string, const vector<uint8_t>& x_string) const {
    int n = z_string.size();
    if (n != x_string.size()) {
    }
    vector<pauli_t> paulis(n, 0);
    for (int i = 0; i < n; i++) {
        paulis[i] = (z_string[i] << 1) | x_string[i];
    }
    return paulis;
}

int PauliArray::size() const {
    return paulis.size();
}

vector<pauli_t> PauliArray::get_paulis() const {
    return paulis;
}

vector<uint8_t> PauliArray::get_x_string() const {
    if (x_string.empty() && !paulis.empty()) {
        const_cast<vector<uint8_t>&>(x_string).resize(paulis.size());
        #pragma omp parallel for
        for (size_t i = 0; i < paulis.size(); i++) {
            const_cast<vector<uint8_t>&>(x_string)[i] = paulis[i] & 0x01;
        }
    }
    return x_string;
}



vector<uint8_t> PauliArray::get_z_string() const {
    if (z_string.empty() && !paulis.empty()) {
        const_cast<vector<uint8_t>&>(z_string).resize(paulis.size());
        #pragma omp parallel for
        for (size_t i = 0; i < paulis.size(); i++) {
            const_cast<vector<uint8_t>&>(z_string)[i] = (paulis[i] >> 1) & 0x01;
        }
    }
    return z_string;

}

void PauliArray::create_xz_strings() {
    int n = paulis.size();
    if (x_string.size() != n) x_string.resize(n);
    if (z_string.size() != n) z_string.resize(n);

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        uint8_t pauli = paulis[i];
        z_string[i] = (pauli >> 1) & 0x01;
        x_string[i] = pauli & 0x01;
    }
}


vector<bool> PauliArray::commutes(const PauliArray& other) {
    int n = this->size();
    if (n != other.size()) {
        throw std::invalid_argument("PauliArray sizes");
    }
    vector<bool> result(n, true);
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        // result[i] = product_table[this->paulis[i]][other.paulis[i]];
        uint8_t z1 = this->paulis[i] >> 1 & 0x01;
        uint8_t x1 = this->paulis[i] & 0x01;
        uint8_t z2 = other.paulis[i] >> 1 & 0x01;
        uint8_t x2 = other.paulis[i] & 0x01;
        result[i] = (z1 * x2 + x1 * z2) % 2 == 0;
    }
    return result;
     
}
// #ifdef USE_PYBIND11
py::array_t<uint8_t> PauliArray::commutes_numpy(const PauliArray& other) {
    // py::gil_scoped_release release;
    
    int n = this->size();
    if (n != other.size()) {
        throw std::invalid_argument("PauliArray sizes");
    }
    
    auto result = py::array_t<bool>(n);
    auto result_ptr = result.mutable_data();
    
    auto now = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        result_ptr[i] = product_table[this->paulis[i]][other.paulis[i]];
    }
    cout << "Commute C++ time : " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms" << endl;
    return result;
}
// #endif

std::ostream &operator<<(std::ostream &os, PauliArray const &pa) { 
    os << "pauliarray[size: " << pa.size() << ", string: [";
    for (int i=0; i< pa.size(); i++) {
        os << pauli_to_str[pa.paulis[i]];
    }
    os << "], phase: " << pa.phase << "]";
    return os;
}

PauliArray PauliArray::compose(const PauliArray& other) const {
    // int n = this->size();
    // vector<uint8_t> result(n, 0);
    // for (int i = 0; i < n; i++) {
    //     result[i] = paulis[i] ^ other.paulis[i];
    // }
    // int this_phase_power = inner_product(this->x_string.begin(), this->x_string.end(), other.z_string.begin(), 0);
    // int other_phase_power = inner_product(this->z_string.begin(), this->z_string.end(), other.x_string.begin(), 0);
    // //TODO: FINIR CA!
    // return result;

    if (this->size() != other.size()) {
        throw std::invalid_argument("PauliArray sizes");
    }

    int n = this->size();
    vector<uint8_t> new_z_strings(n, 0);
    vector<uint8_t> new_x_strings(n, 0);

    vector<uint8_t> this_phase_powers(n, 0);
    vector<uint8_t> other_phase_powers(n, 0);
    vector<uint8_t> new_phase_powers(n, 0);
    vector<uint8_t> commutation_phase_powers(n, 0);
    vector<uint8_t> phase_powers(n, 0);
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        new_z_strings[i] = (this->z_string[i] + other.z_string[i]) & 1; //371
        new_x_strings[i] = (this->x_string[i] + other.x_string[i]) & 1; //372

        this_phase_powers[i] = this->z_string[i] * this->x_string[i]; //374
        other_phase_powers[i] = other.z_string[i] * other.x_string[i]; //375
        new_phase_powers[i] = new_z_strings[i] * new_x_strings[i]; //376
        commutation_phase_powers[i] = 2*(this->x_string[i] * other.z_string[i]);

        phase_powers[i] = (2 * this->x_string[i] * other.z_string[i] +
                   this->z_string[i] * this->x_string[i] +
                   other.z_string[i] * other.x_string[i] -
                   new_z_strings[i] * new_x_strings[i]) & 3;
    }

    return PauliArray(move(new_z_strings), move(new_x_strings), complex<double>(1,0));
}

PauliArray PauliArray::tensor(const PauliArray& other) const {
    vector<pauli_t> result_vector(this->size(), 0);
    complex<double> tot_phase = this->phase * other.phase;
    for (int i = 0; i < this->size(); i++) {
        // pair<pauli_t, complex<double>> res = lookup_mult(this->paulis[i], other.paulis[i]);
        // tot_phase *= res.second;
        // result_vector[i] = res.first;
    }
    PauliArray result_array(result_vector);
    result_array.phase = tot_phase; 
    return result_array;
    
}



PauliArray PauliArray::random(int n) {
    vector<pauli_t> random_paulis;
    random_paulis.reserve(n);
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3);
    // cout << "Generating random PauliArray of size " << n << " ..." << endl;

    // #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        random_paulis.push_back(dis(gen));
        // random_paulis[i] = dis(gen);
    }
    return PauliArray(std::move(random_paulis));
}

PauliArray PauliArray::concatenate(const PauliArray& other) const {

    // vector<pauli_t> new_paulis(this->paulis.size() + other.paulis.size());
    // copy(this->paulis.begin(), this->paulis.end(), new_paulis.begin());
    // copy(other.paulis.begin(), other.paulis.end(), new_paulis.begin() + this->paulis.size());
    // return PauliArray(move(new_paulis));

    // //PARALLEL?
    size_t size1 = this->paulis.size();
    size_t size2 = other.paulis.size();
    vector<pauli_t> new_paulis(size1 + size2);

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (size_t i = 0; i < size1; i++) {
            new_paulis[i] = this->paulis[i];
        }

        #pragma omp for schedule(static)
        for (size_t i = 0; i < size2; i++) {
            new_paulis[size1 + i] = other.paulis[i];
        }
    }
    return PauliArray(move(new_paulis), 1);


}

