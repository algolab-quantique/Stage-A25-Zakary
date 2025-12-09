import cProfile
import io
import os
import sys
import pstats
import time
from pstats import SortKey
import matplotlib.pyplot as plt
import numpy as np

from pauliarray.conversion.qiskit import operator_to_sparse_pauli
from pauliarray.mapping.fermion import JordanWigner

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


def collect_z_run_lengths(qubit_hamiltonien):
    """
    For each Pauli string, find every position with 'Z', count how many consecutive 'Z's follow.
    Returns a list of all Z run lengths.
    """
    z_run_lengths = []
    for pauli_string in qubit_hamiltonien.paulis:
        label = pauli_string.to_label()
        i = 0
        while i < len(label):
            if label[i] == "Z":
                run_len = 1
                j = i + 1
                while j < len(label) and label[j] == "Z":
                    run_len += 1
                    j += 1
                z_run_lengths.append(run_len)
                i = j
            else:
                i += 1
    return z_run_lengths


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


def sym_pauli():
    print("==================== PAULI ARRAY ====================")
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

        mapping = JordanWigner(second_q_op.num_spin_orbitals)

        qubit_hamiltonien = mapping.assemble_qubit_hamiltonian_from_sparses(
            (one_body_orbitals, one_body_values), (two_body_orbitals, two_body_values)
        )
        qubit_hamiltonien = operator_to_sparse_pauli(qubit_hamiltonien).simplify()

        time_list.append(time.time() - start)
        print(f"Time for molecule {mol_idx+1}: {time_list[-1]:.3f} seconds")

    return time_list


def sym_qiskit():
    print("==================== QISKIT ====================")

    time_list = []
    for i in range(len(label_list)):
        start = time.time()
        # print(f"Processing molecule {i+1}/{len(label_list)}")
        atom_labels, positions = label_list[i]()
        atom_str = labels_positions_to_atom_string(atom_labels, positions)

        mol_info = {
            "atom": atom_str,
            "basis": "sto3g",
            "charge": 0,
            "spin": 0,
        }

        driver = PySCFDriver(**mol_info)

        problem = driver.run()
        hamiltonian = problem.hamiltonian
        second_q_op = problem.hamiltonian.second_q_op()

        mapper = JordanWignerMapper()
        qubit_jw_op = mapper.map(second_q_op)

        time_list.append(time.time() - start)
        print(f"Time for molecule {i+1}: {time_list[-1]:.3f} seconds")

    return time_list


def plot_times(list1, list2, labels):
    plt.figure()
    plt.plot(labels, list1, label="PauliArray", marker="o")
    plt.plot(labels, list2, label="Qiskit", marker="o")
    plt.xlabel("Molecule Index")
    plt.ylabel("Time (s)")
    plt.title("Computation Time Comparison")
    plt.legend()
    plt.grid(True)
    # plt.semilogx()
    plt.semilogy()
    # plt.savefig("computation_time_comparison.png")

    plt.show()


def plot_times_percentage(list1, list2, labels):
    max_times = np.maximum(list1, list2)
    pauli_percent = np.array(list1) / max_times * 100
    qiskit_percent = np.array(list2) / max_times * 100

    x = np.arange(len(labels))
    width = 0.35

    plt.figure(figsize=(10, 6))
    bars1 = plt.bar(x - width / 2, pauli_percent, width, label="PauliArray")
    bars2 = plt.bar(x + width / 2, qiskit_percent, width, label="Qiskit")
    plt.xticks(x, labels, rotation=45)
    plt.ylabel("Relative Time (%)")
    plt.title("Relative Time Between PauliArray and Qiskit\n(molecule simulation)")
    plt.legend(loc="center left", bbox_to_anchor=(0, 0.5))
    plt.grid(axis="y")
    plt.tight_layout()

    # Find the maximum bar height to adjust ylim
    max_height = max([bar.get_height() for bar in list(bars1) + list(bars2)])
    plt.ylim(0, max_height + 15)  # Add extra space above bars

    # Add time and percentage values on top of each bar
    for idx, bar in enumerate(bars1):
        height = bar.get_height()
        plt.text(
            bar.get_x() + bar.get_width() / 2,
            height + 4,
            f"{list1[idx]:.2f}s\n{pauli_percent[idx]:.1f}%",
            ha="center",
            va="bottom",
            fontsize=9,
        )
    for idx, bar in enumerate(bars2):
        height = bar.get_height()
        plt.text(
            bar.get_x() + bar.get_width() / 2,
            height + 4,
            f"{list2[idx]:.2f}s",
            ha="center",
            va="bottom",
            fontsize=9,
        )

    plt.show()


if __name__ == "__main__":
    qiskit_times = sym_qiskit()
    pauli_times = sym_pauli()
    molecule_labels = label_list_names[: len(pauli_times)]
    # plot_times_percentage(pauli_times, qiskit_times, molecule_labels)
    plot_times(pauli_times, qiskit_times, molecule_labels)
