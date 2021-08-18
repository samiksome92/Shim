# Shim

A C++ program which accepts two specific definitions `EXE` and `WD` and creates a shim executable which executes `EXE` with `WD` as working directory and passes all arguments to `EXE` as is. Primary use is creating shims for programs so that the shims can be kept in a central directory added to `%PATH%` thus avoiding cluttering `%PATH%` with multiple entries.

## Usage
To create a shim, simply compile shim.cpp with necessary definitions. For example, creating a shim for `C:\Program Files\7-Zip\7z.exe` compile as:

    cl /O2 /Foshim.obj /Feshim.exe /DEXE="L\"C:\\Program Files\\7-Zip\\7z.exe\"" shim.cpp

Note the usage of `L`, `\"` and `\\`. These are required so that the UTF-16 string is defined properly. If a specific working directory is needed it can be specified by deifning `WD` in a similar fashion.

A helper batch file is provided which takes care of setting these details. To use the batch file, simply run it as:

    shim.bat <EXE> <WD>

Where, `<EXE>` is the path to the executable, and `<WD>` is the path to the working directory (which can be skipped if not needed). Thus the same example using the batch file would be

    shim.bat "C:\Program Files\7-Zip\7z.exe"

<b>NOTE</b>: In either case the compilation environment must be set properly, so that `cl.exe` and standard Windows libraries are in `%PATH%`.