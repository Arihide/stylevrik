@echo off
@rem -- follow urls are sample to build on vscode with batch file.
@rem https://blogs.msdn.microsoft.com/vcblog/2016/10/24/building-your-c-application-with-visual-studio-code/
@rem http://gabekore.org/vscode-c-windows

set "VSCMD_START_DIR=%CD%"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat" (call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat") ^
else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat" (call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat")
set compilerflags=/Zi /EHsc /DEBUG
set linkerflags=/OUT:VRIK_SolverTest.exe
cl.exe %compilerflags% VRIK_SolverTest.cpp json11/json11.cpp iksolver/IK_QJacobian.cpp iksolver/IK_QSegment.cpp iksolver/IK_QTask.cpp /link %linkerflags%
