import numpy as np
import pauliarray as pa
import pauliarray.binary.void_operations as vops
import pauliarray.binary.bit_operations as bops

import z2r_accel as z2


NUM_QUBITS = 9
NUM_ROWS = 3


def matmul():
    ran1 = pa.PauliArray.random((NUM_ROWS,), NUM_QUBITS)
    z1 = ran1.z_voids
    x1 = ran1.x_voids

    ran_mat = np.array([[1, 0, 1], [0, 1, 1], [1, 1, 0]], dtype=np.uint8)
    ran_mat2 = np.array([[0, 1, 1], [1, 0, 1], [1, 1, 0]], dtype=np.uint8)
    # z1_i = vops.voids_to_int_strings(z1)
    # x1_i = vops.voids_to_int_strings(x1)
    # print(z1_i)
    print("========== Python ==========")
    res_py = bops.matmul(ran_mat, ran_mat2)
    print(res_py)

    print("========== Z2 ==========")
    ran1_void = vops.bit_strings_to_voids(ran_mat)
    ran2_void = vops.bit_strings_to_voids(ran_mat2)
    res_z2 = z2.matmul(ran1_void, ran2_void, NUM_QUBITS, NUM_QUBITS)
    print(vops.voids_to_bit_strings(res_z2, NUM_QUBITS))

    # print("Result from z2r_accel.matmul:")
    # print(res)

    # print("Input PauliArray:")
    # print(ran1.inspect())


def concatenate():
    # ran1 = pa.PauliArray.random((NUM_ROWS,), NUM_QUBITS)
    # z1 = ran1.z_voids
    # x1 = ran1.x_voids

    # res = z2.concatenate(z1, x1)
    # print("Concatenated voids:")
    # print(z1)
    # print(x1)
    # print(res)

    # z1_i = vops.voids_to_bit_strings(z1, NUM_QUBITS)
    # x1_i = vops.voids_to_bit_strings(x1, NUM_QUBITS)
    # res_i = vops.voids_to_bit_strings(res, NUM_QUBITS)

    z1_i = np.array([[0, 0, 0], [0, 0, 0], [0, 0, 0]], dtype=np.uint8)
    x1_i = np.array([[1, 1, 1], [1, 1, 1], [1, 1, 1]], dtype=np.uint8)

    z1 = vops.bit_strings_to_voids(z1_i).reshape(-1, 1)
    x1 = vops.bit_strings_to_voids(x1_i).reshape(-1, 1)
    # print("Z1 voids:")
    # print(z1)

    res = z2.concatenate(z1, x1, axis=1)
    # res_i = vops.voids_to_bit_strings(res, NUM_QUBITS*2)
    res_i = z2.z2_to_uint8(res, NUM_QUBITS * 2)

    print("Concatenated bit strings:")
    print("\nZ1:")
    print(z1_i)
    print("\nX1:")
    print(x1_i)
    print("\nResult:")
    print(res_i)


# PAS MOI!
def gauss_inv(num_qubits=NUM_QUBITS, trials=20):
    import numpy as np
    import pauliarray.binary.void_operations as vops
    import pauliarray.binary.bit_operations as bops
    import z2r_accel as z2

    def random_invertible(n):
        # Retry until invertible
        while True:
            A = np.random.randint(0, 2, (n, n), dtype=np.uint8)
            A_void = vops.bit_strings_to_voids(A)
            try:
                inv_void = z2.gauss_jordan_inverse(A_void, n)
                return A, inv_void
            except RuntimeError:
                continue  # singular, retry

    for _ in range(trials):
        A_bits, inv_void = random_invertible(num_qubits)
        inv_bits = vops.voids_to_bit_strings(inv_void, num_qubits)
        prod = bops.matmul(A_bits, inv_bits)
        if not np.array_equal(prod, np.eye(num_qubits, dtype=np.uint8)):
            print("FAIL:")
            print("A:\n", A_bits.astype(np.uint8))
            print("A^{-1}:\n", inv_bits.astype(np.uint8))
            print("A*A^{-1}:\n", prod.astype(np.uint8))
        else:
            print("OK")
            print("A:\n", A_bits.astype(np.uint8))
            print("A^{-1}:\n", inv_bits.astype(np.uint8))
            print("A*A^{-1}:\n", prod.astype(np.uint8))

    # Singular case check
    singular = np.zeros((num_qubits, num_qubits), dtype=np.uint8)
    singular_void = vops.bit_strings_to_voids(singular)
    try:
        z2.gauss_jordan_inverse(singular_void, num_qubits)
        print("ERROR: singular matrix inverted unexpectedly")
    except RuntimeError:
        print("Singular matrix correctly raised RuntimeError")


# def test():
#     u = " une "
#     print("Voici" +u "erreur")


def main():
    # matmul()
    # concatenate()
    gauss_inv()
    # test()


if __name__ == "__main__":
    main()
