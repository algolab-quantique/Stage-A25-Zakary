import time
import qiskit
import matplotlib.pyplot as plt
import numpy as np

import pauliarray

import paulicpp as pc
import densepaulicpp as dpc



# n = 250000000
n = 1000000
dim = 500
# n = 50000000



def pa_py():
    print("========== PauliArray (Voids) w/ NumPy ==========")
    start_time = time.time()
    print("Working with n = ", n, "...")

    shape = (dim,)

    p1 = pauliarray.PauliArray.random(shape, n)
    p2 = pauliarray.PauliArray.random(shape, n)

    random_gen_time = time.time()
    print(f"Random genetation time: {((random_gen_time-start_time)*1000):.2f} ms")

    # print(p1.x_strings) 

    # results = p1.commute_with(p2)

    # results = p1.compose(p2)
    results = p1.tensor(p2)
    print(results)

    print(p1)
    print("DONE!")


    end_time = time.time()
    duration = (end_time - start_time) * 1000
    print(f"Operation time: {((end_time - random_gen_time) * 1000):.2f} ms")
    print(f"Execution time: {duration:.2f} ms")




def pa_cpp():
    print("========== Python PauliArray w/ C++ libraries==========")
    start_time = time.time()
    print("Working with n = ", n, "...")

    p1 = pc.PauliArray.random(n)
    p2 = pc.PauliArray.random(n)

    random_gen_time = time.time()
    print(f"Random genetation time: {((random_gen_time-start_time)*1000):.2f} ms")

    results = p1.commutes_numpy(p2)
        
    print("DONE!")


    end_time = time.time()
    duration = (end_time - start_time) * 1000
    print(f"Operation time: {((end_time - random_gen_time) * 1000):.2f} ms")
    print(f"Execution time: {duration:.2f} ms")


def dpa_cpp():
    print("========== Python DensePauliArray w/ C++ libraries==========")
    start_time = time.time()
    print("Working with n = ", n, "...")

    p1 = dpc.DensePauliArray.random(dim, n)
    p2 = dpc.DensePauliArray.random(dim, n)


    random_gen_time = time.time()
    print(f"Random genetation time: {((random_gen_time-start_time)*1000):.2f} ms")

    # print(p1.to_string())
    # print(p2.to_string())
    # results = p1.commutes_batch(p2)
    pr = p1.tensor(p2)
    # print(pr.to_string())
        
    print("DONE!")


    end_time = time.time()
    duration = (end_time - start_time) * 1000
    print(f"Operation time: {((end_time - random_gen_time) * 1000):.2f} ms")
    print(f"Execution time: {duration:.2f} ms")

def main():
    pa_py()
    print("\n\n")
    # pa_cpp()
    dpa_cpp()
    

if __name__ == "__main__":
    main()
