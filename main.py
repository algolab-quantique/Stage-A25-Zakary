import time
import numpy as np
import matplotlib.pyplot as plt

import pauliarray.binary.void_operations as vo_py
import voidops as vo_cpp
# sizes = [100, 1000, 10000, 100000, 500000, 1000000, 5000000, 10000000, 100000000, 1000000000]
sizes = np.logspace(0, 9, 50, dtype=int)
# sizes = np.logspace(0, 8, 50, dtype=int)
# sizes = [3*10**9]
# sizes = [10, 30, 50]

def void_py(bit_strings_1, bit_strings_2):
    voids_1 = vo_py.bit_strings_to_voids(bit_strings_1)
    voids_2 = vo_py.bit_strings_to_voids(bit_strings_2)
    start_time = time.time()

    # r = vo_py.bitwise_dot(voids_1, voids_2)
    # r = vo_py.bitwise_count(voids_1) + vo_py.bitwise_count(voids_2)
    # r = vo_py.bitwise_and(voids_1, voids_2)
    r = vo_py.bitwise_xor(voids_1, voids_2)
    # r = vo_py.bitwise_not(voids_1)

    end_time = time.time()
    return end_time - start_time, r

def void_cpp(bit_strings_1, bit_strings_2):
    voids_1 = vo_py.bit_strings_to_voids(bit_strings_1)
    voids_2 = vo_py.bit_strings_to_voids(bit_strings_2)
    start_time = time.time()

    # r = vo_cpp.bitwise_dot(voids_1, voids_2)
    # r = vo_cpp.bitwise_count(voids_1) + vo_cpp.bitwise_count(voids_2)
    # r = vo_cpp.bitwise_and(voids_1, voids_2)
    r = vo_cpp.bitwise_xor(voids_1, voids_2)
    # r = vo_cpp.bitwise_not(voids_1)


    end_time = time.time()
    return end_time - start_time, r

def main():
    py_times = []
    cpp_times = []
    results = []

    for size in sizes:
        print(f"\n========== Testing size: {size} ==========")
        bit_strings_1 = np.random.randint(0, 2, size=(size,), dtype=np.uint8)
        bit_strings_2 = np.random.randint(0, 2, size=(size,), dtype=np.uint8)

        print("---- Python ----")
        py_time, py_result = void_py(bit_strings_1, bit_strings_2)
        print(f"NPy execution time: {py_time:.7f} seconds")
        # print(f"Result: {py_result}")

        print("---- C++ ----")
        cpp_time, cpp_result = void_cpp(bit_strings_1, bit_strings_2)
        print(f"C++ execution time: {cpp_time:.7f} seconds")
        # print(f"Result: {cpp_result}")
        assert np.array_equal(py_result, cpp_result), "you fucked up brah!"

        py_times.append(py_time)
        cpp_times.append(cpp_time)
        results.append((py_result, cpp_result))

    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, py_times, label='Python', marker='o')
    plt.plot(sizes, cpp_times, label='C++', marker='o')
    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Input Size')
    plt.ylabel('Execution Time (seconds)')
    plt.title('NP bitwise voids V.S C++ bitwise voids')
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()