@echo off

set start_time=%TIME%

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if not exist ..\build mkdir ..\build
pushd ..\build
cl -MT -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Zi ..\handmadehero\handmadehero.cpp /link -opt:ref -subsystem:windows user32.lib gdi32.lib 
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
