@echo off

set start_time=%TIME%

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

set CommonCompilerFlags=-MT -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Zi
set CommonLinkerFlags=-opt:ref user32.lib gdi32.lib winmm.lib


if not exist ..\build mkdir ..\build
pushd ..\build
:: 32bit build
:: cl  %CommonCompilerFlags% ..\handmadehero\handmadehero.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

:: 64bit build
cl %CommonCompilerFlags% ..\handmadehero\win32_handmade.cpp /link %CommonLinkerFlags%
popd

echo Build started at %start_time%
echo Build finished at %TIME%
..\build\handmadehero.exe

:: -WX warnings as errors
:: -W4 warning lvl 4
:: -wd[warning number] ignore warning
::    4100 unused param
::    4201 nameless struct/union
::    4189 unused variable / compiler optimizes that
:: -MT multithread
:: -Gm- minimal rebuild off (recompailing only new code)
::  
