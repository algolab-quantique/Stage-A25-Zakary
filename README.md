# PauliC++ 
WIP project which converts some of [PauliArray](https://github.com/algolab-quantique/pauliarray)'s functions to C++ in order to speed up the library all the whilst retaining the easy-to-use Python structure and syntax.

This project uses [pybind11](https://github.com/pybind/pybind11) as its translator between Python/Numpy and C++. 

#### Current C++ conversions:
 | Module Name     | Speed | 1-to-1 ? | Finished ? |
|-----------------|-------|----------|------------|
| void_operations.py | 1-9x  | Yes      | No         | 
| pauli_array.py  | 1-6x    | No       | No         |


## Structure
This is a dev repo. Anything outside of `/paulicpp` is subject to deletion. The project structure is the exact same as PauliArray, with the following additions:
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
- Having `g++` or `clang` is ***fundamental*** to this project, as it uses the gcc/clang intrinsic *__builtin_popcount()* as one of its core operation. This is one of the reason why the C++ implementation is faster than previous pure-python.
- Pre-installing in a venv `mypy` is not *necessary*, but not having it means the stub files will not be generated when compiling the source code.
- Having lots of time to waste to debug this library.

## Installation
Navigate to the `paulicpp` directory, create a virtual environment then run:
```console
$ python3 compiler.py
```
After which, run:
```console
$ pip install .
```
Once done, you may call the library as usual within a python script with `import pauliarray`.
<br> If you want to check what backend the library uses (C++ or a Python fallback), try:
``` python
import pauliarray.binary.void_operations as vops
print("Using", vops.get_backend(), "backend.")
```
