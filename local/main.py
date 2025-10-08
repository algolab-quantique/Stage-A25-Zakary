import time
import numpy as np
import matplotlib.pyplot as plt
import pauliarray.binary.void_operations as vops

import os
import sys
import importlib
import traceback
import json

from pauliarray.src.build import sparsepaulicpp as spc


def main():
    with open("voids.json", "r") as f:
        data = json.load(f)
        zs = data["Z"]
        xs = data["X"]
    ran_bits1 = zs[-1]
    ran_bits2 = xs[-1]
    # ran_bits1 = np.random.randint(0, 2, size=(10,), dtype=np.uint8).tolist()
    # ran_bits2 = np.random.randint(0, 2, size=(10,), dtype=np.uint8).tolist()
    dpoint1 = spc.make_dpoint(ran_bits1)
    dpoint2 = spc.make_dpoint(ran_bits2)

    print("Random bits 1:", ran_bits1)
    print("Random bits 2:", ran_bits2)
    print("Dpoint 1:", dpoint1)
    print("Dpoint 2:", dpoint2)
    spc.overlap_dpoint(dpoint1, dpoint2)
    print("XOR:", spc.xor_dpoint(dpoint1, dpoint2))
    print("AND:", spc.and_dpoint(dpoint1, dpoint2))
    print("OR:", spc.or_dpoint(dpoint1, dpoint2))


   
    

if __name__ == "__main__":
    main()