@echo off
setlocal
cd /d "%~dp0"
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "DS_VS=%%i"
if not defined DS_VS exit /b 1
call "%DS_VS%\VC\Auxiliary\Build\vcvars32.bat"
if errorlevel 1 exit /b 1
if not exist build mkdir build
cl /nologo /W4 /WX /O2 /MT /LD /EHsc version.cpp /Fo:build\ds_resolution.obj /link /OUT:build\ds_resolution.dll /IMPLIB:build\ds_resolution.lib /MACHINE:X86 /DYNAMICBASE
exit /b %errorlevel%
