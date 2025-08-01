@echo off
if "%~1"=="" (
    echo Usage: push.bat "Your commit message"
    echo Example: push.bat "Added window painting functionality"
    exit /b 1
)

echo Adding all changes to git...
git add .

echo Committing with message: "%*"
git commit -m "%*"

echo Pushing to GitHub...
git push

echo Done!
