import numpy as np
from numpy.typing import NDArray
from typing import Tuple

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
    # The C backend `_cz2m.unique` only accepts the ndarray argument (positional).
    # If the caller requests extra information (indices/inverse/counts) or uses
    # an axis, fall back to numpy.unique on the zx_voids so we preserve the
    # numpy-like interface. Otherwise call the fast C implementation.
    if axis is not None or return_index or return_inverse or return_counts:
        # Mirror the Python-side behavior used elsewhere: operate on zx_voids
        # and convert back into PauliArray when appropriate.
        if axis is None:
            # flatten semantics
            tmp_paulis = paulis.flatten() if hasattr(paulis, "flatten") else paulis
            axis = 0
        out = np.unique(
            z2r,
            axis=axis,
            return_index=return_index,
            return_inverse=return_inverse,
            return_counts=return_counts,
        )
        if return_index or return_inverse or return_counts:
            out = list(out)
            unique_zx_voids = out[0]
            out[0] = paulis.__class__.from_zx_voids(unique_zx_voids, paulis.num_qubits)
        else:
            unique_zx_voids = out
            out = paulis.__class__.from_zx_voids(unique_zx_voids, paulis.num_qubits)
        return out

    # simple fast path: call C backend with only positional ndarray arg
    return _cz2m.unique(z2r)


def unordered_unique(z2r: NDArray) -> Tuple[NDArray, NDArray]:
    return _cz2m.unordered_unique(z2r)


# def unordered_unique(
#     object,
#     axis = None,
#     return_index: bool = False,
#     return_inverse: bool = False,
#     return_counts: bool = False,
# ):
#     if hasattr(object, 'zx_voids'):
#         z2r = object.zx_voids

#     else:
#         z2r = object

#     return _cz2m.unordered_unique(z2r)


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
