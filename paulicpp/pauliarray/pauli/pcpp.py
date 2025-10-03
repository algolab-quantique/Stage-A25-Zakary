import pauliarray as pa
import paulicpp as pc
from pauliarray.binary import void_operations

def commute_with(p1: pa.PauliArray, p2: pa.PauliArray):
    return pc.commute_with(p1.z_voids, p1.x_voids, p2.z_voids, p2.x_voids)

def random(size, num_qubits):
    z, x = pc.random_zx_strings(size, num_qubits)
    return pa.PauliArray.from_z_strings_and_x_strings(z, x)

def tensor(p1: pa.PauliArray, p2: pa.PauliArray) -> pa.PauliArray:
    
    z_combined, x_combined = pc.tensor(p1.z_voids, p1.x_voids, p2.z_voids, p2.x_voids)
    return pa.PauliArray(z_combined, x_combined, p1.num_qubits + p2.num_qubits)
