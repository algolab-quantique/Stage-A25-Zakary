## When Writing
**Avoid Broadcasting in C++:** Broadcasting is a complex process that required substantial engineering effort from the NumPy team to implement efficiently. Rather than reimplementing ourself broadcasting in C++, either:
- Perform broadcasting in Python before passing data to C++
- Use pybind11's [vectorize](https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html#vectorizing-functions) functionality when dealing with simple functions

**Avoid Uncontiguous Data**: The speedup made by this library is mainly due to the contiguity of NDArrays, and how this property can be exploited with extreme compiler optimizations (SIMD) or by careful implementation of certain patterns and structure. For uncontiguous data, either:
TODO: EXPLAIN CONTIGUITY IN DETAIL!
- Rearange the data in Python before passing it to C++
- Explicitly state that slowdowns may occur (idk).

**Memory Management:** C++ lacks garbage collection or Rust's borrow checker, so you must manually manage memory efficiently:
- Track pointer lifetimes carefully to avoid dangling pointers
- Prefer zero-copy operations whenever possible
- Only allocate new memory when necessary

**Type Specifications:**
- Use `py::array_t<TYPE>` for typed arrays (e.g., `py::array_t<float>` for NDArrays of floats)
- Use generic `py::array` for Voids (packed binary data)

**Binary Data Types:** Since C++ has no native 'binary' type, a substitute one must be chosen instead. Use these types because they guarantee exact bit lengths and support both arithmetic and binary operations:
- `uint64_t`: 64-bit unsigned integer (for large data inputs)
- `uint8_t`: 8-bit unsigned integer (for small data)
- `int64_t`: 64-bit signed integer. Use ONLY for returning outputs

Avoid:
- `std::byte` (no arithmetic support)
TODO: Clarify size garantees
- `unsigned char` (no size guarantee)
- `unsigned long long int` (unnecessarily verbose)

**Do not use `using namespace std;`**
While convenient, namespace imports create ambiguity in low-level code. Both the standard library and pybind11 may define functions with identical names (e.g., `make_tuple()`), which would result in unpredicable function calls. Explicit namespace prefixes ensure:
- Functions are called from the correct library
- Code behavior is predictable
- Future maintainers understand the code's intent

---

## Comments
In Python files, use Docstrings:
``` python

def add_integers(a:int, b:int) -> int:
	"""
	Calculates the sum of two integers
	Args:
		a (int): The first operand
		b (int): The second operand
	Returns:
		int: The sum of both inputs
	"""
	return a + b
```

In C++, use Doxygen comments:
``` c++
/**
* @brief Calculates the sum of two integers
*
* @param a The first integer operand
* @param b The second integer operand
* @return int The sum of both inputs
*/
int add_integers(int a, int b) {
	return a + b;
}
```

---

## Formatting
Please format your files using the project's [[auto-formatters|External Tools#Auto-formatters]]. A GitHub Action will perform checks on every push to detect if all files were correctly formatted.