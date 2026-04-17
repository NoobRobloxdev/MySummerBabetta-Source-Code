@echo off

echo ---------------------------------------
echo Setting MSYS64 Path
set PATH=%PATH%;C:\msys64\ucrt64\bin;C:\msys64\mingw64\bin;C:\mingw64\bin;C:\MinGW\bin
echo Done Setting MSYS64 Path
echo ---------------------------------------

echo ---------------------------------------
echo MY SUMMER BABETTA BUILDING TOOL / BUILD TOOL VERSION: 0.1.3A
echo ---------------------------------------

echo ---------------------------------------
echo Building "MY SUMMER BABETTA (ALPHA)" 
echo ---------------------------------------
echo Killing the procceses for Launcher.exe and Game.exe
taskkill /F /IM Launcher.exe /T >nul 2>&1
taskkill /F /IM Game.exe /T >nul 2>&1
echo ---------------------------------------


if not exist "build" mkdir build
if not exist "build\mods" mkdir build\mods

copy /Y "code\*.dll" "build\"

echo ---------------------------------------
echo Coping Sounds/Textures to Build/Sounds Build/Textures
xcopy /E /I /Y "Sounds" "build\Sounds"
xcopy /E /I /Y "Textures" "build\Textures"
echo ---------------------------------------

echo Compiling Launcher (main.cpp)...
g++ code/main.cpp -o build/Launcher.exe -std=c++17 -I include -L lib -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32

if %ERRORLEVEL% NEQ 0 (
    echo [!] Launcher failed.
    pause
    exit /b
)

echo Compiling Was (Diffrent Game but it will be in the Launcher in Launcher Version 0.4.8A or idk) (Was.cpp)...
g++ code/babettasim.cpp -o build/babettasim.exe --std=c++17 -I include -L lib -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32

echo Compiling Game (Game.cpp)...
g++ code/Game.cpp -o build/Game.exe -std=c++17 -I include -L lib -lraylib -lopengl32 -lgdi32 -lwinmm

if %ERRORLEVEL% NEQ 0 (
    echo [!] Game failed.
    pause
    exit /b
)




echo ---------------------------------------
echo !! BUILD SUCCESSFUL !!
echo Better to run is the Launcher not the Game (Game.exe) directly or Babetta Simulator (babettasim.exe)
echo ---------------------------------------
echo Run build/Launcher.exe to start. or Run build/Game.exe also Run build/babettasim.exe for no reason if you want
echo ---------------------------------------
pause