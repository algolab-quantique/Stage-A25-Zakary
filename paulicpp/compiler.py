import subprocess
import os
import sys


cwd = os.getcwd()

PKG_NAME = "pauliarray"
l_modules = ["paulicpp", "voidops"]  # , "densepaulicpp"
step_num = 0 

def compile_generic(module) -> bool:
    global step_num
    step_num += 1
    print(f"\n[{step_num}] --- Compiling {module}...")
    try:
        command = [
            "g++",
            "-g",
            "-fopenmp",
            "-O3",
            "-flto=auto",
            "-march=native",
            # "-Wall",
            "-shared",
            "-std=c++17",
            "-fPIC",
            "$(python3 -m pybind11 --includes)",
            f"{PKG_NAME}/src/bindings/{module}_bindings.cpp",
            "-o",
            f"{PKG_NAME}/src/build/{module}$(python3-config --extension-suffix)"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
        print(f"{module} compiled successfully.")
        print(f"Output located at: {PKG_NAME}/src/build/{module}.xxx.so")
        return True
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while compiling {module}: {e}")
        return False  


def make_stub_generic(libname, modname):
    global step_num
    step_num += 1
    print(f"\n[{step_num}] --- Generating stubs for {libname}...")
    try:
        env = os.environ.copy()
        env["PYTHONPATH"] = os.path.join(os.getcwd(), PKG_NAME, "src", "build")
        # subprocess.run("stubgen -m paulicpp -o ./src/stubs", shell=True, check=True, env=env)
        command = [
            # "PYTHONPATH=" + cwd,
            "stubgen",
            "-m",
            modname,
            "-o",
            f"./{PKG_NAME}/stubs"
        ]
        subprocess.run(" ".join(command), shell=True, check=True, env=env)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while generating stub for {libname}: {e}")

def make_stubs(option="all"):
    match option:
        case "all":
            for module in l_modules:
                make_stub_generic(module, f"{module}")

        case "pa":
            make_stub_generic("paulicpp", "paulicpp")
        case "dpa":
            make_stub_generic("densepaulicpp", "densepaulicpp")
        case "voidops":
            make_stub_generic("voidops", "voidops")


def compile_cpp(option="all"):
    ret : bool = True
    match option:
        case "all":
            for module in l_modules:
                ret = ret and compile_generic(module)
            print(f"\nAll modules compiled: {ret}")
        case "pa":
            ret = compile_generic("paulicpp")
        case "dpa":
            ret = compile_generic("densepaulicpp")
        case "voidops":
            ret = compile_generic("voidops")
            
    if ret:
        make_stubs(option)
    

def add_pythonpath():
#     export PYTHONPATH=/home/user/Documents/STAGES/T1/Stage-A25-Zakary/stubs:$PYTHONPATH
# python3 test.py
    print("===== Setting PYTHONPATH =====")
    sys.path.insert(0, os.path.join(os.getcwd(), f"{PKG_NAME}/src/build"))
    print(f"PYTHONPATH set to {sys.path[0]}")

    

def main():

    add_pythonpath()
    print(sys.path)
    compile_cpp("all")


if __name__ == "__main__":
    main()