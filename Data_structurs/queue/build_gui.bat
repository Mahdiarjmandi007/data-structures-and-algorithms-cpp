@echo off
setlocal

rem Path to the raylib build for this MinGW (UCRT) toolchain.
if not defined RAYLIB_HOME set "RAYLIB_HOME=D:\toll\raylib-6.2-ucrt"

set "GXX=D:\toll\mingw64\bin\g++.exe"
if not exist "%GXX%" set "GXX=g++"

if not exist "%RAYLIB_HOME%\include\raylib.h" (
    echo [error] raylib.h was not found in "%RAYLIB_HOME%\include".
    echo         Set RAYLIB_HOME to the raylib installation folder.
    exit /b 1
)

"%GXX%" -std=c++17 -O2 -Wall -Wextra -mwindows "%~dp0gui.cpp" -o "%~dp0gui.exe" ^
    -I"%RAYLIB_HOME%\include" -L"%RAYLIB_HOME%\lib" ^
    -lraylib -lopengl32 -lgdi32 -lwinmm ^
    -static -static-libgcc -static-libstdc++

if errorlevel 1 (
    echo.
    echo [error] Build failed.
    exit /b 1
)

echo.
echo [ok] Built: %~dp0gui.exe
