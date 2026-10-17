# Starfall Project Plan

## Goal

Create a playable C++17 2D space-survival shooter using raylib. Complete a reliable core loop before adding enemy ships, power-ups, audio, or visual effects.

## MVP

1. Start from a menu and control the ship with WASD or arrow keys.
2. Fire bullets with Space; bullets travel upward and are removed off-screen.
3. Spawn asteroids over time; asteroids drift downward and are removed when they leave the play area.
4. Resolve bullet and asteroid collisions once, award 100 points per asteroid, and damage the player on asteroid contact.
5. Show score and three hull points; end the run at zero health and allow Enter to restart.
6. Use delta time for movement and timers, and keep transient objects from accumulating.

## Modules

Game owns state, entities, spawning, collision resolution, scoring, and the loop. Player, Bullet, and Asteroid own their motion and drawing. Collision provides circle overlap checks. UI renders menu, HUD, and game-over screens. Enemy and AudioManager are compile-ready extension points, not active in the MVP.

## Deferred

Enemy behavior, sound and music, power-ups, saved high score, pause, particles, and sprite assets are postponed until the MVP is stable.

## Build and delivery

Use build.bat with the sibling 64-bit MinGW raylib distribution and MSYS2 UCRT64 compiler. Keep project source and documentation under game; the sibling raylib distribution is a dependency.
