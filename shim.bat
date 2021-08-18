@echo off

set EXE=%~1
set WD=%~2

if "%WD%" == "" goto NoWD

:HasWD
set EXE=%EXE:\=\\%
set WD=%WD:\=\\%
cl /O2 /Foshim.obj /Feshim.exe /DEXE="L\"%EXE%\"" /DWD="L\"%WD%\"" shim.cpp
goto End

:NoWD
set EXE=%EXE:\=\\%
cl /O2 /Foshim.obj /Feshim.exe /DEXE="L\"%EXE%\"" shim.cpp
goto End

:End
del shim.obj