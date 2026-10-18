@echo off
echo =========================================================
echo  Compiling Smart Parking Visualization System (C/OpenGL)
echo =========================================================

set PATH=C:\msys64\ucrt64\bin;%PATH%

:: Terminate any running instance of smart_parking.exe to prevent file locking errors
taskkill /f /im smart_parking.exe >nul 2>&1

gcc -std=c99 -Wall -O2 main.c parking.c graphics.c navigation.c vehicle.c camera.c -o smart_parking.exe -lfreeglut -lglu32 -lopengl32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful! Created smart_parking.exe
    echo Starting application...
    echo.
    smart_parking.exe
) else (
    echo.
    echo Compilation failed. Please check for errors above.
    pause
)
