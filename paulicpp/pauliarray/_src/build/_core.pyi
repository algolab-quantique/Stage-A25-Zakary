"""
Python bindings for the PauliArray class using Pybind11
"""
from __future__ import annotations
import collections.abc
import numpy
import numpy.typing
import typing
__all__: list[str] = ['bitwise_and', 'bitwise_commute_with', 'bitwise_count', 'bitwise_dot', 'bitwise_matmul', 'bitwise_not', 'bitwise_or', 'bitwise_row_echelon', 'bitwise_transpose', 'bitwise_xor', 'compose', 'paded_bitwise_not', 'random_zx_strings', 'tensor', 'to_matrix', 'unique', 'unordered_unique']
def bitwise_and(voids_1: numpy.ndarray, voids_2: numpy.ndarray) -> numpy.ndarray:
    """
    addwad
    """
def bitwise_commute_with(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: numpy.ndarray, arg3: numpy.ndarray) -> numpy.typing.NDArray[numpy.bool]:
    """
    Check commutation between two Pauli arrays
    """
def bitwise_count(arg0: numpy.ndarray) -> typing.Any:
    """
    addwad
    """
def bitwise_dot(arg0: numpy.ndarray, arg1: numpy.ndarray) -> typing.Any:
    """
    addwad
    """
def bitwise_matmul(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: typing.SupportsInt, arg3: typing.SupportsInt) -> numpy.ndarray:
    """
    addwad
    """
def bitwise_not(arg0: numpy.ndarray) -> numpy.ndarray:
    """
    addwad
    """
def bitwise_or(arg0: numpy.ndarray, arg1: numpy.ndarray) -> numpy.ndarray:
    """
    addwad
    """
def bitwise_row_echelon(arg0: numpy.ndarray, arg1: typing.SupportsInt) -> numpy.ndarray:
    """
    addwad
    """
def bitwise_transpose(arg0: numpy.ndarray, arg1: typing.SupportsInt) -> numpy.ndarray:
    """
    addwad
    """
def bitwise_xor(arg0: numpy.ndarray, arg1: numpy.ndarray) -> numpy.ndarray:
    """
    addwad
    """
@typing.overload
def compose(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: numpy.ndarray, arg3: numpy.ndarray) -> tuple:
    """
    Compose two Pauli arrays
    """
@typing.overload
def compose(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: numpy.ndarray, arg3: numpy.ndarray) -> tuple:
    """
    Compose two Pauli arrays
    """
def paded_bitwise_not(voids: numpy.ndarray, num_qubits: typing.SupportsInt) -> numpy.ndarray:
    """
    addwad
    """
def random_zx_strings(arg0: collections.abc.Sequence[typing.SupportsInt]) -> tuple:
    """
    Gfddy
    """
def tensor(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: numpy.ndarray, arg3: numpy.ndarray) -> tuple:
    """
    awdwa
    """
def to_matrix(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: typing.SupportsInt) -> numpy.typing.NDArray[numpy.complex128]:
    """
    addwad
    """
def unique(zx_voids: numpy.ndarray, return_index: bool = False, return_inverse: bool = False, return_counts: bool = False) -> typing.Any:
    """
    Unique arrays 1
    """
def unordered_unique(arg0: numpy.ndarray) -> tuple:
    """
    Returns unordered unique rows of the input array
    """
