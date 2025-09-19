# PauliC++ 
WIP project which converts some of [PauliArray](https://github.com/algolab-quantique/pauliarray)'s functions to C++ in order to speed up the library all the whilst retaining the easy-to-use Python structure and syntax.

## Structure
- `/src` : folder for source code & headers written  in C++
- `/bindings` : folder for PyBind11 bindings 
- `/stubs` : folder for stubs, which are the python declaration of the C++ classes/functions. This is needed for intellisense and autocomplete to work
- `compiler.py` : small script which tries to compile from source, create stubs and automate installation the process. May or may not work...!

## Installation
#### Requirements
- A python3.x.x interpreter
- You need these Python librairies, installable via ***pip***: `PyBind11`, `mypy`, `NumPy` and `Matplotlib`
- A modern C++ compiler which supports at least C++17. For the moment **only** `gcc/g++` is supported
- A Unix system

Go to the directory where `compiler.py` is in, then run the script. If no errors occur, you can access the exposed C++ calls as you would any other Python library via `import paulicpp` and `import densepaulicpp`. 