import time
import numpy as np
import matplotlib.pyplot as plt
import os
import sys

import pauliarray as pa

_old_pauli = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ""))
if _old_pauli not in sys.path:
    sys.path.insert(0, _old_pauli)


from old_pauliarray.pauliarray.pauli import pauli_array as old_pa
from pauliarray.pauli import pauli_array as pcpp


# print(f"PauliArray version: {pcpp.__version__}")
# print(f"Old PauliArray version: {old_pa.__version__}")


OPTION = "UNIQUE"
LIBS = ["UNIQUE", "COMMUTE", "TENSOR"]
sizes = np.logspace(1, 7, 30, dtype=int)
length = 5

# sizes = [2, 5, 10]
# shapes = [(int(s),) for s in np.logspace(0, 7, num=10)]

# shapes = [(size,) for size in sizes]
VERBOSE = False

def pauli_py(p1: pcpp.PauliArray, p2: pcpp.PauliArray):
    start_time = time.time()
    res = None
    match OPTION:
        case "COMMUTE":
            res = old_pa.bitwise_commute_with(p1, p2)

        case "UNIQUE":
            res = old_pa.unique(p1)
            print("Nbr unique:", res.shape[0])
            if VERBOSE:
                print("Result:", res.inspect())

        case "TENSOR":
            res = old_pa.tensor(p1, p2)

        case _:
            print(f"Unknown option: {OPTION}")

    
    end_time = time.time()
    return end_time - start_time, res



def pauli_cpp(p1: pcpp.PauliArray, p2: pcpp.PauliArray):
    start_time = time.time()

    res = None
    match OPTION:
        case "COMMUTE":
            res = pcpp.bitwise_commute_with(p1, p2)

        case "UNIQUE":
            res = pcpp.unordered_unique(p1)
            # res = pcpp.unique(p1)
            print("Nbr unique:", res.shape[0])

            if VERBOSE:
                print("Result:", res.inspect())

        case "TENSOR":
            res = pcpp.tensor(p1, p2)

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

    for size in sizes:

        print(f"\n\n========== Testing size: {size} ==========")
        shape = (size,)

        p1 = pcpp.PauliArray.random(shape, length)
        p2 = pcpp.PauliArray.random(shape, length)

        # p1 = pcpp.PauliArray.random_cpp(shape)
        # p2 = pcpp.PauliArray.random_cpp(shape)
        # print(p1.inspect())

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
        
    end = time.time()
    print(f"\n\nTotal execution time for all sizes: {end - start:.7f} seconds")

    plt.figure(figsize=(10, 6))

    plt.plot(sizes, py_times, label='NumPy', marker='o')
    plt.plot(sizes, cpp_times, label='C++', marker='o')

    plt.xscale('log')  
    plt.yscale('log')
    # plt.semilogx()
    plt.xlabel('Nbr of bits')
    plt.ylabel('Execution Time (seconds)')
    plt.title(f'NP functions V.S C++\nFunction: {OPTION}')
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.show()



if __name__ == "__main__":
    main()
