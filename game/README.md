# Starfall

Starfall is a 2D space survival shooter built with C++17 and raylib. This MVP includes a menu, keyboard movement, continuous shooting, asteroid waves, collision damage, scoring, and restart. Enemy ships and audio are reserved for later work.

## Controls

- Move: WASD or arrow keys
- Shoot: hold Space
- Start or restart: Enter
- Exit: Escape

## Build on Windows

1. Install MSYS2 UCRT64 and make sure C:\msys64\ucrt64\bin is on PATH.
2. Place a 64-bit MinGW raylib distribution in the sibling raylib folder. Required files are include\raylib.h and lib\libraylib.a. The build statically links the MinGW C++ runtimes.
3. Run build.bat from this folder.
4. Launch build\Starfall.exe.

build.bat links the local sibling static library. Keep compiler and raylib architecture compatible.

## Project layout

- src/, include/: game implementation and interfaces
- assets/: optional textures, audio, and fonts; primitive drawings are used when assets are absent
- docs/: project plan and manual test checklist

## MVP notes

The game uses procedural shapes and a generated starfield; no external art or sound files are required. Asteroids award 100 points when destroyed and reduce hull on contact. Difficulty gradually increases through shorter spawn intervals and faster asteroids.

