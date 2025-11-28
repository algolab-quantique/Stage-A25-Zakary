# FAQ
### Q: Is having a header and a bindings file for each module redundant if both declare the same functions?
**A:** ...Yes, and No.

The header file (.h) for a module can only declare C++'s side of the code. Functions, defines, includes and pragmas live there. 
The binding file serves as the gateway between Python's calls and C++'s responses. In all current cases, the function names are exactly the same on both sides, but its possible to have a function called `foo()` in Python that actually calls `bar()` from C++


