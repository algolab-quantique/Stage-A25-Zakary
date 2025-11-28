Please see the wiki for more information.
<!-- This documentation assumes you have basic knowledge in C/C++ syntax and are familiar with PauliArray's Python code.

There is currently a reformatting and renaming effort throughout the project. Names like 'paulicpp' will be changed to something more agnostic and appropriate in the near future.

## Code Convention
`{MOD}` is a module's name. There can be multiple modules with different names compiled to the same directories. 
- `/_src` Contains the C++ source code (.hpp) for every module.
- `/src/bindings` Contains the pybind11 bindings necessary for the translation to and from Python.
- `src/build` Contains the compiled dynamic shared libraries and stub files.


### Build Requirements
To build this library from source, you will need:
- Python 3.10+
- GCC or Clang that supports at least C++20
- A 64-bit Unix-like system
- (Optional) libomp/OpenMP

Note: In theory, Windows can compile the entire project with no issues. However, since MSVC is a completely different beast to GCC or Clang, no build options have been added to support it. MinGW would also probably work.

---
## Nomenclature

### Z2R
*Also know as 'Voids', 'bit_string'.*
<br>Is defined as: $\mathbb{Z}_2$ Rows.

- '$\mathbb{Z}_2$' denotes the cyclic group of integers modulo 2 (i.e the set {0, 1}).
- 'Rows' states that the structure is a row (list) of elements.

Thus, the structure is a long list of binary 1s and 0s. This packed representation significantly reduces memory usage and enables efficient bitwise operations.
- Python: `z = np.array([0, 1, 1 0], dtype=np.int8)`. This takes 4 bytes of memory
- C++: `z = 0b0110`. This takes 1 byte of memory

### CZ2M
Is defined as: C $\mathbb{Z}_2$ Matrix

This name is in reference to the C++ module which operates on Z2Rs to produce matrices.

### The bitwise_[...] prefix
Any functions with this prefix must be declared inside of `bitops.hpp` and adhere to the following:
1. Must accept any size and shape of NDArrays, as long as every input is the same shape and contiguous.
2. Must return an NDArray of the same shape, size, and dtype as the inputs.
3. Follows as closely as possible NumPy's functions of the same name.
   
The only execeptions to rule 2 are `bitwise_count()` and `bitwise_dot()`, which both compress the last dimension of the array.



---

### General Coding Standards
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

### Docstrings
A Python docstring such as:
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

Is roughly equivalent to the Doxygen comment:
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
Additional Doxygen tags include `@deprecated`, `@attention`, and `@warning`, which are used throughout the project where appropriate.
  

### Auto-formatter
Since Black is only usable for Python files, we instead use [clang-format](https://clang.llvm.org/docs/ClangFormat.html), a highly customizable formatter that we can adjust to look more or less like Black.

All of the options are found inside of the `.clang-format` file. As a base, we use LLVM's coding format and override some specific options:
- `UseTab: Never`: Always forces spaces for indentation
- `IndentWidth: 4`: Use 4-space standard for indentation
- `IndentPPDirectives: AfterHash`: Indent preprocessor directives after their '#'

# Interacting with Python
We use [pybind11](https://github.com/pybind/pybind11) to interface between Python and C++. Pybind11 was chosen over alternatives like Boost.Python for its:
- Seamless NumPy NDArray integration
- Superior performance
- Modern C++ support

Include these headers at the top of files that interact with Python:
``` C++
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;
```
Note: Omit `<pybind11/numpy.h>` if you don't need to pass NumPy arrays.

### Writing Functions
C++ requires a lot more work and boilerplate in order to get the same result as in Python. 
The following functions both flip every bit of data inside of an NDArray:

Python:
https://numpy.org/doc/stable/reference/generated/numpy.bitwise_invert.html#numpy.bitwise_invert
``` python
def bitwise_not(voids: NDArray) -> NDArray:
    int_strings = voids_to_int_strings(voids)
    new_int_strings = np.invert(int_strings)
    new_voids = int_strings_to_voids(new_int_strings)

    return new_voids
```


C++:
``` C++
inline py::array bitwise_not(py::array voids) {
	auto buffer = voids.request();
	const uint64_t *ptr_64 = std::bit_cast<uint64_t *>(buffer.ptr);
	py::array result_arr = py::array(voids.dtype(), buffer.shape);
	// ...
	uint64_t *result_ptr_64 = std::bit_cast<uint64_t *>(result_arr.request().ptr);
	// ...
	for (int i = 0; i < TOTAL_NUM_64_CHUNKS; i++) {
		result_ptr_64[i] = ~ptr_64[i];
	}
	return result_arr
}
```
Lets look at the function declaration:
- `inline` means that the following function will be *expanded* instead of *called* from any other functions. What this means in practice is that the external call will be entirely replaced with the actual code of this function. This increases binary file sizes but reduces traditional function call overhead.
- `py::array` declares an object that is a generic NDArray with no specific type.

And as for the code within the function:
- `auto buffer = voids.request()`: Creates a buffer object with type inferred at compile time. The `request()` method returns a zero-copy view of the NDArray's data when possible.
- `std::bit_cast<uint64_t *>` is a near zero-overhead reinterpretation of the data underneath.
- `py::array result_arr = py::array(voids.dtype(), buffer.shape)`: Creates a new NDArray with the same dtype and shape as the input. This array can be returned directly to Python.
- `ptr_out_64[i] = ~ptr_64[i]` inverts the chunk of 64 bits and assigns the result to our result pointer 

### Exposing Functions to Python (Bindings)
All C++ functions must be declared in their module's binding file to be accessible from Python.
Lets say we want to add `bitwise_not` to our module,
```C++
#include "your_source_file.hpp"

PYBIND11_MODULE(your_module_name, m) {
    m.doc() = "The description of your module";
    
    m.def("bitwise_not", &bitwise_not, 
          "Flips each bit in an NDArray", 
          py::arg("voids"));
}
```

# Optimization Efforts
## General
In most cases, we cast the data of a Voids array to `uint64_t` via `std::bit_cast<>`. This is done because in most modern computers, 64 bits is the largest natively supported integer size. Even when an array’s dimension is smaller than 64 bits, treating it as a sequence of raw 64-bit blocks enables highly optimized bitwise manipulation and reduces overhead.





## Multi-threading
To fully utilize available hardware, we implement multi-threading wherever safe and beneficial. OpenMP provides simple syntax with powerful results while maintaining code readability.
### Using OpenMP
Before adding OpenMP, make it **certain** that your code is thread-safe. Data races and memory corruption are difficult to debug. While mutexes can provide thread safety over otherwise unsafe operations, they often cause performance degradation in small functions (our primary use case). Use mutexes cautiously and verify they provide net benefits.

As for the syntax, most *for(...)* loops in the project are prefaced with:
``` C++
#ifdef USE_OPENMP
	#pragma omp parallel for if (local_variable >= SOME_MACRO) schedule(static)
#endif
	for (i=0; i<local_variable; i++) {
		// Code that does something which could be multi-threaded
	}
```
- `#ifdef` is a preprocessor directive that only compiles it's code block if the specific macro is defined. In this case, `USE_OPENMP` is our own macro, and is only ever defined within CMake's compiling instruction. This is necessary since if OpenMP is not installed but still tries to compile OMP-specific pragmas, the compiler will throw out an error and exit.
- `#pragma omp parallel for` says that we'll be using OpenMP's directives, which will pass the next *for(...)* loop to multiple threads.
- `if (local_variable >= SOME_MACRO)` assures that the multithreading only occurs if the statement evaluates to True. Usually this is with a counter or size of a data point compared to an arbitrarily chosen constant.
- `schedule(static)` assign each loop iterations to threads in a even, round-robin distribution.

More keywords exist, but they are specific to certain behaviors that are much less common in this project

### Understanding the GIL

Python's Global Interpreter Lock allows only one thread to execute Python bytecode at a time. This simplifies Python's memory management but prevents true multi-threading. Only multiprocessing can achieve true parallelism under the GIL.

However, it is possible to get freed from the shackles of the GIL within C++. This can be done by declaring a section like this:
```C++
{
  py::gil_scoped_release release;
  // very heavy CPU loop (Real multithreading allowed)
}
// The GIL is automaticaly reaquired here
```
Releasing the GIL has some overhead, so it is only useful when code needs to be truly parallelized.
**Release the GIL when:**
- Operations are CPU-intensive and long-running
- No Python API calls are needed
- Using OpenMP or manual threading
  
See `unordered_unique()`'s management of the GIL and OpenMP for more information. It is the very last function inside of `paulicpp/pauliarray/src/paulicpp.hpp`

Note: The Python language is working towards removing the GIL. This could take many years before it is accomplished, buf if/when it finished, there will likely be some breakage around any parts explicitly managing the GIL. See [PEP 703](https://peps.python.org/pep-0703/)

## Building
WIP
### pyproject.toml
`scikit-build-core` is the build backend as Flit not currently compatible with custom C++ compilation steps.

sasssaaa
  

### CMake
CMake is a cross-platform build tool. It has clean integration with pybind11 and `scikit-build-core` for `pip install` support.

The configuration file is `CMakeLists.txt`, found in the project root.

**Build Options:**
- `USE_OPENMP`: Enable/disable OpenMP multi-threading
    - Default: `ON`
    - Disable manually in `CMakeLists.txt` or via pip: `pip install . --config-settings=cmake.define.USE_OPENMP=OFF`

---

# Name 


## Why C++?
1. Performance: Most operations can be sped-up with either precise bit-on-bit algorithms and/or extreme compiler optimizations (like SIMD). Results are usually between 1.5-9x faster than pure NumPy
 2. Memory: Handling Pauli strings within C++ allows for packed representations with next to no compromises.  Less RAM 
 3. Flexibility: C++ offers total control over what a function needs to do. This is especially useful when XYZ
 ---

# External tools and libraries
The tools used for this project are:
- pybind11
- pybind-stubgen
- CMake
- clang-format

## Why pybind11?
Pybind11 was chosen for its minimal boilerplate, seamless NumPy integration, and modern C++ support. It allows us to expose C++ functions and classes to Python with very little overhead, making it easy to maintain and extend the codebase.

Since pybind is chosen as the interface for this project, the stub generator is obviously pybind-stubgen. However, some problems have appeared with it, notably when trying to add external libraries. It may get replaced with mypy's stubgen in the future.

Other alternatives for optimizing code to work with Python are:

1. **Numba**
    - While Numba is excellent for accelerating generic Python code, our data structures are highly specialized and already optimized in C++. Numba would offer limited additional benefit and cannot easily accelerate C++ code.

2. **Cython**
   - Cython is powerful for optimizing Python code and interfacing with C, but it often requires rewriting parts (if not most) of functions to take full advantage of its features. At that point, we might as well rewrite everything in pure C/C++

3. **Boost.Python**
     - Boost.Python is feature-rich but introduces significant overhead and complexity. It requires more boilerplate code, demands loads more dependencies, and is generally slower compared to pybind11.




# Data structures
Handling long lists of 1s and 0s is at the heart of this project, and needs to be done as efficiently as possible. The current way all data is managed is throught a packed representation of integers.

Example: `ex = 0b01100111` stored in a unsigned integer.  
   - Drawback: for extreme cases where almost all entries are consecutively 1s or 0s, this method is slower and takes more memory space than a sparse implementation 
   - Benefits:
     - Enables native bitwise ops (AND/OR/XOR/NOT), popcount, shifts, etc.
     - SIMD, simpler/faster multithreading on most operations 


1. **Binary list**
   - Example: `ex = [0,1,1,0,0,1,1,1]`
   - Drawback: each element takes up a whole byte while only one bit is needed. Poor memory density and difficult SIMD exploitation makes this approach slow in almost any scenario.

2. **Sparse intervals**
   - Example: `ex = [(1,3),(5,8)]` denotes runs of consecutive 1s.  
   - Drawback: good for very very sparse patterns but degrades rapidely to O(n). Enormous overhead for most cases.

   


<img src = "https://github.com/algolab-quantique/Stage-A25-Zakary/blob/main/assets/datastruct_comparison.png" align="center" width="300">

--- -->