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
  
**Bug Fixes:**
- Never actually needed to manually generate doxygen documentation! All documents in docs/html/ purged in branch `main`.
  
**TODOs & Known Issues:**
- Implement automatic Python and C++ test suite
- Fully integrate project with PauliArray
- Multithreading is inconsistent for small arrays; suspect [the GIL](Optimizations.md) as root cause. Probable fix is to release the GIL during each bitwise operation.
- pybind-stubgen has difficulties and crashes when creating stubs for files using external libraries (e.g., xxhash)
- std::unordered_map is inneficient (?). Better alternatives are available online. Main use case for a better hashmap is in `unordered_unique()`
- `concatenate()` should be split into two (one for each axis). There should also be an option or other function that permits direct insertion of one matrix onto/into another.
- 

**Notes:**  
- First prototype of project logo made. 