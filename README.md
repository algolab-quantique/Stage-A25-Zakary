# PauliC++ 
WIP project which converts some of [PauliArray](https://github.com/algolab-quantique/pauliarray)'s functions to C++ in order to speed up the library all the whilst retaining the easy-to-use Python structure and syntax.

This project uses [pybind11](https://github.com/pybind/pybind11) as its translator between Python/Numpy and C++. 

#### Current C++ conversions:

| void_operations.py            | Speedup  | 1-to-1 ? | Notes |
|-----------------              |-------   |----------|------------|
| _xor(), _and(), _or(), _not() | 1x to 5x | Yes      | n/a |
| _count(), _dot()              | 2x to 7x | Yes      | n/a   |

| pauli_array.py          | Speedup  | 1-to-1 ? | Notes |
|-----------------        |-------   |----------|------------|
| unique()                | 1.5x to 3x | Almost...      | 'axes' parameter is not implemented.   |
| bitwise_commute_with()  | 1x to 4x | Yes          | Weird Numpy array stuff going on. |

#### Sparse structure tests
| Sparse                  | Speedup  | 1-to-1 ? | Notes |
|-----------------        |-------   |----------|------------|
| _xor(), _and(), _or(), _not(), _count(), _dot()  | 0.002x to 40x | NO!      | Speedup (or slowdown, in many cases) depends entirely on the proximity of 1s in the binary void blobs.   |







## Structure
Anything outside of `/paulicpp` is subject to deletion. The project structure is the exact same as PauliArray, with the following additions:
```
├── pauliarray
|   ├── binary
|   ├── conversion
|   ├── estimation
|   ├── mapping
|   ├── pauli
|   ├── src
|   │   ├── bindings
|   │   │   └── {MODULE}_bindings.cpp
|   │   ├── build
|   │   │   └── {MODULE}.{dynamic_lib_extension}
|   │   └── {MODULE}.hpp
|   │
|   ├── stubs
|   │   └── {MODULE}.pyi
|   │
|   └── utils
|
└── compiler.py
```

- `/src` Contains the C++ source code (.hpp) for every modules.
- `/src/bindings` Contains the necessary pybind11 bindings necessary for the translation to and from Python.
- `src/build` Contains the dynamic shared libraries. 
    <br>Linux/macOS -> xyz.so
    <br>Windows -> xyz.dll
- `/stubs` Contains the stub files which are used to provide hints to the IDE about the various C++ functions.
- `compiler.py` Tries to compile every module using g++.






## Requirements
- Python 3.10+
- Having `g++` or `clang` is ***fundamental*** to this project, as it uses the gcc/clang intrinsic *__builtin_popcount()* as one of its core operation. This is one of the reason why the C++ implementation is faster compared to pure python or even NumPy
- Pre-installing in a venv `mypy` is not *necessary*, but not having it means the stub files will not be generated when compiling the source code.
- Having lots of time to waste to debug this library.

## Installation
If you are on macOS, please be sure to install OpenMP. This can easily be done with [brew](https://brew.sh/)
```console
$ brew install llvm
$ brew install libomp
```

<!--
```console
$ python3 compiler.py
``` -->
Navigate to the `paulicpp` directory, create a virtual environment and install the project with:
```console
$ cd paulicpp
$ python -m venv .venv
$ pip install .
```
This should automatically compile the C++ code based on your machine and link place it to the correct path.

Once done, you may call the library as usual within a python script with `import pauliarray`.
<br> If you want to check what backend the library uses (C++ or a Python fallback), try:
``` python
import pauliarray.binary.void_operations as vops
print("Using", vops.get_backend(), "backend.")
```

<!-- ## Compiler.py
COMPILER.PY IS CURRENTLY DEPRECATED.

You may call this script with the following arguments:
- `-m` or `--module` : Choose which module(s) to compile. Default is `all`.
<br> Accepted values are ["all", "pa", "dpa", "voidops"]
- `-c` or `--compiler` : Choose which compiler to use. Default is `g++`. 
<br> Accepted values are ["g++", "gcc", "clang++", "clang"].  -->
