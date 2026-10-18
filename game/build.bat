@echo off
setlocal
set "GAME_DIR=%~dp0"
for %%I in ("%GAME_DIR%..") do set "STARFALL_ROOT=%%~fI"
set "RAYLIB_DIR=%STARFALL_ROOT%\raylib"
set "OUT_DIR=%GAME_DIR%build"
where g++ >nul 2>nul
if errorlevel 1 (
    echo Error: g++ was not found. Install MSYS2 UCRT64 and add its bin folder to PATH.
    exit /b 1
)
for /f "delims=" %%T in ('g++ -dumpmachine') do set "TARGET=%%T"
echo Using compiler target: %TARGET%
echo %TARGET% | findstr /i "x86_64" >nul
if errorlevel 1 (
    echo Error: This build expects a 64-bit MinGW compiler.
    exit /b 1
)
if not exist "%RAYLIB_DIR%\include\raylib.h" (
    echo Error: raylib headers were not found at "%RAYLIB_DIR%\include".
    exit /b 1
)
if not exist "%RAYLIB_DIR%\lib\libraylib.a" (
    echo Error: libraylib.a was not found at "%RAYLIB_DIR%\lib".
    exit /b 1
)
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
g++ -std=c++17 -O2 -Wall -Wextra -I"%GAME_DIR%include" -I"%RAYLIB_DIR%\include" -L"%RAYLIB_DIR%\lib" ^
    "%GAME_DIR%src\main.cpp" "%GAME_DIR%src\Game.cpp" "%GAME_DIR%src\Player.cpp" ^
    "%GAME_DIR%src\Bullet.cpp" "%GAME_DIR%src\Asteroid.cpp" "%GAME_DIR%src\Enemy.cpp" ^
    "%GAME_DIR%src\Collision.cpp" "%GAME_DIR%src\UI.cpp" "%GAME_DIR%src\AudioManager.cpp" ^
    "%GAME_DIR%src\NetPeer.cpp" "%GAME_DIR%src\Discovery.cpp" ^
    "%GAME_DIR%src\ShipCatalog.cpp" "%GAME_DIR%src\ShipRenderer.cpp" ^
    -static -static-libgcc -static-libstdc++ -o "%OUT_DIR%\Starfall.exe" -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
if exist "%RAYLIB_DIR%\lib\raylib.dll" copy /Y "%RAYLIB_DIR%\lib\raylib.dll" "%OUT_DIR%\raylib.dll" >nul
echo Build succeeded: "%OUT_DIR%\Starfall.exe"
endlocal



