import subprocess
import os
import sys
import argparse


cwd = os.getcwd()

PKG_NAME = "pauliarray"
l_modules = ["paulicpp", "voidops", "sparsepaulicpp"]  # , "densepaulicpp"
supported_compilers = ["g++", "clang++", "gcc", "clang"]
step_num = 0 

def compile_generic(module, compiler) -> bool:
    global step_num

    if compiler not in supported_compilers:
        raise ValueError(f"Unsupported compiler: {compiler}")
    elif compiler == "g++" or compiler == "gcc":
        step_num += 1
        print(f"\n[{step_num}] --- Compiling {module}...")
        try:
            try:
                includes = subprocess.check_output(["python3", "-m", "pybind11", "--includes"], text=True).strip()
            except Exception:
                includes = subprocess.check_output("python3 -m pybind11 --includes", shell=True, text=True).strip()
            try:
                ext_suffix = subprocess.check_output(["python3-config", "--extension-suffix"], text=True).strip()
            except Exception:
                import sysconfig
                ext_suffix = sysconfig.get_config_var("EXT_SUFFIX") or ".so"

            out_path = f"{PKG_NAME}/src/build/{module}{ext_suffix}"

            command = (
                "g++ "
                "-fopenmp "
                "-Ofast "
                "-flto=auto "
                "-march=native "
                "-shared "
                "-std=c++20 "
                "-fPIC "
                f"{includes} "
                f"{PKG_NAME}/src/bindings/{module}_bindings.cpp "
                "-o "
                f"{out_path}"
            )
            print(f"Running: {command}")
            subprocess.run(command, shell=True, check=True)
            print(f"{module} compiled successfully.")
            print(f"Output located at: {out_path}")
            return True
        except subprocess.CalledProcessError as e:
            print(f"An error occurred while compiling {module}: {e}")
            return False 
        

    elif compiler == "clang++" or compiler == "clang":
        step_num += 1
        print(f"\n[{step_num}] --- Compiling {module}...")
        try:
            try:
                includes = subprocess.check_output(["python3", "-m", "pybind11", "--includes"], text=True).strip()
            except Exception:
                includes = subprocess.check_output("python3 -m pybind11 --includes", shell=True, text=True).strip()
            try:
                ext_suffix = subprocess.check_output(["python3-config", "--extension-suffix"], text=True).strip()
            except Exception:
                import sysconfig
                ext_suffix = sysconfig.get_config_var("EXT_SUFFIX") or ".so"

            out_path = f"{PKG_NAME}/src/build/{module}{ext_suffix}"

            command = (
                "clang++ "
                "-fopenmp=libomp "
                "-Ofast "
                "-flto=auto "
                "-march=native "
                "-shared "
                "-std=c++20 "
                "-fPIC "
                f"{includes} "
                f"{PKG_NAME}/src/bindings/{module}_bindings.cpp "
                "-o "
                f"{out_path}"
            )
            print(f"Running: {command}")
            subprocess.run(command, shell=True, check=True)
            print(f"{module} compiled successfully.")
            print(f"Output located at: {out_path}")
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


def compile_cpp(option="all", compiler="g++"):
    ret : bool = True
    match option:
        case "all":
            for module in l_modules:
                ret = ret and compile_generic(module, compiler)
            print(f"\nAll modules compiled: {ret}")
        case "pa":
            ret = compile_generic("paulicpp", compiler)
        case "dpa":
            ret = compile_generic("densepaulicpp", compiler)
        case "voidops":
            ret = compile_generic("voidops", compiler)
            
    if ret:
        make_stubs(option)
    

def add_pythonpath():
#     export PYTHONPATH=/home/user/Documents/STAGES/T1/Stage-A25-Zakary/stubs:$PYTHONPATH
# python3 test.py
    print("===== Setting PYTHONPATH =====")
    sys.path.insert(0, os.path.join(os.getcwd(), f"{PKG_NAME}/src/build"))
    # print(f"PYTHONPATH set to {sys.path[0]}")

    

def main():

    parser = argparse.ArgumentParser(description="Compile C++ modules and generate stubs.")
   
    parser.add_argument(
        "--module", "-m",
        type=str,
        choices=["all", "pa", "dpa", "voidops"],
        default="all",
        help="Choose which module to compile (default: all)"
    )
    parser.add_argument(
        "--compiler", "-c",
        type=str,
        choices=supported_compilers,
        default="g++",
        help="Choose the compiler to use (default: g++)"
    )
    add_pythonpath()
    print(sys.path)
    compile_cpp(option=parser.parse_args().module, compiler=parser.parse_args().compiler)


if __name__ == "__main__":
    main()