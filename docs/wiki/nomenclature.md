# Nomenclature
## Z2R
*Also know as 'Voids', 'bit_string'.*
<br>Is defined as: $\mathbb{Z}_2$ Rows.

- '$\mathbb{Z}_2$' denotes the cyclic group of integers modulo 2 (i.e the set {0, 1}).
- 'Rows' states that the structure is a row (list) of elements.

Thus, the structure is a long list of binary 1s and 0s. This packed representation significantly reduces memory usage and enables efficient bitwise operations.
- Python: `z = np.array([0, 1, 1 0], dtype=np.int8)`. This takes 4 bytes of memory
- C++: `z = 0b0110`. This takes 1 byte of memory

## CZ2M
Is defined as: C $\mathbb{Z}_2$ Matrix

This name is in reference to the C++ module which operates on Z2Rs to produce matrices.

## The bitwise_[...] prefix
Any functions with this prefix must be declared inside of `bitops.hpp` and adhere to the following:
1. Must accept any size and shape of NDArrays, as long as every input is the same shape and contiguous.
2. Must return an NDArray of the same shape, size, and dtype as the inputs.
3. Follows as closely as possible NumPy's functions of the same name.
   
The only execeptions to rule 2 are `bitwise_count()` and `bitwise_dot()`, which both compress the last dimension of the array.
