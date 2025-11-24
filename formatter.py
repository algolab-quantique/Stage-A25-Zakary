import sys, os, black, subprocess


def format_black():
    try:
        subprocess.run([sys.executable, "-m", "black", ".", "--line-length", "100"], check=True)
        print("Python code formatted successfully with Black.")
        return 0
    except subprocess.CalledProcessError as e:
        print(f"Error during formatting: {e}")
        return -1

def format_clang():
    files = []
    for root, _, filenames in os.walk("."):
        for filename in filenames:
            if filename.endswith((".cpp", ".h", ".hpp", ".cxx", ".cc")):
                files.append(os.path.join(root, filename))
    try:
        subprocess.run(["clang-format", "-i"] + files, check=True)
        print("C/C++ code formatted successfully with clang-format.")
        return 0
    except subprocess.CalledProcessError as e:
        print(f"Error during formatting: {e}")
        return -2


if __name__ == "__main__":
    format_black()
    format_clang()
