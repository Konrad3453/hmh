@echo off
if "%~1"=="" (
    echo Usage: push.bat "Your commit message"
    echo Example: push.bat "Added window painting functionality"
    exit /b 1
)

echo Adding all changes to git...
git add .

echo Committing with message: %1
git commit -m %1

echo Pushing to GitHub...
git push


REM Enable ANSI escape sequences (Windows 10 or later required)
for /f "tokens=2 delims=[]" %%i in ('ver') do set version=%%i
if not "%version%"=="Version 10.0" goto :done

>nul 2>&1 reg query "HKCU\Console" /v VirtualTerminalLevel
if errorlevel 1 (
    reg add "HKCU\Console" /v VirtualTerminalLevel /t REG_DWORD /d 1 /f >nul
)

:done
REM Output green text using ANSI escape sequence
echo [[32mDone![0m]