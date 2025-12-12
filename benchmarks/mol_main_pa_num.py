import cProfile
import io
import os
import sys
import pstats
import time
from pstats import SortKey
import matplotlib.pyplot as plt
import numpy as np

main_pauliarray_path = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "pauliarray_main/pauliarray")
)
if main_pauliarray_path not in sys.path:
    sys.path.insert(0, main_pauliarray_path)


from pauliarray.conversion.qiskit import operator_to_sparse_pauli as main_operator_to_sparse_pauli
from pauliarray.mapping.fermion import JordanWigner as main_JordanWigner

from qiskit_nature.second_q.drivers import PySCFDriver
import datetime
import json
import pprint

from qiskit_nature.second_q.drivers import PySCFDriver
from qiskit_nature.second_q.mappers import JordanWignerMapper


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

    positions = np.array(
        [[0, 0, 0], [-distance_oh_x, -distance_oh_y, 0], [distance_oh_x, -distance_oh_y, 0]]
    )

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
    positions = np.array(
        [[0, 0, height], [rho_x_0, rho_y_0, 0], [rho_x_1, rho_y_1, 0], [rho_x_2, rho_y_2, 0]]
    )

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


def labels_positions_to_atom_string(atom_labels, positions):
    return "; ".join(
        f"{label} {x:.4f} {y:.4f} {z:.4f}" for label, (x, y, z) in zip(atom_labels, positions)
    )


label_list = [
    h2_labels_positions,
    lih_labels_positions,
    h2o_labels_positions,
    nh3_labels_positions,
    n2_labels_positions,
    c2h2_labels_positions,
    c2h4_labels_positions,
    c3h8_labels_positions,
]
label_list_names = ["H2", "LiH", "H2O", "NH3", "N2", "C2H2", "C2H4", "C3H8"]


def get_num_orbitals(atom_labels, positions):
    mol_info = build_mol_info(atom_labels, positions)
    driver = PySCFDriver(**mol_info)
    problem = driver.run()
    num_spin_orbitals = problem.num_spin_orbitals
    return num_spin_orbitals


label_list_num_qubits = [get_num_orbitals(*label_list[i]()) for i in range(len(label_list))]


def sym_main_pauli():
    print("==================== OLD PAULI ARRAY ====================")
    time_list = []

    for mol_idx, mol_labels_position in enumerate(label_list):
        start = time.time()
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

        mapping = main_JordanWigner(second_q_op.num_spin_orbitals)

        qubit_hamiltonien = mapping.assemble_qubit_hamiltonian_from_sparses(
            (one_body_orbitals, one_body_values), (two_body_orbitals, two_body_values)
        )
        qubit_hamiltonien = main_operator_to_sparse_pauli(qubit_hamiltonien).simplify()

        time_list.append(time.time() - start)
        print(f"Time for molecule {mol_idx+1}: {time_list[-1]:.3f} seconds")

    return time_list


def main():
    time_old = sym_main_pauli()
    # put list to json
    with open("./results/main_pa_times.json", "w") as f:
        json.dump({"num_qubits": label_list_num_qubits, "time": time_old}, f, indent=4)


if __name__ == "__main__":
    main()
