@echo off
set "PATH=C:\msys64\ucrt64\bin;%PATH%"

echo ------------------------------------------
echo BUILDING MY SUMMER BABETTA (ALPHA)
echo ------------------------------------------

echo Compiling Launcher...
g++ code/main.cpp -o build/Launcher.exe -mwindows -lcomctl32

echo Compiling Game...
echo  !! This is builded by LOM_Noob and Jurmat, Do not Republish this code gain in github. (Github: https://github.com/NoobRobloxdev/ )
g++ code/game.cpp -o build/Game.exe -lraylib -lopengl32 -lgdi32 -lwinmm

echo ------------------------------------------
if %errorlevel% equ 0 (
    echo BUILD SUCCESSFUL!
    echo Launching the Launcher now...
    start build/Launcher.exe
) else (
    echo !! BUILD FAILED !! Check the errors above.
    echo !! This is builded by LOM_Noob and Jurmat, Do not Republish this code gain in github. (Github: https://github.com/NoobRobloxdev/ )
    pause
)