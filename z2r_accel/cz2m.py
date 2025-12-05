"""
File: cz2m.py
Author: Zakary Romdhane (zakary.romdhane@usherbrooke.ca)
Brief:
Version: 0.1
Date: 2025-12-05

Copyright: Copyright 2025 Zakary Romdhane
"""

import numpy as np
from numpy.typing import NDArray
from typing import Tuple
import pauliarray as pa

try:
    from ._core.build import _cz2m

    C_CCP = True
except ImportError:
    C_CCP = False

# assert C_CCP, "C++ backend not available."


def _contiguous(a):
    if a.flags.c_contiguous:
        return a
    else:
        return np.ascontiguousarray(a)


def _bc(a, b):
    if a.shape == b.shape:
        a2 = _contiguous(a)
        b2 = _contiguous(b)
        return a2, b2
    tmp = np.broadcast_arrays(a, b)
    return np.ascontiguousarray(tmp[0]), np.ascontiguousarray(tmp[1])


def unique(
    paulis,
    axis=None,
    return_index: bool = False,
    return_inverse: bool = False,
    return_counts: bool = False,
):
    if hasattr(paulis, "zx_voids"):
        z2r = paulis.zx_voids
    else:
        z2r = paulis

    return _cz2m.unique(z2r, return_index, return_inverse, return_counts)


def unordered_unique(z2r: NDArray) -> Tuple[NDArray, NDArray]:
    return _cz2m.unordered_unique(z2r)


def unordered_unique(
    paulis,
    axis=None,
    return_index: bool = False,
    return_inverse: bool = False,
    return_counts: bool = False,
):
    if hasattr(paulis, "zx_voids"):
        z2r = paulis.zx_voids
    else:
        z2r = paulis

    idx, inv = _cz2m.unordered_unique(z2r)

    uniques = pa.PauliArray.from_zx_voids(paulis.zx_voids[idx], paulis.num_qubits)

    # print("Uniques:", uniques)

    if return_index:
        return uniques, idx
    elif return_inverse:
        return uniques, inv
    elif return_counts:
        return uniques
    else:
        return uniques


def random_zx_strings(shape):
    return _cz2m.random_zx_strings(shape)


def transpose(z2r: NDArray, num_qubits: int) -> NDArray:
    return _cz2m.transpose(_contiguous(z2r), num_qubits)


# def pretty_print_z2r(z2r: NDArray, num_qubits: int) -> str:
#     bit_strings = z2r_to_bit_strings(z2r, num_qubits)

#     lines = []
#     for row in bit_strings:
#         row_flat = row.ravel()  # flatten to 1D
#         line = "".join(["1" if bit else "0" for bit in row_flat])
#         lines.append(line)

#     return "\n".join(lines)


def matmul(z2r_1: NDArray, z2r_2: NDArray, a_num_qubits: int, b_num_qubits: int) -> NDArray:
    return _cz2m.matmul(_contiguous(z2r_1), _contiguous(z2r_2), a_num_qubits, b_num_qubits)


def row_echelon(z2r: NDArray, num_qubits: int) -> NDArray:
    return _cz2m.row_echelon(_contiguous(z2r), num_qubits)


def concatenate(z_voids: NDArray, x_voids: NDArray, axis=0) -> NDArray:
    return _cz2m.concatenate(_contiguous(z_voids), _contiguous(x_voids), axis)


def z2_to_uint8(z2r: NDArray, num_qubits: int) -> NDArray:
    return _cz2m.z2_to_uint8(_contiguous(z2r), num_qubits)


def gauss_jordan_inverse(matrix: NDArray, num_qubits: int) -> NDArray:
    return _cz2m.gauss_jordan_inverse(_contiguous(matrix), num_qubits)
