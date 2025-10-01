import time
import matplotlib.pyplot as plt
import numpy as np

import pauliarray as pa
import paulicpp as pc

# sizes = np.logspace(1, 8, 50, dtype=int)
sizes= [2, 5]

def pauli_py(p1: pa.PauliArray, p2: pa.PauliArray):
    start_time = time.time()
    r = p1.compose(p2)
    end_time = time.time()
    print(r)

    return end_time - start_time, r

def pauli_cpp(p1: pa.PauliArray, p2: pa.PauliArray):
    start_time = time.time()
    z1 = p1.z_voids
    x1 = p1.x_voids
    z2 = p2.z_voids
    x2 = p2.x_voids
    new_z, new_x = pc.compose(z1, x1, z2, x2)
    r = pa.PauliArray.from_z_strings_and_x_strings(new_z, new_x)

    end_time = time.time()
    print(r)
    return end_time - start_time, r

def main():
    py_times = []
    cpp_times = []
    results = []

    for size in sizes:
        print(f"\n========== Testing size: {size} ==========")
        p1 = pa.PauliArray.random((size,), 2)
        p2 = pa.PauliArray.random((size,), 2)


        print("---- Python ----")
        py_time, py_result = pauli_py(p1, p2)
        print(f"NPy execution time: {py_time:.7f} seconds")

        print("---- C++ ----")
        cpp_time, cpp_result = pauli_cpp(p1, p2)
        print(f"C++ execution time: {cpp_time:.7f} seconds")

        # assert np.array_equal(py_result, cpp_result), "you fucked up brah!"

if __name__ == "__main__":
    main()