# Packaging
Since we have custom C++ code that links to external libraries, in order to distribute the project to PyPI correctly, it needs to be built on 
https://packaging.python.org/en/latest/tutorials/packaging-projects/

You will need to install `twine` and `build` via pip. 
The project's API key will also be necessary to upload to PyPI.

Z2P can be found on [TestPyPi](https://test.pypi.org/project/z2r-accel/)



# Automatic upload
You can try to automatically upload the package by calling `builder.py`.

If you are on MacOS and DONT have OpenMP installed, run the script with:
```
python3 builder.py --no-omp
```

# Manual upload
## On Linux
For various [reasons](https://peps.python.org/pep-0513/#rationale), packaging on Linux is a bit difficult. Currently, [auditwheel](https://github.com/pypa/auditwheel) is used to repair the `linux_x86_64` and change it to some sort of `manylinux` tag. (?)

You'll need to install `auditwheel`.

In the project's root folder, build it:
```
python -m build --outdir ./dist/{PROJECT_VERSION}
# python -m build --outdir ./dist/0.0.5/
```

Then, repair the .whl file. 
```bash
auditwheel repair -w ./dist/{PROJECT_VERSION} ./dist/{PROJECT_VERSION}/z2r_accel.whl
# the file name for you is probably different than this. It should follow the convention of:
#                      z2r_accel-{VER}-{PYTHON}-{ABI}-{PLATFORM}.whl

```
If you don't get any errors, thi will produce a file that looks roughtly like `z2r_accel-0.0.2-cp312-cp312-manylinux_2_39_x86_64`. You can now safely upload the files to PyPI:

```
twine upload ./dist/{PROJECT_VERSION}/*manylinux*.whl ./dist/{PROJECT_VERSION}/*.tar.gz 
```


## On MacOS
TODO: Write documentation

## Testing install
```
pip install -i https://test.pypi.org/simple/ z2r-accel
```