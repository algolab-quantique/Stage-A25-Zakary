test
## Data structure
Handling long lists of 1s and 0s is at the heart of this project, and needs to be done as efficiently as possible. The current way all data is managed is throught a packed representation of integers such that:

`ex = 0b01100111` or `ex = 103`, 
   - Drawback: for extreme cases where almost all entries are consecutively 1s or 0s, this method is slower and takes more memory space than a sparse implementation 
   - Benefits:
     - Enables native bitwise ops (AND/OR/XOR/NOT), popcount, shifts, etc.
     - SIMD, simpler/faster multithreading on most operations 

Alternatives were tried, but proved to be less efficient in either memory or speed:

1. **Binary list**
    <br>`ex = [0,1,1,0,0,1,1,1]`, a list of individual integers which are always either 1 or 0
   - Drawback: each element takes up a whole byte while only one bit is needed. Poor memory density and difficult SIMD exploitation makes this approach slow in almost any scenario.

2. **Sparse intervals**
    <br>`ex = [(1,3),(5,8)]`, a list of pairs which denotes runs of consecutive 1s.  
   - Drawback: good for very very sparse patterns but degrades rapidely to O(n). Enormous overhead for most cases.

---

## General
In most cases, we cast the data of a Z2R array to `uint64_t` via `std::bit_cast<>`. This is done because in most modern computers, 64 bits is the largest natively supported integer size. When an object's total size is less than 64 bits, casting is mostly on `uint8_t` and sometimes on `bitset<N>`


## Multi-threading
To fully utilize available hardware, we implement multi-threading wherever safe and beneficial. OpenMP provides simple syntax with powerful results while maintaining code readability.
### Using OpenMP
Before adding OpenMP, make it **certain** that your code is thread-safe. Data races and memory corruption are difficult to debug. While mutexes can provide thread safety over otherwise unsafe operations, they often cause performance degradation in small functions (our primary use case). Use mutexes cautiously and verify they provide net benefits.

As for the syntax, most *for(...)* loops in the project are prefaced with:
```cpp
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
```cpp
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

> Note: The Python language is working towards removing the GIL. This could take many years before it is accomplished, buf if/when it finished, there will likely be some breakage around any parts explicitly managing the GIL. See [PEP 703](https://peps.python.org/pep-0703/)
