# Project Structure and Architecture
## Structure
The project structure is the exact same as PauliArray's [Voids](https://github.com/algolab-quantique/pauliarray/tree/Voids) branch, with the following additions:
``` 
├── _src
│   ├── bindings
│   │   └── {MOD}_bindings.cpp
│   ├── build
│   │   ├── _{MOD}.[dyn_lib_ext]
│   │   └── _{MOD}.pyi
│   └── {MOD}.hpp
└── {MOD}.py
```
`{MOD}` is a module's name. There can be multiple modules with different names compiled to the same directories. 
- `/_src` Contains the C++ source code (.hpp) for every module.
- `/src/bindings` Contains the pybind11 bindings necessary for the translation to and from Python.
- `src/build` Contains the compiled dynamic shared libraries and stub files.


## Architecture
```mermaid
graph TB
    subgraph "Python Layer"
        A[Python User Code]
        B[NumPy Arrays]
        C[Python API]
    end
    
    subgraph "Binding Layer"
        D[pybind11 Bindings<br/>*_bindings.cpp]
        E[Type Conversion<br/>py::array ↔ C++]
        F[Function Exposure<br/>PYBIND11_MODULE]
    end
    
    subgraph "C++ Core Layer"
        G[Header Files<br/>*.hpp]
        H[Function Operations]
        I[OpenMP Threading]
        J[GIL Management]
    end
    
    subgraph "Build System"
        K[CMakeLists.txt]
        L[Compiler<br/>clang/gcc]
        M[Dynamic Libraries<br/>.so/.dylib/.dll]
    end
    
    A --> B
    B --> C
    C --> D
    D --> E
    E --> F
    F --> G
    G --> H
    H --> I
    G --> J
    
    K --> L
    L --> M
    M --> D
```