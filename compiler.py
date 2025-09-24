import subprocess
import os
import sys

cwd = os.getcwd()
# out_dir = os.path.join(cwd, "out")

def compile_pauliarray() -> bool:
    print("===== Compiling PauliArray C++ code =====")
    try:
        command = [
            "g++",
            
            "-fopenmp",
            "-O3",
            "-march=native",
            # "-Wall",
            "-shared",
            "-std=c++17",
            "-fPIC",
            "$(python3 -m pybind11 --includes)",
            "bindings/paulicpp_bindings.cpp",
            "-o",
            "paulicpp$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while compiling pauliarray: {e}")
        return False

def compile_densepauliarray() -> bool:
    print("===== Compiling DensePauliArray C++ code =====")
    try:
        command = [
            "g++",
            "-g",
            "-fopenmp",
            "-O3",
            "-march=native",
            # "-Wall",
            "-shared",
            "-std=c++17",
            "-fPIC",
            "$(python3 -m pybind11 --includes)",
            "bindings/densepaulicpp_bindings.cpp",
            "-o",
            "densepaulicpp$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
        print("DensePauliArray - OK")
        return True

    except subprocess.CalledProcessError as e:
        print(f"An error occurred while compiling densepauliarray: {e}")
        return False

def compile_cpp(option="all"):
    ret : bool = False
    match option:
        case "all":
            compile_pauliarray()
            compile_densepauliarray()
        case "pa":
            ret = compile_pauliarray()
        case "dpa":
            ret = compile_densepauliarray()

    if ret:
        make_stubs(option)

def make_stub(libname, modname):
    print(f"===== Generating stubs for {libname} =====")
    try:
        command = [
            # "PYTHONPATH=" + cwd,
            "stubgen",
            "-m",
            modname,
            " -o",
            "./stubs"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while generating stub for {libname}: {e}")

def make_stubs(option="all"):
    match option:
        case "all":
            make_stub("paulicpp", "paulicpp")
            make_stub("densepaulicpp", "densepaulicpp")
        case "pa":
            make_stub("paulicpp", "paulicpp")
        case "dpa":
            make_stub("densepaulicpp", "densepaulicpp")
    
    

def add_pythonpath():
#     export PYTHONPATH=/home/user/Documents/STAGES/T1/Stage-A25-Zakary/stubs:$PYTHONPATH
# python3 test.py
    print("===== Setting PYTHONPATH =====")
    stubs_path = os.path.join(cwd, "stubs")
    bindings_path = os.path.join(cwd, "bindings")
    
    current_pythonpath = os.environ.get('PYTHONPATH', '')
    new_pythonpath = f"{bindings_path}:{stubs_path}:{current_pythonpath}"
    os.environ['PYTHONPATH'] = new_pythonpath
    
    if stubs_path not in sys.path:
        sys.path.insert(0, stubs_path)
    if bindings_path not in sys.path:
        sys.path.insert(0, bindings_path)
    
    print(f"PYTHONPATH set to: {new_pythonpath}")

    

def main():
    compile_cpp("dpa")
    # add_pythonpath()


if __name__ == "__main__":
    main()