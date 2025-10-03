import xxhash
import numpy as np
import pauliarray as pa

def hash_voids(zx_voids):
    if axis is None:
        paulis = paulis.flatten()
        axis = 0
    elif axis >= paulis.ndim:
        raise ValueError("")
    else:
        axis = axis % paulis.ndim

    hashes = hash_voids(paulis.zx_voids)
    # Call np.unique ONCE and use all outputs from this call
    unique_hashes, unique_indices, inverse, counts = np.unique(
        hashes,
        axis=axis,
        return_index=True,
        return_inverse=True,
        return_counts=True,
    )

    unique_paulis = paulis[unique_indices]

    results = [unique_paulis]
    if return_index:
        results.append(unique_indices)
    if return_inverse:
        results.append(inverse)
    if return_counts:
        results.append(counts)

    if len(results) == 1:
        return results[0]
    return tuple(results)

def hash_voids(zx_voids):
    zx_bytes = np.ascontiguousarray(zx_voids).view(np.uint8)
    zx_bytes = zx_bytes.reshape(zx_voids.shape[0], -1)
    # Hash each Pauli string as bytes
    return [xxhash.xxh3_64_intdigest(row.tobytes()) for row in zx_bytes]


def main():
    p1 = pa.PauliArray.from_labels(
        ["IX", "XI", "XX", "YY", "ZZ", "IZ", "ZI", "ZZ", "ZX"]
    )
    zx_voids = p1.zx_voids
    print(hash_voids(zx_voids))

if __name__ == "__main__":
    main()