import numpy as np

import pauliarray.binary.void_operations as vops
import pauliarray as pa

NUM_QUBITS = 2
NUM_ROWS = 4


def transpose():
    ran = pa.PauliArray.random((NUM_ROWS,), NUM_QUBITS)
    z = ran.z_voids
    x = ran.x_voids
    # z, x = vops.random_zx_strings((1, NUM_QUBITS))

    z_int = vops.pretty_print_voids(z, NUM_QUBITS)
    
    t = vops.bitwise_transpose(z, NUM_QUBITS)
    t_int = vops.pretty_print_voids(t, z.shape[0])
    # t_int = np.squeeze(t_int, axis=1)

    t_t = vops.bitwise_transpose(t, z.shape[0])
    t_t_int = vops.pretty_print_voids(t_t, NUM_QUBITS)

    print("======================== Binary daya ========================")
    print("Original Z voids:")
    print(z)
    print("\nTransposed Z voidd:")
    print(t)
    print("\nDouble transposed Z voids:")
    print(t_t)

    print("\n\n======================== Int representation ========================")
    print("Original Z voids:")
    print(z_int)
    print("\nTransposed Z voids:")
    print(t_int)
    print("\nDouble transposed Z voids:")
    print(t_t_int)

    print("\n======================== Check equality ========================")
    print("Z dtype:", z.dtype)
    print("Transposed dtype:", t.dtype)
    print("Double transposed dtype:", t_t.dtype)
    print("\nCheck equality between original, and double transposed:", np.array_equal(z, t_t))


def matmul():
    ran = pa.PauliArray.random((NUM_ROWS,), NUM_QUBITS)
    z = ran.z_voids
    x = ran.x_voids

    ran2 = pa.PauliArray.random((NUM_QUBITS,), NUM_ROWS)
    z2 = ran2.z_voids
    x2 = ran2.x_voids

    print("\n\n======================== Int representation ========================")
    print("First Z voids:")
    print(vops.pretty_print_voids(z, NUM_QUBITS))
    print("\nSecond Z voids:")
    print(vops.pretty_print_voids(z2, NUM_ROWS))




    m = vops.bitwise_matmul(z, z2, NUM_QUBITS, NUM_ROWS)
    

    
    print("\nMatrix multiplication result:")
    print(vops.pretty_print_voids(m, NUM_ROWS))


def main():
    # transpose()
    matmul()






if __name__ == "__main__":
    main()


