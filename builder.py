import os, sys, subprocess
import z2r_accel as z2
import argparse

IS_LINUX: bool = False
if sys.platform.startswith("linux"):
    IS_LINUX = True


def build(USE_OMP: bool = True):
    # python -m build --outdir ./dist/{PROJECT_VERSION}
    if USE_OMP:
        subprocess.run(
            [sys.executable, "-m", "build", "--outdir", f"./dist/{z2.__version__}"],
            check=True,
        )
    else:
        subprocess.run(
            [
                sys.executable,
                "-m",
                "build",
                "--config-setting=cmake.define.USE_OPENMP=OFF",
                "--outdir",
                f"./dist/{z2.__version__}",
            ],
            check=True,
        )


def auditwheel():
    # auditwheel repair -w ./dist/{PROJECT_VERSION} ./dist/{PROJECT_VERSION}/z2r_accel.whl
    dist_dir = f"./dist/{z2.__version__}"
    for filename in os.listdir(dist_dir):
        if filename.endswith(".whl"):
            filepath = os.path.join(dist_dir, filename)
            subprocess.run(
                ["auditwheel", "repair", filepath, "-w", dist_dir],
                check=True,
            )


def upload():
    # python3 -m twine upload --repository testpypi dist/*manylinux*.whl dist/*.tar.gz
    dist_dir = f"./dist/{z2.__version__}"

    if IS_LINUX:
        subprocess.run(
            [
                sys.executable,
                "-m",
                "twine",
                "upload",
                "--repository",
                "testpypi",
                os.path.join(dist_dir, "*manylinux*.whl"),
                os.path.join(dist_dir, "*.tar.gz"),
            ],
            check=True,
        )
    else:
        subprocess.run(
            [
                sys.executable,
                "-m",
                "twine",
                "upload",
                "--repository",
                "testpypi",
                os.path.join(dist_dir, "*.whl"),
                os.path.join(dist_dir, "*.tar.gz"),
            ],
            check=True,
        )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--no-omp",
        action="store_true",
        help="Build without OpenMP support (for macOS compatibility)",
    )

    args = parser.parse_args()
    if args.no_omp:
        print("Building without OpenMP support!")
        build(USE_OMP=False)
    else:
        build()

    if IS_LINUX:
        auditwheel()
    upload()


if __name__ == "__main__":
    main()
