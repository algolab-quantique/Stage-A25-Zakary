# External Tools
## Auto-formatters
All formatters can be automatically called before each commit with the help of [pre-commit](https://pre-commit.com/)! To set it up, simply nagivate to the root of the project and call
```
pre-commmit install
```
This will make it clang-format and black are called before any commit is made. 

### For Python
We use [Black](https://github.com/psf/black), with `--line-length 100`


### For C++
We use [clang-format](https://clang.llvm.org/docs/ClangFormat.html).

All of the options are found inside of the `.clang-format` file. LLVM's format is the base, with some specific overridden options:
- `UseTab: Never`: Always forces spaces for indentation
- `IndentWidth: 4`: Use 4-space standard for indentation
- `IndentPPDirectives: AfterHash`: Indent preprocessor directives after their '#'

### For CMake
We use [cmake-format](https://cmake-format.readthedocs.io/en/latest/installation.html).

It's an old tool that is a bit superfluous (since there's only 1 cmake file). Not very necessary to keep. 


---

## Build Systems
### In Python
[scikit-build-core](https://github.com/scikit-build/scikit-build-core) is used to bridge the gap between a Python end-user trying to `pip install x` and CMake manually compiling dynamic libraries. It makes installation of the library a simple 1-step process for most users.

### In C++
[CMake](https://cmake.org/) is the tool that generates files for OS-specific compilation of source code. It has great compatibility with pybind11 and scikit-build-core.


---

## Documentation
Code comments are written in the [Doxygen](https://www.doxygen.nl/) format, with a better looking CSS/JS from [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css)
Any markdown files created should be located in `/docs/wiki/`. 

On any push to `main`, a GH Action will trigger and try to generate the documentation and push changes to the `gh-pages` branch. This usually takes a minute. If successful, the GH will update the website at: https://algolab-quantique.github.io/Stage-A25-Zakary/

To test documentation without having to merge/push on `main`, navigate to project root and call `doxygen Doxyfile`. Output should be located in `/docs/html/`. Open `index.html` with a web browser to navigate the site locally.


---

## Pybind11 (and why it was chosen)
Pybind11 was chosen for its minimal boilerplate, seamless NumPy integration, and modern C++ support. It allows us to expose C++ functions and classes to Python with very little overhead, making it easy to maintain and extend the codebase.

Since pybind is chosen as the interface for this project, the stub generator is obviously [pybind-stubgen](https://github.com/sizmailov/pybind11-stubgen). However, some problems have appeared with it, notably when trying to add external libraries. It may get replaced with mypy's stubgen in the future.

Other alternatives were considered, but ultimetly rejected:

1. **Numba**
    - While Numba is excellent for accelerating generic Python code, our data structures are highly specialized and already optimized in C++. Numba would offer limited additional benefit and cannot easily accelerate C++ code.

2. **Cython**
   - Cython is powerful for optimizing Python code and interfacing with C, but it often requires rewriting parts (if not most) of functions to take full advantage of its features. At that point, we might as well rewrite everything in pure C/C++

3. **Boost.Python**
     - Boost.Python is feature-rich but introduces significant overhead and complexity. It requires more boilerplate code, demands loads more dependencies, and is generally slower compared to pybind11.





