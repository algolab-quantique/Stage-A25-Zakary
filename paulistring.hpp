#include <iostream>
#include <cstdint> // uint8_t
#include <vector>
#include <bitset>

using namespace std;
using pauli_t = uint8_t;



class PauliString {
    public:
        bool commutes(const PauliString& other) const;

    private:
};

bool PauliString::commutes(const PauliString& other) const {
    return true;
}