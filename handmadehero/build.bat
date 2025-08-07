@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if not exist ..\build mkdir ..\build
pushd ..\build
cl  -FC -Zi -Od -MDd ..\handmadehero\handmadehero.cpp user32.lib gdi32.lib winmm.lib
popd