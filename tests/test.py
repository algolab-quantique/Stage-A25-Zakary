import z2r_convert as convert
import numpy as np

arr = np.array([[1, 1, 1], [0, 0, 0]])
z2r = convert.bool_arr_to_z2r(arr)
print(z2r)
print(z2r.dtype)
