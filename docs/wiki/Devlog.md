# 2025
## November
### 24th-28th
- Created `gh-pages` branch. Manual overwrite and pushes needed.
- Licensed project under Apache 2.0
- Automatic deployement of Doxygen documentation to the GH pages on any new push to `main`. Manual generation of docs still needed by typing `doxygen Doxyfile` before pushes.

## December
### 1st-5th
- When the first iterations of the dense bitwise operations was created, a strange pattern arrose: if multithreading was used for small-ish array sizes, the program would sometimes be more efficient, and sometimes spikes up to 100x slower than expected. This bug was never fixed. However, i have a hunch that this bug occurs due to [the GIL](Optimizations.md)! One way to fix this would be to release the GIL when iterating over the 64 bit an tail chunks. If this issue is fixed, bitwise operations can (possibly) become 2x faster in some cases
- Created `dev` branch
- 
