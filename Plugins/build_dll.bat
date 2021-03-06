@echo off
@rem -- follow urls are sample to build on vscode with batch file.
@rem https://blogs.msdn.microsoft.com/vcblog/2016/10/24/building-your-c-application-with-visual-studio-code/
@rem http://gabekore.org/vscode-c-windows

set "VSCMD_START_DIR=%CD%"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat") ^
else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" (call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat")
set compilerflags=/Ox /EHsc /c
cl.exe %compilerflags% VRIK_SolverDll.cpp json11/json11.cpp iksolver/IK_QJacobian.cpp iksolver/IK_QSegment.cpp iksolver/IK_QTask.cpp

set compilerflags=/DLL
set linkerflags=/OUT:VRIKSolver.dll
link.exe %compilerflags% VRIK_SolverDll.obj json11.obj IK_QJacobian.obj IK_QSegment.obj IK_QTask.obj %linkerflags%