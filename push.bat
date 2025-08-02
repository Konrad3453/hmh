@echo off
if "%~1"=="" (

    exit /b 1
)

echo Adding all changes to git...
git add .

echo Committing with message: "%*"
git commit -m "%*"

echo Pushing to GitHub...
git push

echo Done!
