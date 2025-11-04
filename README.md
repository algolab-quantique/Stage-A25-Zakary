# PauliArray, with a side of C++
<img src = "https://github.com/algolab-quantique/Stage-A25-Zakary/blob/main/assets/pa_logo.svg" align="right" width="220">
Accelerator project which converts some of [PauliArray](https://github.com/algolab-quantique/pauliarray) backend to C++ in order to speed up the library all the whilst retaining the easy-to-use Python structure and syntax.


### Requirements
- Python 3.10+
- GCC or Clang, with versions that support at least C++20
- Unix system

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
Once again, this is **not** the recommended way to install this library! You have been warned.

Clone this repo and navigate to the `paulicpp` directory. You may need to create a virtual environment depending on your needs. After that, do:
``` console
pip install . --config-settings=cmake.define.USE_OPENMP=OFF
```

---

## Verification

Once installed, you may call the library as usual within a Python script with `import pauliarray`.
<br> If you want to check what backend the library uses (C++ or a Python fallback), try inside a Python file:

``` python
import pauliarray.binary.void_operations as vops
print(vops.get_backend())
```
If you get *'C++*', congrats, the library has been sucessfully installed and compiled!
<br> If you get '*Python*', something went terribly wrong with the installation. Try to install with the other way (i.e Try WITHOUT OpenMP if you tried to install it WITH and vice-versa). If it still doesnt work, please submit an issue
