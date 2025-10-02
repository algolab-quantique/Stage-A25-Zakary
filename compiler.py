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
            # "-g",
            "-fopenmp",
            "-O3",
            # "-mavx2",
            "-flto=auto",
            # "-funroll-loops",
            # "-fprefetch-loop-arrays",
            "-march=native",
            # "-Wall",
            "-shared",
            "-std=c++17",
            "-fPIC",
            "$(python3 -m pybind11 --includes)",
            "src/bindings/paulicpp_bindings.cpp",
            "-o",
            "build/paulicpp$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
        print("PauliArray - OK")
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
            "src/bindings/densepaulicpp_bindings.cpp",
            "-o",
            "build/densepaulicpp$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
        print("DensePauliArray - OK")
        return True

    except subprocess.CalledProcessError as e:
        print(f"An error occurred while compiling densepauliarray: {e}")
        return False
    
def compile_voidops() -> bool:
    print("===== Compiling Void Operations C++ code =====")
    try:
        command = [
            "g++",
            # "-g",
            "-fopenmp",
            "-O3",
            # "-mavx2",
            "-flto=auto",
            # "-funroll-loops",
            # "-fprefetch-loop-arrays",
            "-march=native",
            # "-Wall",
            "-shared",
            "-std=c++17",
            "-fPIC",
            "$(python3 -m pybind11 --includes)",
            "src/bindings/voidops_bindings.cpp",
            "-o",
            "build/voidops$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
        print("Void Operations - OK")
        return True

    except subprocess.CalledProcessError as e:
        print(f"An error occurred while compiling void operations: {e}")
        return False

def compile_cpp(option="all"):
    ret : bool = True
    match option:
        case "all":
            compile_pauliarray()
            # compile_densepauliarray()
            compile_voidops()
        case "pa":
            ret = compile_pauliarray()
        case "dpa":
            ret = compile_densepauliarray()
        case "voidops":
            ret = compile_voidops()

    if ret:
        make_stubs(option)

def make_stub(libname, modname):
    print(f"===== Generating stubs for {libname} =====")
    try:
        env = os.environ.copy()
        env["PYTHONPATH"] = os.path.join(os.getcwd(), "build")
        # subprocess.run("stubgen -m paulicpp -o ./src/stubs", shell=True, check=True, env=env)
        command = [
            # "PYTHONPATH=" + cwd,
            "stubgen",
            "-m",
            modname,
            " -o",
            "./src/stubs"
        ]
        subprocess.run(" ".join(command), shell=True, check=True, env=env)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while generating stub for {libname}: {e}")

def make_stubs(option="all"):
    match option:
        case "all":
            make_stub("paulicpp", "paulicpp")
            # make_stub("densepaulicpp", "densepaulicpp")
            make_stub("voidops", "voidops")
        case "pa":
            make_stub("paulicpp", "paulicpp")
        case "dpa":
            make_stub("densepaulicpp", "densepaulicpp")
        case "voidops":
            make_stub("voidops", "voidops")
    
    

def add_pythonpath():
#     export PYTHONPATH=/home/user/Documents/STAGES/T1/Stage-A25-Zakary/stubs:$PYTHONPATH
# python3 test.py
    print("===== Setting PYTHONPATH =====")
    import sys, os
    sys.path.insert(0, os.path.join(os.getcwd(), "build"))
    print(f"PYTHONPATH set to {sys.path[0]}")

    

def main():
    add_pythonpath()
    compile_cpp("all")


if __name__ == "__main__":
    main()