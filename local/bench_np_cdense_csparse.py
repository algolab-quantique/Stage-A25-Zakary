import time
import numpy as np
import matplotlib.pyplot as plt

import os
import sys
import importlib
import traceback
import json

from pauliarray.src.build import sparsepaulicpp as spc
import pauliarray.binary.void_operations as c_vops
import pauliarray as pa


import os, sys
_old_pauli = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if _old_pauli not in sys.path:
    sys.path.insert(0, _old_pauli)

from old_pauliarray.pauliarray.binary import void_operations as np_vops


SIZE = 20
DENSITY = 0.05
PROXIMITY = 0.7
OPTION = "XOR"
sizes = np.logspace(0, 7, 50, dtype=int)
# sizes = np.logspace(1, 6, 50, dtype=int)
# sizes = [100]

# def random_bits(size):
#         a = np.random.randint(0, 2, size=size, dtype=np.uint8).tolist()
#         b = np.random.randint(0, 2, size=size, dtype=np.uint8).tolist()
#         return a, b

def f2d2s(f : float):
    # s = str(f).split(".")
    s = str(f).replace(".", ".")
    return s

def np_dense(void1, void2, option):
    start_time = time.time()
    res = None
    match option:
        case "AND":
            res = np_vops.bitwise_and(void1, void2)
        case "OR":
            res = np_vops.bitwise_or(void1, void2)
        case "XOR":
            res = np_vops.bitwise_xor(void1, void2)
        case "NOT":
            res = np_vops.bitwise_not(void1)
        case "DOT":
            res = np_vops.bitwise_dot(void1, void2)
        case "COUNT":
            res = np_vops.bitwise_count(void1)
        case _:
            print(f"Unknown option: {option}")


    end_time = time.time()
    return end_time - start_time, res

def dense(void1, void2, option):
    start_time = time.time()
    res = None
    match option:
        case "AND":
            res = c_vops.bitwise_and(void1, void2)
        case "OR":
            res = c_vops.bitwise_or(void1, void2)
        case "XOR":
            res = c_vops.bitwise_xor(void1, void2)
        case "NOT":
            res = c_vops.bitwise_not(void1)
        case "DOT":
            res = c_vops.bitwise_dot(void1, void2)
        case "COUNT":
            res = c_vops.bitwise_count(void1)
        case _:
            print(f"Unknown option: {option}")


    end_time = time.time()
    return end_time - start_time, res


def sparse(dpoint1, dpoint2, option):
    start_time = time.time()
    res = None
    match option:
        case "AND":
            res = spc.and_dpoint(dpoint1, dpoint2)
        case "OR":
            res = spc.or_dpoint(dpoint1, dpoint2)
        case "XOR":
            res = spc.xor_dpoint(dpoint1, dpoint2)
        case "NOT":
            res = spc.not_dpoint(dpoint1, )
        # case "DOT":
        #     res = spc.dot_dpoint(dpoint1, dpoint2)
        case "COUNT":
            res = spc.popcount_dpoint(dpoint1)
        case _:
            print(f"Unknown option: {option}")


    end_time = time.time()
    return end_time - start_time, res
    


def main():
    numpy_times = []
    sparse_times = []
    dense_times = []
    results = []
    assert c_vops.get_backend() == "C++", "C++ backend not set!"

    for size in sizes:
        print(f"\n\n========== Testing size: {size} ==========")
        ran_bits1 = spc.generate_random_vec(size, DENSITY, PROXIMITY)
        ran_bits2 = spc.generate_random_vec(size, DENSITY, PROXIMITY)
        # ran_bits3 = spc.generate_random_vec(size, DENSITY, PROXIMITY)
        # ran_bits4 = spc.generate_random_vec(size, DENSITY, PROXIMITY)

        print("---- Numpy Dense ----")
        arr1 = np_vops.bit_strings_to_voids(ran_bits1)
        arr2 = np_vops.bit_strings_to_voids(ran_bits2)
        np_dense_time, np_dense_result = np_dense(arr1, arr2, OPTION)
        print(f"NPy execution time: {np_dense_time:.7f} seconds")
    

        print("\n---- Dense ----")
        arr1 = c_vops.bit_strings_to_voids(ran_bits1)
        arr2 = c_vops.bit_strings_to_voids(ran_bits2)
        dense_time, dense_result = dense(arr1, arr2, OPTION)
        print(f"Dense execution time: {dense_time:.7f} seconds")


        print("\n---- Sparse ----")
        dpoint1 = spc.make_dpoint(ran_bits1)
        dpoint2 = spc.make_dpoint(ran_bits2)
        # print("first dpoint:")
        # spc.show_dpoints(dpoint1)
        # print("\nsecond dpoint:")
        # spc.show_dpoints(dpoint2)
        sparse_time, sparse_result = sparse(dpoint1, dpoint2, OPTION)
        # print("\nresult dpoint:")
        # spc.show_dpoints(sparse_result)
        print(f"C++ execution time: {sparse_time:.7f} seconds")


        numpy_times.append(np_dense_time)
        dense_times.append(dense_time)
        sparse_times.append(sparse_time)
        # results.append((py_result, cpp_result))
    

    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, numpy_times, label='Numpy Dense', marker='o')
    plt.plot(sizes, dense_times, label='Dense', marker='o')
    plt.plot(sizes, sparse_times, label='Sparse', marker='o')
    plt.xscale('log')
    plt.yscale('log')
    # plt.semilogx()
    plt.xlabel('Nbr of bits')
    plt.ylabel('Execution Time (seconds)')
    plt.title(f'Dense vs Sparse on {OPTION} operation\nWith Density={DENSITY}, Proximity={PROXIMITY}')
    plt.legend()
    plt.grid()
    plt.tight_layout()

    plt.savefig(f"results/np_cdense_csparse/voids_{OPTION}_density{f2d2s(DENSITY)}_proximity{f2d2s(PROXIMITY)}.png")
    plt.show()



   
    

if __name__ == "__main__":
    main()