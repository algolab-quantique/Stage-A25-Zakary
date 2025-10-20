import time
import numpy as np
import matplotlib.pyplot as plt
import os
import sys
import json


from pauliarray.src.build import sparsepaulicpp as spc
import pauliarray.binary.void_operations as c_vops
import pauliarray as pa

_old_pauli = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if _old_pauli not in sys.path:
    sys.path.insert(0, _old_pauli)

from old_pauliarray.pauliarray.binary import void_operations as np_vops # pyright: ignore[reportMissingImports]


DENSITY = 0.5
PROXIMITY = 0.5
OPTION = "XOR"
# LIBS = ["NP", "C_DENSE", "C_SPARSE"]
LIBS = ["NP", "C_DENSE"]
sizes = np.logspace(0, 8, 50, dtype=int)
VERBOSE = False
shapes = [(s,) for s in sizes]
# sizes = [2, 5, 10]



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

    if VERBOSE:
        print("Result:\n", res)
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


    if VERBOSE:
        print("Result:\n", res)
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
            res = spc.count_dpoint(dpoint1)
        case _:
            print(f"Unknown option: {option}")


    end_time = time.time()
    return end_time - start_time, res
    


def main():
    numpy_times = []
    sparse_times = []
    dense_times = []

    start = time.time()
    assert c_vops.get_backend() == "C++", "C++ backend not set!"

    for shape in shapes:
        print(f"\n\n========== Testing size: {shape} ==========")
        if "C_SPARSE" in LIBS:
            ran_bits1 = spc.generate_random_vec(shape[-1], DENSITY, PROXIMITY)
            ran_bits2 = spc.generate_random_vec(shape[-1], DENSITY, PROXIMITY)
        else:
            # ran_bits1 = np.random.randint(0, 2, size=shape, dtype=np.uint8)
            # ran_bits2 = np.random.randint(0, 2, size=shape, dtype=np.uint8)
            ran_bits1, ran_bits2 = c_vops.random_zx_strings(shape)
        if VERBOSE:
            print("ran_bits1: \n",ran_bits1)
            print("ran_bits2: \n",ran_bits2)
            # print("random1: \n",ran_bits1)
            # print("random2: \n",ran_bits2)

        if "NP" in LIBS:
            print("---- Numpy Dense ----")
            arr1 = np_vops.bit_strings_to_voids(ran_bits1)
            arr2 = np_vops.bit_strings_to_voids(ran_bits2)
            np_dense_time, np_dense_result = np_dense(arr1, arr2, OPTION)
            print(f"NPy execution time:    {np_dense_time:.7f} seconds")
            numpy_times.append(np_dense_time)
    
        if "C_DENSE" in LIBS:
            print("\n---- Dense ----")
            arr1 = c_vops.bit_strings_to_voids(ran_bits1)
            arr2 = c_vops.bit_strings_to_voids(ran_bits2)
            dense_time, dense_result = dense(arr1, arr2, OPTION)
            print(f"Dense execution time:  {dense_time:.7f} seconds")
            dense_times.append(dense_time)


        if "C_SPARSE" in LIBS:
            print("\n---- Sparse ----")
            dpoint1 = spc.make_dpoint(ran_bits1)
            dpoint2 = spc.make_dpoint(ran_bits2)

            # print("random1: \n",ran_bits1)
            # print("random2: \n",ran_bits2)
            # print("\ndpoint1:")
            # spc.show_dpoints(dpoint1)
            # print("dpoint2:")
            # spc.show_dpoints(dpoint2)
            # spc.overlap_dpoint(dpoint1, dpoint2)
            sparse_time, sparse_result = sparse(dpoint1, dpoint2, OPTION)
            # print("\nresult dpoint:")
            # spc.show_dpoints(sparse_result)
            print(f"Sparse execution time: {sparse_time:.7f} seconds")
            sparse_times.append(sparse_time)

        if "NP" in LIBS and "C_DENSE" in LIBS:
            assert np.array_equal(np_dense_result, dense_result), "Results do not match between Numpy and C_Dense!"

        # results.append((py_result, cpp_result))
    
    end = time.time()
    print(f"\n\nTotal execution time for all sizes: {end - start:.7f} seconds")

    # Plotting
    plt.figure(figsize=(10, 6))
    if "NP" in LIBS:
        plt.plot(sizes, numpy_times, label='Numpy Dense', marker='o')
    if "C_DENSE" in LIBS:
        plt.plot(sizes, dense_times, label='Dense', marker='o')
    if "C_SPARSE" in LIBS:
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
    # plt.plot(sizes, sizes, 'k--', label='O(n)')
    # plt.plot(sizes, sizes**2, 'k--', label='O(n)')

    if os.path.exists("results/np_cdense_csparse"):
        plt.savefig(f"results/np_cdense_csparse/voids_{OPTION}_density{f2d2s(DENSITY)}_proximity{f2d2s(PROXIMITY)}.png")
    plt.show()



   
    

if __name__ == "__main__":
    main()