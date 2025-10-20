import numpy as np

from pauliarray.src.build import sparsepaulicpp as spc
import pauliarray.binary.void_operations as c_vops
import pauliarray as pa

shape = (10,)

def main():
    ran_bits1 = np.random.randint(0, 2, size=shape, dtype=np.uint8)
    ran_bits2 = np.random.randint(0, 2, size=shape, dtype=np.uint8)
    print("Random bits 1:", ran_bits1)
    print("Random bits 2:", ran_bits2)

    s1 = spc.make_dpoint_set(ran_bits1)
    s2 = spc.make_dpoint_set(ran_bits2)
    print("Dpoint set 1:", s1)
    print("Dpoint set 2:", s2)
    # print(type(t))
    res = spc.xor_set(s1, s2)
    print("XOR result:", res)



if __name__ == "__main__":
    main()


