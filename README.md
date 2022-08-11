# Shim

A program to create a shim to an executable. Optionally also accepts arguments for setting the working directory and output file name. Utilises NTFS alternate streams to store data regarding the shim. Primary use is creating shims for programs so that the shims can be kept in a central directory added to `%PATH%` thus avoiding cluttering `%PATH%` with multiple entries.

## Compilation
Compile with `cl` as follows.

    cl /O2 /FoShim.obj /FeShim.exe Shim.cpp

## Usage
    Shim.exe [PATH] [OPTIONS...]

    Options:
      --dir, -d <dir>      Working directory
      --out, -o <out>      Output path

If `--dir` if not specified, the working directory will be wherever the shim is called. If `--out` is not specified the application tries to extract the executable name from path.