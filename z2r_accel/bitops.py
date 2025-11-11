import numpy as np
from numpy.typing import NDArray

try:
    from ._src.build import _bitops
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



def bitwise_count(z2r: NDArray) -> NDArray[np.int64]:
    return _bitops.bitwise_count(_contiguous(z2r))


def bitwise_not(z2r: NDArray) -> NDArray:
    return _bitops.bitwise_not(_contiguous(z2r))


def bitwise_dot(
    z2r_1: NDArray,
    z2r_2: NDArray,
) -> NDArray[np.int64]:

    return _bitops.bitwise_dot(*_bc(z2r_1, z2r_2))


def bitwise_and(
    z2r_1: NDArray,
    z2r_2: NDArray,
) -> NDArray:
    return _bitops.bitwise_and(*_bc(z2r_1, z2r_2))


def bitwise_xor(
    z2r_1: NDArray,
    z2r_2: NDArray,
) -> NDArray:
    return _bitops.bitwise_xor(*_bc(z2r_1, z2r_2))



def bitwise_or(
    z2r_1: NDArray,
    z2r_2: NDArray,
) -> NDArray:
    return _bitops.bitwise_or(*_bc(z2r_1, z2r_2))



def paded_bitwise_not(z2r: NDArray, num_qubits: int) -> NDArray:
#     return _bitops.paded_bitwise_not(_contiguous(z2r), num_qubits)
    return None


def get_backend():
    return "C++" if C_CCP else "Python"
