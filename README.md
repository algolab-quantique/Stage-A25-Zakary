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
| unique()                | 1.5x to 3x | Almost...      | - 'axes' parameter is not implemented. da  |
| unordered_unique()      | 2-10x | Nope! | - 'axes' parameter is not implemented. <br/>- If i get parralization to work, we get up to 7x consistently. <br/>- Only works for 2D arrays  |
| bitwise_commute_with()  | 1x to 4x | Yes          | Weird Numpy array stuff going on. |



#### Sparse structure tests
| Sparse                  | Speedup  | 1-to-1 ? | Notes |
|-----------------        |-------   |----------|------------|
| std::vector<dpoint>  | 0.002x to 40x | NO!      | Time complexity depends on the proximity of the 1s. This ranges from Ω(1) to O(n) |
| std::set<dpoint> | even slower than vectors | NO!          | Since a vector's index lookup is O(1) and a set's (red-black tree) is O(log n), we get slowed more and more as comlexity increases such that in the worst case, the operations can take up to O(n log n). |








## Structure
Anything outside of `/paulicpp` is subject to deletion. The project structure is the exact same as PauliArray, with the following additions:

saliu
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
|   │   │   └── {MODULE}.pyi
|   │   └── {MODULE}.hpp
|   │
|   └── utils
|
└── compiler.py
```

- `/src` Contains the C++ source code (.hpp) for every modules.
- `/src/bindings` Contains the necessary pybind11 bindings necessary for the translation to and from Python.
- `src/build` Contains the dynamic shared libraries and stub files.
    <br>Linux/macOS -> xyz.so
    <br>Windows -> xyz.dll
- `compiler.py` Deprecated file.






### Requirements
- Python 3.10+
- GCC or Clang, with versions that support at least C++20
- Unix system
- Having lots of time to waste to debug this library.

# Installation
If you are on macOS, it is highly recommended to use OpenMP, as it provides multithreading to the C++ library. Whilst it is not ***necessary*** to do so, you may notice worst performance.

| Platform | Official support? | 
|----------|-------   |
| Windows | No |
| Windows (ARM) | No |
| Linux  | Yes |
| Linux (ARM) | No |
| Mac (Intel CPU) | Yes |
| Mac (Apple Silicon) | Yes |


## With OpenMP

### Step 1:
#### MacOS
If you are on MacOS, please install [Brew](https://brew.sh/), then paste the following into your terminal: 
```
brew install llvm libomp
```
#### Linux
Most linux distros come with with GCC or Clang pre-installed. Check that your compiler is compliant with the C++20 standard and that you have OpenMP installed. 

### Step 2:
Then, clone this repo and navigate to the `paulicpp` directory. You may need to create a virtual environment depending on your needs. After that, do:
```
pip install .
```
This should automatically compile the C++ code based on your machine and link place it to the correct path.



## Without OpenMP
This method is agnostic to your operating system, you juste need GCC or Clang that can compile C++20.

Again, this is **not** the recommended way to install this library! You have been warned.

Clone this repo and navigate to the `paulicpp` directory. You may need to create a virtual environment depending on your needs. After that, do:
``` console
pip install . --config-settings=cmake.define.USE_OPENMP=OFF
```

## Verification

Once installed, you may call the library as usual within a Python script with `import pauliarray`.
<br> If you want to check what backend the library uses (C++ or a Python fallback), try inside a Python file:

``` python
import pauliarray.binary.void_operations as vops
print(vops.get_backend())
```
If you get *'C++*', congrats, the library has been sucessfully installed and compiled!
<br> If you get '*Python*', something went terribly wrong with the installation. Try to install with the other way (i.e Try WITHOUT OpenMP if you tried to install it WITH and vice-versa). If it still doesnt work, please contact me or submit an issue
