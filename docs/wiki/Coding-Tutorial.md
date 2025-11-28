In this exemple, we'll create a new C++ module and integrate it to the existing Python files


# Coding in C++
Before anything, choose the name of your new module. This will be later refered to as: {MODULE}
Create a new header (.h) file in `_core/include/` and a source (.cpp) file  in `_core/src/` with the name of your module. In your new header file, add:
``` C++
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;
```
> Omit `<pybind11/numpy.h>` if you don't need to pass NumPy arrays.

## Writing Functions
Lets write a function that takes an NDArray of packed bits, flips every bit in it to a new resulting array and finally returns that.

Declare the function in the header file by adding `py::array bitwise_not(py::array z2r);`, then begin its implementation in the .cpp source:
C++:
``` C++
py::array bitwise_not(py::array z2r) {
	auto buffer = z2r.request();
	const uint64_t *ptr_64 = std::bit_cast<uint64_t *>(buffer.ptr); 
	py::array result_arr = py::array(z2r.dtype(), buffer.shape);
	// ...
	uint64_t *result_ptr_64 = std::bit_cast<uint64_t *>(result_arr.request().ptr);
	// ...
	for (int i = 0; i < TOTAL_NUM_64_CHUNKS; i++) {
		result_ptr_64[i] = ~ptr_64[i];
	}
	return result_arr
}
```
For the function declaration:
- `py::array` declares an object that is a generic NDArray with no garanteed type. This is used because there is no C++ type equivalent to the `|V{N}` dtype.

The variables:
- `buffer` is an object that holds all of the input NDArray's data (e.g., shape, dtype, strides, values, etc.)
- `*ptr_64` is a pointer to 64 bits of data from the input. Accessing the next block of 64 bits is by incrementing the index of the pointer (e.g, `ptr_64[0]`, `ptr_64[1]`, ... `ptr_64[n]`). Note that since it is `const`, it can only read from memory and can never change the input data
- `result_arr` is a new NDArray created in C++ that will hold the results of the operation
- `*result_ptr_64` is the pointer to the results' data. 

And as for the code of the function:
- `auto buffer = z2r.request()`: Creates a buffer object with type inferred at compile time. The `request()` method returns a zero-copy view of the NDArray's data when possible.
- `std::bit_cast<uint64_t *>` reinterprets the data pointer to only see the underlying bits. This means that whatever the type of the data, its ignored and seen only as binary data.
- `py::array result_arr = py::array(z2r.dtype(), buffer.shape)`: Creates a new NDArray with the same dtype and shape as the input.
- `ptr_out_64[i] = ~ptr_64[i]` inverts the chunk of 64 bits and assigns the result to the result pointer 




## Exposing Functions (Bindings)
Now that we have a new function, we need to expose it so that it can be called from inside Python.

To do so, navigate to `_src/bindings/` and make a new C++ file (`{MODULE}_bindings.cpp`). Once done, add the following:
```C++
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "../{MODULE}.hpp"

PYBIND11_MODULE(_{MODULE}, m) {
    m.doc() = "This is the description for {MODULE}!";
    
    m.def("bitwise_not", &bitwise_not, 
          "Flips each bit in an NDArray", 
          py::arg("z2r"));
}
```
- sadsdsadsadsa



## Compiling Modules 
Each module must be compiled to a dynamic library by CMake. To do so, simply go to the `CMakeLists.txt` file and at the very last line, add: 
``` cmake
configure_pybind_module(
    _{MODULE}
    z2r_accel/_core/bindings/{MODULE}_bindings.cpp
    z2r_accel/_core/src/{MODULE}.cpp
    # Add any other dependencies here
)
    
``` 

## Calling from Python
The last step is to make a new Python file to define how functions are called. Move to the `z2r_accel` and create a Python file (`{MODULE}.py`). Inside it, add:
```
try:
    from ._core.build import _{MODULE}
    C_CCP = True
except ImportError:
    C_CCP = False
```
You can now can any functions declared in your binding file as any normal library (like `_my_module.bitwise_not()`) 