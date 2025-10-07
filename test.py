import pauliarray.binary.void_operations as vops
print("Using", vops.get_backend(), "backend.")

import pauliarray

p1 = pauliarray.PauliArray.random((5,), 10)
p2 = pauliarray.PauliArray.random((5,), 10)
print(p1.bitwise_commute_with(p2))