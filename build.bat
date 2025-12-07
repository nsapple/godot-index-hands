@echo off
echo ====================================
echo Building SteamVR Fingers Extension
echo ====================================
echo.
echo Using %NUMBER_OF_PROCESSORS% CPU cores for compilation
echo.

echo Step 1: Building godot-cpp (Debug)...
cd godot-cpp
call scons platform=windows target=template_debug -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to build godot-cpp debug
    cd ..
    pause
    exit /b 1
)

echo.
echo Step 2: Building godot-cpp (Release)...
call scons platform=windows target=template_release -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to build godot-cpp release
    cd ..
    pause
    exit /b 1
)
cd ..

echo.
echo Step 3: Building extension (Debug)...
call scons platform=windows target=template_debug -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to build extension debug
    pause
    exit /b 1
)

echo.
echo Step 4: Building extension (Release)...
call scons platform=windows target=template_release -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to build extension release
    pause
    exit /b 1
)

echo.
echo ====================================
echo Build Complete!
echo ====================================
echo.
echo Built files are in: demo/addons/steamvr_fingers/bin/
echo.
echo You can now open the demo project in Godot 4.3
echo.
pause
