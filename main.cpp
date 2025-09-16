#include <chrono>
#include <iostream>


#include "pauliarray.hpp"
#include "sparsepauliarray.hpp"
#include "densepauliarray.hpp"

using namespace std;

int n = 500000000;

void test_pauliarray(){
    // vector<uint8_t> p = {I, X, Y, Z};
    // PauliArray pauli_array(p);

    // vector<uint8_t> p2 = {I, Y, X, Z};
    // PauliArray pauli_array2(p2);
    // cout << "Taille de l'array : " << pauli_array.size() << endl;
    // vector<bool> results = pauli_array.commutes(pauli_array2);
    // for (int i=0; i<pauli_array.size(); i++) {
    //     cout << "Pauli " << i << " commutes ? " << boolalpha << results[i] << endl;
    // }
    // cout << pauli_array << endl;
    // cout << pauli_array2 << endl;

    
    
    cout << "========== C++ PauliArray ==========" << endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    int n = 500000000;
    // int n = 250000000;
    // int n = 100000000;
    int n2 = 1;
    cout << "Working with n = " << n << " ..." << endl;
    PauliArray p1 = PauliArray::random(n);
    PauliArray p2 = PauliArray::random(n);

    auto time_rand = std::chrono::high_resolution_clock::now();
    auto duration_rand = std::chrono::duration_cast<std::chrono::milliseconds>(time_rand - start_time);
    std::cout << "Random generation time: " << duration_rand.count() << " ms" << std::endl;
    
    // auto pr = p1.concatenate(p2); //non mais avant oui
    auto pr = p1.commutes(p2); //OK!
    // auto pr = p1.compose(p2); //non

    // cout << pr << endl;
    cout << "DONE!" << endl;


    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_exec = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - time_rand);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    cout << "Operation time: " << duration_exec.count() << " ms" << std::endl;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
}

void test_spa(){
    cout << "========== C++ SparsePauliArray ==========" << endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    // int n = 500000000;
    // int n = 250000000;
    // int n = 100000000;
    // int n = 100;
    float density = 0.5;
    int n2 = 1;
    cout << "Working with n = " << n << " ..." << endl;

    SparsePauliArray p1(n, true, density);
    SparsePauliArray p2(n, true, density);

    auto time_rand = std::chrono::high_resolution_clock::now();
    auto duration_rand = std::chrono::duration_cast<std::chrono::milliseconds>(time_rand - start_time);
    std::cout << "Random generation time: " << duration_rand.count() << " ms" << std::endl;
    
    
    // cout << p1 << endl;
    // cout << p2 << endl;
    
    
    auto pr = p1.commutes(p2); 


    cout << "DONE!" << endl;
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_exec = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - time_rand);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    cout << "Operation time: " << duration_exec.count() << " ms" << std::endl;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;

}

void test_dpa(){
    cout << "========== C++ DensePauliArray ==========" << endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    int n = 500000000;
    // int n = 250000000;
    // int n = 100000000;
    int n2 = 1;
    cout << "Working with n = " << n << " ..." << endl;

    // SparsePauliArray p2(n, true, density);

    auto time_rand = std::chrono::high_resolution_clock::now();
    auto duration_rand = std::chrono::duration_cast<std::chrono::milliseconds>(time_rand - start_time);
    std::cout << "Random generation time: " << duration_rand.count() << " ms" << std::endl;
    
    
    // cout << p1 << endl;
    // cout << p2 << endl;
    
    
    // auto pr = p1.commutes(p2);
}


int main() {
    // test_pauliarray();
    // test_spa();
    test_dpa();
    


    return 0;
}