import subprocess
import os
import sys

cwd = os.getcwd()
# out_dir = os.path.join(cwd, "out")

def compile_pauliarray():
    print("===== Compiling PauliArray C++ code =====")
    try:
        command = [
            "g++",
            "-fopenmp",
            "-O3",
            "-Wall",
            "-shared",
            "-std=c++17",
            "-fPIC",
            "$(python3 -m pybind11 --includes)",
            "bindings/paulicpp_bindings.cpp",
            "-o",
            "paulicpp$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while compiling pauliarray: {e}")

def compile_densepauliarray():
    print("===== Compiling DensePauliArray C++ code =====")
    try:
        command = [
            "g++",
            "-fopenmp",
            "-O3",
            "-Wall",
            "-shared",
            "-std=c++17",
            "-fPIC",
            "$(python3 -m pybind11 --includes)",
            "bindings/densepaulicpp_bindings.cpp",
            "-o",
            "densepaulicpp$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while compiling densepauliarray: {e}")

def compile_cpp(option="all"):
    match option:
        case "all":
            compile_pauliarray()
            compile_densepauliarray()
        case "pa":
            compile_pauliarray()
        case "dpa":
            compile_densepauliarray()

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
#     export PYTHONPATH=/home/zwouklebleu/Documents/STAGES/T1/Stage-A25-Zakary/stubs:$PYTHONPATH
# python3 test.py
    print("===== Setting PYTHONPATH =====")
    # stubs_path = os.path.join(cwd, "stubs")
    # if stubs_path not in sys.path:
    #     sys.path.append(stubs_path)
    try:
        command = [
            "export",
            "PYTHONPATH=" + os.path.join(cwd, "stubs") + ":$PYTHONPATH"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while setting PYTHONPATH: {e}")
    

def main():
    compile_cpp("dpa")
    add_pythonpath()


if __name__ == "__main__":
    main()