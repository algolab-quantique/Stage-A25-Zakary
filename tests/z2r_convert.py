import numpy as np
from numpy.typing import NDArray


def int_strings_to_voids(int_strings: NDArray[np.uint]) -> NDArray:
    """
    Converts int strings into voids.

    Args:
        int_strings (NDArray[np.uint]): _description_

    Returns:
        NDArray: _description_
    """

    pad_int_strings = pad_int_strings_to_commensurate_itemsize(
        int_strings, int(2 ** np.ceil(np.log2(int_strings.dtype.itemsize)))
    )

    void_type_size = pad_int_strings.dtype.itemsize * pad_int_strings.shape[-1]
    voids = np.squeeze(
        np.ascontiguousarray(pad_int_strings).view(np.dtype((np.void, void_type_size))), axis=-1
    )

    return voids


def pad_int_strings_to_commensurate_itemsize(
    int_strings: NDArray[np.uint], new_itemsize: int
) -> NDArray[np.uint]:
    """
    Pad int_strings so that the new int_strings size is a multiple of the new_itemsize. For example, a 3 uint16 (size=2) int_string and a new_itemsize=8 would pad 1 uint16.

    Args:
        int_strings (NDArray[np.uint]): _description_
        new_itemsize (int): _description_

    Returns:
        NDArray[np.uint]: _description_
    """
    assert new_itemsize in [1, 2, 4, 8]

    tot_int_strings_size = int_strings.shape[-1] * int_strings.dtype.itemsize

    new_int_size = np.lcm(int_strings.dtype.itemsize, new_itemsize)
    tot_num_new_ints = np.ceil(tot_int_strings_size / new_int_size).astype(int)

    tot_new_int_strings_size = int(2 ** np.ceil(np.log2(tot_num_new_ints * new_itemsize)))
    tot_num_ints = tot_new_int_strings_size // int_strings.dtype.itemsize

    int_to_pad = tot_num_ints - int_strings.shape[-1]

    if int_to_pad == 0:
        pad_int_strings = int_strings
    else:
        pad_width = [(0, 0)] * (int_strings.ndim - 1)
        pad_width.append((0, int_to_pad))

        pad_int_strings = np.pad(int_strings, pad_width)

    return pad_int_strings


def bool_arr_to_z2r(bool_arr: NDArray[np.bool_]) -> NDArray:
    return None
