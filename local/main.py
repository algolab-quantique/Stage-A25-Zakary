import pauliarray as pa

import pauliarray.binary.void_operations as vops
import numpy as np

# random_bits = np.random.randint(0, 2, size=150, dtype=np.uint8).tolist()

# void1 = vops.bit_strings_to_voids(random_bits)
# print(void1.dtype)
import pauliarray.binary.void_operations as vops
print("Using", vops.get_backend(), "backend.")