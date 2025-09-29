# %%

import cProfile
import io
import pstats
import time
from pstats import SortKey

import numpy as np
from pauliarray.conversion.qiskit import operator_to_sparse_pauli
from pauliarray.mapping.fermion import JordanWigner
from qiskit_nature.second_q.drivers import PySCFDriver


def h2_labels_positions():
    distance_hh = 0.735
    atom_labels = ["H", "H"]
    positions = np.array([[0, 0, -distance_hh / 2], [0, 0, distance_hh / 2]])

    return atom_labels, positions


def lih_labels_positions():

    distance_lih = 1.6

    atom_labels = ["Li", "H"]
    positions = np.array([[0, 0, 0], [0, 0, distance_lih]])

    return atom_labels, positions


def h2o_labels_positions():

    angle_hoh = 104.5
    distance_oh = 0.9584

    atom_labels = ["O", "H", "H"]

    theta = (np.pi - angle_hoh) / 2
    distance_oh_x = distance_oh * np.cos(theta)
    distance_oh_y = distance_oh * np.sin(theta)

    positions = np.array([[0, 0, 0], [-distance_oh_x, -distance_oh_y, 0], [distance_oh_x, -distance_oh_y, 0]])

    return atom_labels, positions


def nh3_labels_positions():

    atom_labels = ["N", "H", "H", "H"]

    distance_nh, angle_hnh, phi = 1.017, 107.8 / 180 * np.pi, 0

    factor = (1 + 2 * np.sin(angle_hnh)) / 3
    height = distance_nh * np.sqrt(factor)
    rho = distance_nh * np.sqrt(1 - factor)
    rho_x_0 = rho * np.cos(phi)
    rho_y_0 = rho * np.sin(phi)
    rho_x_1 = rho * np.cos(2 * np.pi / 3 + phi)
    rho_y_1 = rho * np.sin(2 * np.pi / 3 + phi)
    rho_x_2 = rho * np.cos(-2 * np.pi / 3 + phi)
    rho_y_2 = rho * np.sin(-2 * np.pi / 3 + phi)
    positions = np.array([[0, 0, height], [rho_x_0, rho_y_0, 0], [rho_x_1, rho_y_1, 0], [rho_x_2, rho_y_2, 0]])

    return atom_labels, positions


def c2h2_labels_positions():

    distance_cc = 1.203
    distance_ch = 1.06

    atom_labels = ["C", "C", "H", "H"]
    half_distance_cc = distance_cc / 2
    positions = np.array(
        [
            [-half_distance_cc, 0, 0],
            [half_distance_cc, 0, 0],
            [-(half_distance_cc + distance_ch), 0, 0],
            [half_distance_cc + distance_ch, 0, 0],
        ]
    )

    return atom_labels, positions


def c2h4_labels_positions():

    distance_ch = 1.07
    distance_cc = 1.33
    angle_hcc = 121.8

    atom_labels = ["C", "C", "H", "H", "H", "H"]

    half_distance_cc = distance_cc / 2

    theta = np.pi - angle_hcc
    distance_ch_y = distance_ch * np.sin(theta)
    distance_ch_x = distance_ch * np.cos(theta)

    positions = np.array(
        [
            [-half_distance_cc, 0, 0],
            [half_distance_cc, 0, 0],
            [-(half_distance_cc + distance_ch_x), distance_ch_y, 0],
            [-(half_distance_cc + distance_ch_x), -distance_ch_y, 0],
            [half_distance_cc + distance_ch_x, distance_ch_y, 0],
            [half_distance_cc + distance_ch_x, -distance_ch_y, 0],
        ]
    )

    return atom_labels, positions


def c3h8_labels_positions():

    atom_labels = ["C", "C", "C", "H", "H", "H", "H", "H", "H", "H", "H"]

    positions = np.array(
        [
            [0.0000, -0.5689, 0.0000],
            [-1.2571, 0.2844, 0.0000],
            [1.2571, 0.2845, 0.0000],
            [0.0000, -1.2183, 0.8824],
            [0.0000, -1.2183, -0.8824],
            [-1.2969, 0.9244, 0.8873],
            [-1.2967, 0.9245, -0.8872],
            [-2.1475, -0.3520, -0.0001],
            [2.1475, -0.3520, 0.0000],
            [1.2968, 0.9245, 0.8872],
            [1.2968, 0.9245, -0.8872],
        ]
    )

    return atom_labels, positions


def n2_labels_positions():

    distance_nn = 1.09
    atom_labels = ["N", "N"]
    positions = np.array([[0, 0, -distance_nn / 2], [0, 0, distance_nn / 2]])

    return atom_labels, positions


def build_geometry(atom_labels, positions):
    geometry = [(label, pos) for label, pos in zip(atom_labels, positions)]

    return geometry


def build_mol_info(atom_labels, positions):
    atom_str = ""
    for label, pos in zip(atom_labels, positions):
        atom_str += f"{label} {pos[0]}  {pos[1]}  {pos[2]};"

    mol_info = {
        "atom": atom_str,
        "basis": "sto3g",
        "charge": 0,
        "spin": 0,
        # "unit": DistanceUnit.ANGSTROM,
    }

    return mol_info


def main():

    label_list = [h2_labels_positions, lih_labels_positions, h2o_labels_positions, nh3_labels_positions, 
                c2h2_labels_positions, c2h4_labels_positions, c3h8_labels_positions, n2_labels_positions]
    # label_list = [h2_labels_positions, lih_labels_positions, h2o_labels_positions, nh3_labels_positions, 
    #             c2h2_labels_positions, c2h4_labels_positions, n2_labels_positions]
    
    # label_list = [h2_labels_positions]
    n_x, n_z, n_y, n_i = 0, 0, 0, 0
    nz_z, nz_x, n_zx = 0, 0, 0
    n_strings, n_qubits = 0, 0
    reps = 1

    laps = np.zeros(reps)
    pr = cProfile.Profile()
    pr.enable()
    start = time.time()

    for mol_idx in range(len(label_list)):
        mol_labels_position = label_list[mol_idx]
        mol_info = build_mol_info(*mol_labels_position())
        driver = PySCFDriver(**mol_info)
        problem = driver.run()
        hamiltonian = problem.hamiltonian
        coefficients = hamiltonian.electronic_integrals
        second_q_op = problem.hamiltonian.second_q_op()

        one_body_orbitals = [[], []]
        one_body_values = []

        two_body_orbitals = [[], [], [], []]
        two_body_values = []
        for term in second_q_op.terms():
            if len(term[0]) == 2:
                one_body_orbitals[0].append(term[0][0][1])
                one_body_orbitals[1].append(term[0][1][1])
                one_body_values.append(term[1])
            else:
                two_body_orbitals[0].append(term[0][0][1])
                two_body_orbitals[1].append(term[0][1][1])
                two_body_orbitals[2].append(term[0][2][1])
                two_body_orbitals[3].append(term[0][3][1])
                two_body_values.append(term[1])

        one_body_orbitals = [np.array(c, dtype=np.int_) for c in one_body_orbitals]
        two_body_orbitals = [np.array(c, dtype=np.int_) for c in two_body_orbitals]
        one_body_values = np.array(one_body_values)
        two_body_values = np.array(two_body_values)

        mapping = JordanWigner(second_q_op.num_spin_orbitals)

        

        for rep_idx in range(reps):
            t0 = time.time()
            qubit_hamiltonien = mapping.assemble_qubit_hamiltonian_from_sparses(
                (one_body_orbitals, one_body_values), (two_body_orbitals, two_body_values)
            )
            qubit_hamiltonien = operator_to_sparse_pauli(qubit_hamiltonien).simplify()
            t1 = time.time()
            laps[rep_idx] = t1 - t0
            
            # MOI!!!!!!
            
            for pauli_string in qubit_hamiltonien.paulis:  # Use 'pauli_string' instead of 'i'
                for pauli_op in pauli_string.to_label():  # Use 'pauli_op' instead of 'j'
                    if pauli_op == 'X':
                        n_x += 1
                    elif pauli_op == 'Z':
                        n_z += 1
                    elif pauli_op == 'Y':
                        n_y += 1
                    elif pauli_op == 'I':
                        n_i += 1 
                nz_z += np.sum(pauli_string.x)
                nz_x += np.sum(pauli_string.z)
                n_zx += len(pauli_string.x)
            
            # print("==== Molecule info ====")
            # print(f"Number of terms in the Hamiltonian: {len(qubit_hamiltonien)}")
            n_strings += len(qubit_hamiltonien)
            # print("Number of qubits:", qubit_hamiltonien.num_qubits)
            n_qubits += qubit_hamiltonien.num_qubits

                # print(nz_z, nz_x)
                
    pr.disable()

    print("========== Analyse PauliArray ==========")
    print(f"Time taken : {time.time() - start:.3f} seconds")
    # print("Number of strings:", n_strings)
    # print("Number of qubits:", n_qubits)
    total = n_x + n_z + n_y + n_i
    print(f"Total ops: {total}")
    print(f"X ops: {n_x}, Z ops: {n_z}, Y ops: {n_y}, I ops: {n_i}")
    print(f"Ratio X ops: {100*n_x/total:.2f}%, Ratio Z ops: {100*n_z/total:.2f}%, Ratio Y ops: {100*n_y/total:.2f}%, Ratio I ops: {100*n_i/total:.2f}%")

    print("=======================================")
    print("Non-zero z_voids: ", nz_z)
    print("Non-zero x_voids: ", nz_x)
    print("Total length: ", n_zx)
    print(f"Ratio of non-zero z_voids: {nz_z/(n_zx)*100:.2f}%")
    print(f"Ratio of non-zero x_voids: {nz_x/(n_zx)*100:.2f}%")
    # print(f"Ratio of all non-zero voids: {(nz_z + nz_x)/(n_zx)*100:.2f}%")
    print("=======================================")

    # print(f"\nNon-zero Z ops: {nz_z}, Non-zero X ops: {nz_x}")
    # ratio = (nz_z + nz_x) / (len(qubit_hamiltonien) * qubit_hamiltonien.num_qubits)
    # print(f"Total non-zero ops: {nz_z+nz_x}/{len(qubit_hamiltonien)*qubit_hamiltonien.num_qubits}")
    # print(f"Ratio of non-zero ops: {ratio:.6f} = {(1-ratio)*100:.2f}% sparsity")

    # print("Non-zero ops:")
    # print(qubit_hamiltonien.paulis)


    s = io.StringIO()
    sortby = SortKey.CUMULATIVE
    ps = pstats.Stats(pr, stream=s).sort_stats(sortby)

    # ps.print_stats()
    ps.dump_stats("sparse_chrono_pauliarray.prof")
    # print(s.getvalue())

    # print("PauliArray")

    print(f"{np.average(laps):.3f}, {np.std(laps):.3f}")

        # print(len(qubit_hamiltonien))
        # print(qubit_hamiltonien.num_qubits)

if __name__ == "__main__":
    main()
