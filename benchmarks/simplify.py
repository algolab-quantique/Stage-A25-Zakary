import pickle
from qiskit.quantum_info import SparsePauliOp
import pauliarray as pa
import pauliarray.conversion.qiskit as conv

with open("SparsePauliOp.pkl", "rb") as f:
    op: SparsePauliOp = pickle.load(f)

# pa.WeightedPauliArray.from_labels_and_weights(op.paulis.to_labels(), op.coeffs)
# conv.pauli_array_from_pauli_list(op.paulis)
# op.coeffs
wpa = conv.weighted_pauli_array_from_pauli_list_and_coeffs(op.paulis, op.coeffs)
print(wpa.num_qubits)
print(wpa.x_voids)
# op.paulis.to_labels()
