This documentation assumes you have basic knowledge in C/C++ syntax and are familiar with PauliArray's Python code. 

## Code Convention
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
Many special tags are available for Doxygen, like @deprecated, @attention and @warning, which are all used to a certain degree within the project.

### Auto-formatter
Since Black is only usable for Python files, we instead use `clang-format`, a highly customizable formatter that we can adjust to look more or less like Black.
All of the options are found inside of the `.clang-format` file. As a base, we use LLVM's coding format and override some specific options, such as:
-  `UseTab:            Never`: Always forces spaces instead of tabs for indentation
- `IndentWidth:        4`: Use a 4-wide standard for indentation
- `IndentPPDirectives: AfterHash`: Any preprocessor directives are indented before their '#'

### Namespaces
Generally, `using namespace std;` is useful to create C++ code and not worry about the syntax and boilerplate too much. However, since we're dealing with quite low-level code and many functions from other libraries could use the same name (i.e both std and pybind *could* have a function named make_tuple()), typing out std:: before standard functions ensure that it comes from the right library and doesn't cause unexpected errors. In addition, this makes function calls explicit and creates less guess-work for the following developers and maintainers.


## Interacting with Python
To interact with Python objects, we use [pybind11](https://github.com/pybind/pybind11) as a translator. The choice of pybind instead of something such as Boost.Python is mainly due to the seamless exchanges of NumPy's NDArrays to C++ objects and the overall better speed of the library.

In this project, any files that need to talk with Python objects (which in our case is generally only NumPy arrays) need to have these includes to the very top of the file:
``` C++
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;
```


### Functions
To create functions with pybind11, lets take a look and compare NumPy's `bitwise_invert` to our's

C++:
``` C++
inline py::array bitwise_not(py::array voids) {
	auto buf = voids.request();
	const uint64_t *ptr_64 = std::bit_cast<uint64_t *>(buf.ptr);

	py::array res_voids = py::array(voids.dtype(), buf.shape);
	...
	[main code loop]
	...
	return res_voids
}
```
Lets look at the function declaration:
- `inline` means that the following function will be *expanded* instead of *called* from any other functions. What this means in practice is that the external call will be entirely replaced with the actual code of this function. This increases binary file sizes but reduces traditional function call overhead.
- `py::array` declares an object that is a generic NDArray with no specific type. Its necessary to use this as there is currently no way to have an `array_t<void>`.

And as for the code within the function:
- `auto buf =` initializes a variable 'buf' (short for buffer) that will get its type detected at compile time, based on whatever follows the assignment operator. In our case, buf's type is completely irrelevant, and is why the auto keyword is used. 
- `= voids.request()` the request() method is for py::arrays only and returns an object which points to all of the relevant information that an NDArray has. The function is zero-copy whenever possible in order to increase speed.
- `py::array res_voids = py::array(voids.dtype(), buf.shape);` creates a new generic NDArray, with the same dtype and shape as our input's. This object is also directly usable within Python once it gets returned.

### Multithreading
In order to take full advantage of the machine's hardware, we use multithreading as often as possible. To do so, the OpenMP library is used due to its very simple syntax and powerful results, all whilst maintaining readability.

Most *for(...)* loops in the project are prefaced with:
``` C++
#ifdef USE_OPENMP
	#pragma omp parallel for if (local_variable >= SOME_MACRO) schedule(static)
#endif
```
- `#ifdef` is a preprocessor directive that only compiles it's code block if the specific macro is defined. In this case, `USE_OPENMP` is our own macro, and is only ever defined within CMake's compiling instruction. This is necessary since if OpenMP is not installed but still tries to compile OMP-specific pragmas, the compiler will throw out an error and exit.
- `#pragma omp parallel for` says that we'll be using OpenMP's directives, which will pass the next *for(...)* loop to multiple threads.
- `if (local_variable >= SOME_MACRO)` assures that the multithreading only occurs if the statement evaluates to True. Usually this is with a counter or size of a data point compared to an arbitrarily chosen constant (`VOPS_THRESHOLD_PARALLEL` is set to 1 000 000).
- `schedule(static)` assign each loop iterations to threads in a even distribution. 

## Building
### pyproject.toml


### CMake
CMake is a cross-platform build tool that we use to automatically compile each C++ files as dynamic libraries for use inside Python files. Pybind11 works cleanly with it and has automatic integration to `pip install`s when using `scikit-build-core`.

All of the configuration is done in `CMakeLists.txt`, located in the project's root. Users can decide to force CMake to build without OpenMP (no multithreading) by setting `USE_OPENMP` to `OFF`. This can be done manually, or by passing `--config-settings=cmake.define.USE_OPENMP=OFF` as an additional argument to pip.

