# Project Name
<!-- <img src = "https://github.com/algolab-quantique/Stage-A25-Zakary/blob/main/assets/pa_logo.svg" align="right" width="220"> -->
Accelerator project which handles arrays of packed binary values.


### Requirements
- Python 3.10+
- GCC or Clang, with versions that support at least C++20
- Unix system

# Installation
If you are on macOS, it is highly recommended to use OpenMP, as it provides multithreading to the C++ library. Whilst it is not ***necessary*** to do so, you may notice worst performance.

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


---

## Without OpenMP
Once again, this is **not** the recommended way to install this library! You have been warned.

Clone this repo and navigate to the `paulicpp` directory. You may need to create a virtual environment depending on your needs. After that, do:
``` console
pip install . --config-settings=cmake.define.USE_OPENMP=OFF
```
---

# Documentation
Internal documentation can be found on the [repos's wiki](https://github.com/algolab-quantique/Stage-A25-Zakary/wiki). 
