import time
import qiskit
import matplotlib.pyplot as plt
import numpy as np

import qiskit_aer as q_aer
import pauliarray


from qiskit import transpile




def main():

    print("========== Python PauliArray ==========")
    start_time = time.time()

    # n= 1
    n = 750000000
    # n = 250000000
    # n = 10000000
    n2 = 1
    print("Working with n = ", n, "...")

    p1 = pauliarray.PauliArray.random((1,), n)
    p2 = pauliarray.PauliArray.random((1,), n)
    random_gen_time = time.time()
    print(f"Random genetation time: {((random_gen_time-start_time)*1000):.2f} ms")

    for i in range(n2):
        # pr = p1.tensor(p2)
        results = p1.commute_with(p2)
        # pr = p1.compose(p2)
        
    print("DONE!")


    end_time = time.time()
    duration = (end_time - start_time) * 1000
    print(f"Operation time: {((end_time - random_gen_time) * 1000):.2f} ms")
    print(f"Execution time: {duration:.2f} ms")


if __name__ == "__main__":
    main()
