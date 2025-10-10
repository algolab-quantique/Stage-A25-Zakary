import time
import numpy as np
import matplotlib.pyplot as plt
import os
import sys


import pauliarray as pcpp
import pauliarray.binary.void_operations as c_vops

_old_pauli = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ""))
if _old_pauli not in sys.path:
    sys.path.insert(0, _old_pauli)


import old_pauliarray.pauliarray.binary.void_operations as np_vops


# print(f"PauliArray version: {pa.__version__}")
# print(f"Old PauliArray version: {old_pa.__version__}")


sizes = np.logspace(0, 7, 50, dtype=int)
# sizes= [2, 5, 10]

def pauli_py(voids1, voids2):
    start_time = time.time()

    r = np_vops.bitwise_dot(voids1, voids2)

    # print(r)
    
    end_time = time.time()
    return end_time - start_time, r

def pauli_cpp(voids1, voids2):
    start_time = time.time()

    r = c_vops.bitwise_dot(voids1, voids2)

    # print(r)

    end_time = time.time()
    return end_time - start_time, r

def main():
    # import pauliarray.binary.void_operations as vops
    # print("Using", vops.get_backend(), "backend.")
    start = time.time()
    py_times = []
    cpp_times = []
    results = []

    for size in sizes:
        print(f"\n\n========== Testing size: {size} ==========")
        void1 = np.random.randint(0, 2, size=(size,2), dtype=np.uint8)
        void2 = np.random.randint(0, 2, size=(size,2), dtype=np.uint8)
        void1 = c_vops.bit_strings_to_voids(void1)
        void2 = c_vops.bit_strings_to_voids(void2)

        print("---- Python ----")
        py_time, py_result = pauli_py(void1, void2)
        print(f"NPy execution time: {py_time:.7f} seconds")

        print("\n---- C++ ----")
        cpp_time, cpp_result = pauli_cpp(void1, void2)
        print(f"C++ execution time: {cpp_time:.7f} seconds")

        assert np.array_equal(py_result, cpp_result), "you fucked up brah!"

        py_times.append(py_time)
        cpp_times.append(cpp_time)
    
    end = time.time()
    print(f"\n\nTotal execution time for all sizes: {end - start:.7f} seconds")

    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, py_times, label='Python', marker='o')
    plt.plot(sizes, cpp_times, label='C++', marker='o')
    plt.xscale('log')
    plt.yscale('log')
    # plt.semilogx()
    plt.xlabel('Nbr of bits')
    plt.ylabel('Execution Time (seconds)')
    plt.title('NP bitwise voids V.S C++ bitwise voids\nFunction: ')
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.show()



if __name__ == "__main__":
    main()
