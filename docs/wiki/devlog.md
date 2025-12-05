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


**Notes:**  
- First prototype of project logo made. Revisions to color scheme, positioning and vectorization needed for future iterations.
- Project has been added to PauliArray; Passes the exact same number of tests that PA passed without Z2P
- Used PauliArray with Z2P as a dependency in molecule simulation; Same results, no errors, but significantly faster! 
- Feature freeze is active for core C++ functions, but not infrastructure (testing suite) or Python files
- There is a proposal for [std::simd](https://en.cppreference.com/w/cpp/experimental/simd.html) to be added to the ISO Standard of C++26, which would allow for portable SIMD without having to deal with intrinsics. Interesting option for future developement and maintainance of library.