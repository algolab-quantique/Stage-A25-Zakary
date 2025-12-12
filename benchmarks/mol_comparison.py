import json
import matplotlib.pyplot as plt
import numpy as np


def plot_results():

    with open("./results/pa_times.json", "r") as f:
        pa_data = json.load(f)

    with open("./results/old_pa_times.json", "r") as f:
        old_pa_data = json.load(f)

    with open("./results/main_pa_times.json", "r") as f:
        main_pa_data = json.load(f)

    with open("./results/qiskit_times.json", "r") as f:
        qiskit_data = json.load(f)

    num_qubits = pa_data["num_qubits"]
    pa_times = pa_data["time"]
    old_pa_times = old_pa_data["time"]
    main_pa_times = main_pa_data["time"]
    qiskit_times = qiskit_data["time"]

    plt.figure(figsize=(10, 6))
    plt.plot(num_qubits, pa_times, marker="o", label="Pauli Array")
    plt.plot(num_qubits, old_pa_times, marker="o", label="Void Pauli Array")
    plt.plot(num_qubits, main_pa_times, marker="o", label="Main Pauli Array")
    plt.plot(num_qubits, qiskit_times, marker="o", label="Qiskit")
    plt.yscale("log")
    plt.xlabel("Number of Qubits")
    plt.ylabel("Time (seconds)")
    plt.title("Molecular Hamiltonian Construction Time Comparison")
    plt.legend()
    plt.grid(True)
    plt.savefig("./results/molecular_hamiltonian_comparison.png")
    plt.show()


def plot_relative():
    with open("./results/pa_times.json", "r") as f:
        pa_data = json.load(f)

    with open("./results/qiskit_times.json", "r") as f:
        qiskit_data = json.load(f)

    num_qubits = pa_data["num_qubits"]
    pa_times = pa_data["time"]
    qiskit_times = qiskit_data["time"]

    labels = ["H2", "LiH", "H2O", "NH3", "N2", "C2H2", "C2H4", "C3H8"]

    pa_times = np.array(pa_data["time"])
    qiskit_times = np.array(qiskit_data["time"])
    max_times = np.maximum(pa_times, qiskit_times)
    pauli_percent = pa_times / max_times * 100
    qiskit_percent = qiskit_times / max_times * 100

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
            f"{pa_times[idx]:.2f}s\n{pauli_percent[idx]:.1f}%",
            ha="center",
            va="bottom",
            fontsize=9,
        )
    for idx, bar in enumerate(bars2):
        height = bar.get_height()
        plt.text(
            bar.get_x() + bar.get_width() / 2,
            height + 4,
            f"{qiskit_times[idx]:.2f}s",
            ha="center",
            va="bottom",
            fontsize=9,
        )

    plt.show()


def main():
    # plot_results()
    plot_relative()


if __name__ == "__main__":
    main()
