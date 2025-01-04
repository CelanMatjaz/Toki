@echo off
setlocal enabledelayedexpansion

REM Function to format files in a specified directory
:format
REM Ensure the directory argument is not empty
if "%~1"=="" (
    echo Error: No directory provided.
    exit /b 1
)

REM %1 - Relative or absolute directory
REM Resolve the relative directory to an absolute path
for %%d in ("%~1") do set "targetDir=%%~fd"
set "extensions=.cpp .h"

REM Check if the directory exists
if not exist "%targetDir%" (
    echo Error: Directory "%targetDir%" does not exist.
    exit /b 1
)

for /r "%targetDir%" %%f in (*.*) do (
    for %%e in (%extensions%) do (
        if /i "%%~xf"=="%%e" (
            echo Formatting: %%f
            clang-format -i "%%f"
        )
    )
)

exit /b 0

REM Example: Call the function with a relative directory
call :format ".\toki_sandbox"
