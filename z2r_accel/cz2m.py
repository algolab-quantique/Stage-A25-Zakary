import numpy as np
from numpy.typing import NDArray
from typing import Tuple

try:
    from ._src.build import _cz2m

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


def unique() -> None:
    return None


#     paulis: PauliArray,
#     axis: Optional[int] = None,
#     return_index: bool = False,
#     return_inverse: bool = False,
#     return_counts: bool = False,
# ) -> Union[PauliArray, Tuple[PauliArray, NDArray]]:

#     if axis is None:
#         paulis = paulis.flatten()
#         axis = 0
#     elif axis >= paulis.ndim:
#         raise ValueError("")
#     else:
#         axis = axis % paulis.ndim

#     out = np.unique(
#         paulis.zx_voids,
#         axis=axis,
#         return_index=return_index,
#         return_inverse=return_inverse,
#         return_counts=return_counts,
#     )
#     # out = _cz2m.unique(
#     #     paulis.zx_voids,
#     #     # axis=axis,
#     #     return_index=return_index,
#     #     return_inverse=return_inverse,
#     #     return_counts=return_counts
#     # )


#     if return_index or return_inverse or return_counts:
#         out = list(out)
#         unique_zx_voids = out[0]
#         out[0] = PauliArray.from_zx_voids(unique_zx_voids, paulis.num_qubits)
#     else:
#         unique_zx_voids = out
#         out = PauliArray.from_zx_voids(unique_zx_voids, paulis.num_qubits)

#     return out


def unordered_unique(z2r: NDArray) -> Tuple[NDArray, NDArray]:
    return _cz2m.unordered_unique(z2r)


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
