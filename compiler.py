import subprocess
import os
import sys

cwd = os.getcwd()
# out_dir = os.path.join(cwd, "out") 

def compile_cpp():
    print("===== Compiling C++ code =====")
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
        print(f"An error occurred while compiling C++ code: {e}")

def make_stubs():
    print("===== Generating stubs =====")
    try:
        command = [
            # "PYTHONPATH=" + cwd,
            "stubgen",
            "-m",
            "paulicpp"
            " -o",
            "./stubs"
        ]
        subprocess.run(" ".join(command), shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while generating stubs: {e}")

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
    compile_cpp()
    make_stubs()
    add_pythonpath()


if __name__ == "__main__":
    main()