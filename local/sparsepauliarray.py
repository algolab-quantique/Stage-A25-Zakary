from scipy.sparse import csr_array
import pauliarray as pa
import numpy as np

shape = (4, 4)
num_qubits = 2

class SparsePauliArray:
    
    def __init__(self):
        pass

def main():
    p1 = pa.PauliArray.random(shape, num_qubits)
    p2 = pa.PauliArray.random(shape, num_qubits)



if __name__ == "__main__":
    main()