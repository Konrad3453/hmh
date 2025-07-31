@echo off
setlocal enabledelayedexpansion

REM Check if commit message is provided
if "%~1"=="" (
    echo.
    echo Usage: git-push.bat "Your commit message"
    echo Example: git-push.bat "Added window painting functionality"
    echo.
    echo Optional flags:
    echo   -s : Show status before committing
    echo   -f : Force push (use with caution)
    echo.
    exit /b 1
)

set COMMIT_MSG=%1
set SHOW_STATUS=0
set FORCE_PUSH=0

REM Parse additional arguments
:parse_args
if "%~2"=="-s" (
    set SHOW_STATUS=1
    shift
    goto parse_args
)
if "%~2"=="-f" (
    set FORCE_PUSH=1
    shift
    goto parse_args
)
if not "%~2"=="" (
    shift
    goto parse_args
)

REM Show status if requested
if %SHOW_STATUS%==1 (
    echo Current git status:
    git status
    echo.
    pause
)

echo Adding all changes to git...
git add .

REM Check if there are changes to commit
git diff --cached --quiet
if %errorlevel%==0 (
    echo No changes to commit.
    exit /b 0
)

echo Committing with message: %COMMIT_MSG%
git commit -m %COMMIT_MSG%

if %errorlevel% neq 0 (
    echo Error: Commit failed!
    exit /b 1
)

echo Pushing to GitHub...
if %FORCE_PUSH%==1 (
    git push --force
) else (
    git push
)

if %errorlevel%==0 (
    echo ✓ Successfully pushed to GitHub!
) else (
    echo ✗ Push failed! Check your internet connection and repository permissions.
    exit /b 1
)
