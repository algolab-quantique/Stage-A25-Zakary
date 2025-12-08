from qiskit_nature.second_q.operators import FermionicOp
from qiskit_nature.second_q.mappers import JordanWignerMapper
import numpy as np

MAPPER = JordanWignerMapper()


def matrix_hop(num_site, bond_list):
    """Index of qubits starts at zero
    Example for 4 qubits line could be :
    [(0,1), (1,2), (2,3)]
    """
    matrix = np.zeros((num_site, num_site))

    for bond in bond_list:
        matrix[bond[0], bond[1]] = 1
        matrix[bond[1], bond[0]] = 1

    # print("matrix:\n", matrix)
    return matrix


def line_bond_list(num_sites):
    """Function that generates the bond list for line geometry of Hubbard model"""
    return [(i, i + 1) for i in range(num_sites - 1)]


def closed_bond_list(num_sites):
    """Function that generates the bond list for closed geometry of Hubbard model"""
    return [(i, (i + 1) % num_sites) for i in range(num_sites)]


def get_hop_mat_from_geo(num_site, geo="line"):
    """Returns the hopping matrix for a given number of sites and geometry"""
    if geo == "line":
        bond_list = line_bond_list(num_site)
    elif geo == "closed":
        bond_list = closed_bond_list(num_site)
    else:
        raise ValueError("Unknown geometry")

    return matrix_hop(num_site=num_site, bond_list=bond_list)


def hubbard(hop=np.array([[0, 1], [1, 0]]), u=4, mu=None):
    n = len(hop)
    if mu is None:
        mu = u / 2
    hopping = FermionicOp(
        {
            f"+_{i+m} -_{j+m}": hop[i, j]
            for m in [0, n]
            for i in range(n)
            for j in range(n)
            if hop[i, j] != 0
        },
        num_spin_orbitals=2 * n,
    )
    occupation = FermionicOp({f"+_{i} -_{i}": 1 for i in range(2 * n)}, num_spin_orbitals=2 * n)
    interaction = FermionicOp(
        {f"+_{i} +_{i+n} -_{i+n} -_{i}": 1 for i in range(n)}, num_spin_orbitals=2 * n
    )
    return MAPPER.map(-hopping + u * interaction - mu * occupation)


if __name__ == "__main__":
    hop = get_hop_mat_from_geo(num_site=10000, geo="line")
    # print("Hopping matrix:", hop)
    qubit_op = hubbard(hop=hop, u=4, mu=2)

    # print(qubit_op)
    print("END!")
