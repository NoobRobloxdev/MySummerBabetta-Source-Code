@echo off
set PATH=%PATH%;C:\msys64\ucrt64\bin;C:\msys64\mingw64\bin;C:\mingw64\bin;C:\MinGW\bin

echo REPAIRING "MY SUMMER BABETTA (ALPHA) BUILDING TOOL"
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

echo Compiling Game (Game.cpp)...
g++ code/Game.cpp -o build/Game.exe -std=c++17 -I include -L lib -lraylib -lopengl32 -lgdi32 -lwinmm

echo Compiling Was (Was.cpp)...
g++ code/babettasim.cpp -o build/babettasim.exe --std=c++17 -I include -L lib -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32

if %ERRORLEVEL% NEQ 0 (
    echo [!] Game failed.
    pause
    exit /b
)

echo ---------------------------------------
echo !! BUILD SUCCESSFUL !!
echo Run build/Launcher.exe to start.
echo ---------------------------------------
pause