# The Z2 Project
<img src = "https://github.com/algolab-quantique/Stage-A25-Zakary/blob/main/docs/assets/logo_cropped.png" align="right" width="200">
Z2P aims to provide performance and memory efficient functions for NumPy NDArray of dtype Void 




# Installation

To install this package, you can try by using pip:
``` bash
pip install z2r_accel
```
If there is no available distribution available for your machine, you'll need to install it from git. You can try this by pasting:
```
pip install numpy pybind11 pybind11-stubgen scikit-build-core
pip install git+https://github.com/algolab-quantique/Stage-A25-Zakary.git 
```
If this still doesn't work, you'll need to build it [directly from source](#building-from-source).

---

## Building from source
### Requirements
- Python 3.10+
- GCC or Clang, with versions that support at least C++20
- Unix system
- OpenMP (recommended)
> [!IMPORTANT]
> If you are on MacOS, it's recommended to manually install Clang and OpenMP with [Brew](https://brew.sh/) by using `brew install llvm libomp`. 
>
> On most Linux distributions, GCC/clang are pre-installed and with OpenMP support. No need to do anything extra.

### Building with OpenMP

Clone this repository, navigate to the the directory where `pyproject.toml` is located and do:
```
pip install .
```
This should automatically compile the C++ code based on your machine and link place it to the correct path.

---

### Building without OpenMP
> [!WARNING]
> Once again, this is **not** the recommended way to install this library!

Clone this repository, navigate to the the directory where `pyproject.toml` is located and do:
``` console
pip install . --config-settings=cmake.define.USE_OPENMP=OFF
```
---

# Documentation
Internal documentation and API can be found [here](https://algolab-quantique.github.io/Stage-A25-Zakary/). 
