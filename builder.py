import os, sys, subprocess
import z2r_accel as z2


def build():
    # python -m build --outdir ./dist/{PROJECT_VERSION}
    subprocess.run(
        [sys.executable, "-m", "build", "--outdir", f"./dist/{z2.__version__}"],
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


def main():
    build()
    if sys.platform.startswith("linux"):
        auditwheel()
    upload()


if __name__ == "__main__":
    main()
