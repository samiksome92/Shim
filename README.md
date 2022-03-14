# Shim

A program to create a shim to an executable. Optionally also accepts arguments for setting the working directory and output file name. Utilises NTFS alternate streams to store data regarding the shim. Primary use is creating shims for programs so that the shims can be kept in a central directory added to `%PATH%` thus avoiding cluttering `%PATH%` with multiple entries.

## Compilation
Compile with `cl` as follows.

    cl /O2 /FoShim.obj /FeShim.exe Shim.cpp

## Usage
    Shim /E<path> [/D<dir>] [/O<out>]

    /E<path>    Path to executable
    /D<dir>     Working directory
    /O<out>     Output path

If `/D` if not specified, the working directory will be wherever the shim is called. If `/O` is not specified the application tries to extract the executable name from path.