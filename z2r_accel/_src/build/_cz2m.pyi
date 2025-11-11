"""
Python bindings for the PauliArray class using Pybind11
"""
from __future__ import annotations
import collections.abc
import numpy
import numpy.typing
import typing
__all__: list[str] = ['bitwise_commute_with', 'compose', 'matmul', 'random_zx_strings', 'row_echelon', 'tensor', 'to_matrix', 'transpose', 'unique', 'unordered_unique']
def bitwise_commute_with(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: numpy.ndarray, arg3: numpy.ndarray) -> numpy.typing.NDArray[numpy.bool]:
    """
    Check commutation between two Pauli arrays
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
def matmul(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: typing.SupportsInt, arg3: typing.SupportsInt) -> numpy.ndarray:
    """
    addwad
    """
def random_zx_strings(arg0: collections.abc.Sequence[typing.SupportsInt]) -> tuple:
    """
    Gfddy
    """
def row_echelon(arg0: numpy.ndarray, arg1: typing.SupportsInt) -> numpy.ndarray:
    """
    addwad
    """
def tensor(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: numpy.ndarray, arg3: numpy.ndarray) -> tuple:
    """
    awdwa
    """
def to_matrix(arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: typing.SupportsInt) -> numpy.typing.NDArray[numpy.complex128]:
    """
    addwad
    """
def transpose(arg0: numpy.ndarray, arg1: typing.SupportsInt) -> numpy.ndarray:
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
