@echo off

set start_time=%TIME%

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if not exist ..\build mkdir ..\build
pushd ..\build
cl -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Zi ..\handmadehero\handmadehero.cpp user32.lib gdi32.lib 
popd

echo Build started at %start_time%
echo Build finished at %TIME%
