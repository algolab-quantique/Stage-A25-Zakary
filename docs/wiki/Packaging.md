Since we have custom C++ code that links to external libraries, in order to distribute the project to PyPI correctly, it needs to be built on 
https://packaging.python.org/en/latest/tutorials/packaging-projects/

You will need to install `twine` and `build` via pip.

## On Linux
For various [reasons](https://peps.python.org/pep-0513/#rationale), packaging on Linux is a bit difficult. Currently, [auditwheel](https://github.com/pypa/auditwheel) is used to repair the `linux_x86_64` and change it to some sort of `manylinux` tag. (?)

You'll need to install `auditwheel`.

In the project's root folder, build it:
```
python -m build
```

Then, repair the .whl file. 
```bash
auditwheel repair dist/z2r_accel-0.0.2-cp312-cp312-linux_x86_64.whl -w dist/
# the file name for you is probably different than this. It should follow the convention of:
#                      z2r_accel-{VER}-{PYTHON}-{ABI}-{PLATFORM}.whl

```
If you don't get any errors, thi will produce a file that looks roughtly like `z2r_accel-0.0.2-cp312-cp312-manylinux_2_39_x86_64`. You can now safely upload the files to PyPI:

```
twine upload dist/*manylinux*.whl dist/*.tar.gz 
```


## On MacOS
To be determined.

## On Windows...?
To be determined.

## Unstable install
```
pip install -i https://test.pypi.org/simple/ z2r-accel
```