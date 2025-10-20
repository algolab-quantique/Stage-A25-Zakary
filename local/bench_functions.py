import time
import numpy as np
import matplotlib.pyplot as plt
import os
import sys

import pauliarray as pcpp

_old_pauli = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ""))
if _old_pauli not in sys.path:
    sys.path.insert(0, _old_pauli)


import old_pauliarray.pauliarray as pa
from old_pauliarray.pauliarray.pauli import pauli_array as np_pa_fn
from pauliarray.pauli import pauli_array as pcpp_pa_fn


# print(f"PauliArray version: {pa.__version__}")
# print(f"Old PauliArray version: {old_pa.__version__}")


OPTION = "UNIQUE"
LIBS = ["UNIQUE", "COMMUTE", "TENSOR"]
# sizes = np.logspace(0, 6, 50, dtype=int)
sizes = [2]
shapes = [(size,) for size in sizes]
VERBOSE = True

def pauli_py(p1: pa.PauliArray, p2: pa.PauliArray):
    start_time = time.time()
    res = None
    match OPTION:
        case "COMMUTE":
            res = np_pa_fn.bitwise_commute_with(p1, p2)
        case "UNIQUE":
            res = np_pa_fn.unique(p1)
            if VERBOSE:
                print("Result:", res.inspect())
        case "TENSOR":
            res = np_pa_fn.tensor(p1, p2)
        case _:
            print(f"Unknown option: {OPTION}")

    
    end_time = time.time()
    return end_time - start_time, res



def pauli_cpp(p1: pa.PauliArray, p2: pa.PauliArray):
    start_time = time.time()

    res = None
    match OPTION:
        case "COMMUTE":
            res = pcpp_pa_fn.bitwise_commute_with(p1, p2)
        case "UNIQUE":
            uniques = pcpp_pa_fn.unordered_unique(p1)

            if VERBOSE:
                # print("Input:", p1.inspect())
                # print(uniques.inspect())
                # print(index)
                # print(inverse)
                print("Result:", res.inspect())

        case "TENSOR":
            res = pcpp_pa_fn.tensor(p1, p2)
        case _:
            print(f"Unknown option: {OPTION}")
    
    end_time = time.time()
    return end_time - start_time, res




def main():
    # import pauliarray.binary.void_operations as vops
    # print("Using", vops.get_backend(), "backend.")
    start = time.time()
    py_times = []
    cpp_times = []
    results = []

    i = 0
    for i in range(len(sizes)):
        print(f"\n\n========== Testing size: {sizes[i]} ==========")

        p1 = pa.PauliArray.random(shapes[i], sizes[i])
        p2 = pa.PauliArray.random(shapes[i], sizes[i])
     # p1 = pa.PauliArray.random((4, size,2), 2)
        # p2 = pa.PauliArray.random((4, size,2), 2)

        print("---- Python ----")
        py_time, py_result = pauli_py(p1, p2)
        print(f"NPy execution time: {py_time:.7f} seconds")

        print("\n---- C++ ----")
        cpp_time, cpp_result = pauli_cpp(p1, p2)
        print(f"C++ execution time: {cpp_time:.7f} seconds")

        # assert np.array_equal(py_result, cpp_result), "you fucked up brah!"

        py_times.append(py_time)
        cpp_times.append(cpp_time)
        results.append((py_result, cpp_result))
        i += 1
    
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
