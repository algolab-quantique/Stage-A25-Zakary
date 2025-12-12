# Devlog
This file documents changes, fixes, known issues and miscelaneous notes for each week. Please do not change previous entries.


# 2025
## November
### 24th-28th
**Features Added:**
- Created Devlog
- Created `gh-pages` branch. Manual overwrite and pushes needed.
- Automatic deployement of Doxygen documentation to the GH pages on any new push to `main`. Manual generation of docs still needed by typing `doxygen Doxyfile` before pushes.

**Bug Fixes:**
- squashed bug 
  
**TODOs & Known Issues:**
- Implement automatic Python and C++ test suite
- Fully integrate project with PauliArray

**Notes:**  
- Licensed project under Apache 2.0
- A feature freeze next week would be optimal before final release.



## December
### 1st-5th
**Features Added:**
- Created `dev` branch
- Created `tests/` directory. Currently only Python unit tests are available.
- Created `benchmarks/` directory. Contains mostly tests to be fused with pauliarray
  
**Bug Fixes:**
- Never actually needed to manually generate doxygen documentation! All documents in docs/html/ purged in branch `main`.
- Added dummy arguments in `unordered_unique()`'s Python wrapper to match the ones found in `unique()`. This enables direct replacement of the function in PA.
  
**TODOs & Known Issues:**
- Implement automatic Python and C++ test suite --> Catch2 yet to be implemented
- Fully integrate project with PauliArray
- Multithreading is inconsistent for small arrays; suspect [the GIL](Optimizations.md) as root cause. Probable fix is to release the GIL during each bitwise operation.
- pybind-stubgen has difficulties and crashes when creating stubs for files using external libraries (e.g., xxhash)
- std::unordered_map is inneficient (?). Better alternatives are available online. Main use case for a better hashmap is in `unordered_unique()`
- `concatenate()` should be split into two (one for each axis). There should also be an option or other function that permits direct insertion of one matrix onto/into another via an index parameter.
- Doxygen comments for Python are not displayed in HTML output

**Notes:**  
- First prototype of project logo made. Revisions to color scheme, positioning and vectorization needed for future iterations.
- Project has been added to PauliArray; Passes the exact same number of tests that PA passed without Z2P
- Used PauliArray with Z2P as a dependency in molecule simulation; Same results, no errors, but significantly faster! 
- Feature freeze is active for core C++ functions, but not infrastructure (testing suite) or Python files
- There is a proposal for [std::simd](https://en.cppreference.com/w/cpp/experimental/simd.html) to be added to the ISO Standard of C++26, which would allow for portable SIMD without having to deal with intrinsics. Interesting option for future developement and maintainance of library.



### 8th-12th
**Features Added:**
- Added a Table Of Content to the right of every .md pages when generating doxygen documentation
- Added a copy button to code blocks in doxygen documentation
- Created `builder.py`, a simple python script that tries to build and upload the project to PyPI (only test index for now)
- Added --no-omp to builder, a command line argument to skip OMP multi-threading when compiling
- Added a bunch of benchmarks for quantum molecule simulation

**Bug Fixes:**
- Fixed GH Pages doxygen build not having a light/dark mode switch. 
- Fixed doxygen TOC appearing on non-markdown files
  
**TODOs & Known Issues:**
- Implement automatic Python and C++ test suite
- Fully integrate project with PauliArray
- Multithreading is inconsistent for small arrays; suspect [the GIL](optimizations.md) as root cause. Probable fix is to release the GIL during each bitwise operation.
- pybind-stubgen has difficulties and crashes when creating stubs for files using external libraries (e.g., xxhash)
- std::unordered_map is inneficient (?). Main use case for a better hashmap is in `unordered_unique()`. This applies to not only the container, but also the hash function. 
- `concatenate()` should be split into two (one for each axis). There should also be an option or other function that permits direct insertion of one matrix onto/into another via an index parameter.
- Doxygen output does not display Python comments/docstrings in HTML
- Doxygen output does not display the Architecture's Mermaid graph.
- Doxygen output could benefit from having sections in the tree view on the left (seperate hand-made markdown and auto-generated APIs)
- The `_bindings.cpp` have the necessary code to transfer Python and C++, but more documentation could be added (such as function briefs and parameters)

**Notes:**  
- Last week of internship, trying to wrap everything up - Feel free to modify whatever you want, as the project is licensed under Apache 2.0. I suspect that most of what is written could be optimized even further with better knowledge of computer architecture/C++.


---

# What I Know
This section will try to lay out everything that has been done in my three months working on the project by categorizing what i know (or don't), and why it works.

## Rock-Solid and Reliable
- Formatters: Always work on manual triggers
- Formatters (GitHub Actions): Should always check the formatting correctly.
- Pre-commit actions: Will always work if the .yaml file is not modified.
- All `_bitwise()` operations: These work on all dimensions of any input! All NumPy calls can be directly substituted for these, with an expected gain of ~2x on most operations.
- Doxygen Documentation (local): Never had issues when running `doxygen Doxyfile`

## Works, with Caveat(s)
- Doxygen Deployement (GitHub Actions): This works 98% of the time, but there has previoulsy been at least 1 bug unique to the GH Pages that wasn't on my local machine. Awesome-Doxygen-CSS is great, but is also finnicky; Had to modify some JS for Tables of Content to work, and the Doxyfile is also a bit mysterious.
- CMake: Build process works on both of my Linux machines and the Intel Macbook Pro. Build FAILED using M1 Macbook if using OpenMP - Disabling OpenMP directives let the build succeed.
- `transpose()`, `matmul()`, `concatenate()` and `gauss_jordan_inverse()`: They all "work" fine, but they have NOT been heavily tested! They *should* be OK and could replace the calls from `bit_operations.py` in PA, but edge cases could break them.
- `builder.py`: This script tries to automatically build and upload the project to TestPyPI. You need to manually update the version number in both `pyproject.toml` and `/z2r_accel/__init__.py`. The script also fails if there is already a .whl file in PyPI.
- Pybind11: My current workflow doesnt create issues, but it is a bit barebones; There are no custom classes or structs passed to Python, and there is very little (if none at all) OOP. If maintenance is needed to add such things, i dont know how to do it efficiently
- Everything in the devlog's TODOs/issues
- Everything in the Doxygen documentation's Todo/Deprecated Lists

## Mysterious Behavior
- pybind11-stubgen: I think this was a mistake to use. It panics often and makes the CMake build step fail. Why this happens is beyond me. Consider mypy in the future?
- PauliArray inputs: I have not looked deep in the PA source code. Sometimes, an input can be a |V13, but also can transform to the next power of 2 (|V16 in this case). This is behaviour is erratic and i could not find a specific way to initialize PauliArrays such that they were always one way or the other. 


<!-- 

# Template
**Features Added:**
- added feature

**Bug Fixes:**
- squashed bug 
  
**TODOs & Known Issues:**
- {Copy previous week's section!}

**Notes:**  
- details 
  
  -->
