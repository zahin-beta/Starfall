# Starfall

A C++17 2D space shooter built with [raylib](https://www.raylib.com/) — single-player **Survival** mode plus a LAN **1v1 Versus** mode over UDP, with all art and audio generated procedurally at runtime.

**Author:** Zahin Bin Hasan

## Features

- **Survival** — endless asteroid waves, three hull points, difficulty ramps over time.
- **LAN Versus** — host/join with automatic UDP discovery (or manual IP/port entry), host-authoritative simulation with client-side prediction and interpolation, best-of-3 rounds.
- **Six ships** — distinct stats and a unique passive each (see [`game/README.md`](game/README.md) for the full table).
- **Procedural graphics** — parallax starfield with nebula and planets, distinct ship silhouettes, particle thrusters/explosions, bullet trails, screen shake, and half-res bloom glow. No texture assets required.
- **Procedural audio** — sound effects and looping music synthesized at runtime.

## Controls

| Action | Key |
|---|---|
| Rotate | A / D |
| Thrust | W |
| Reverse / brake | S |
| Fire | Space |
| Menu | Number keys, arrows, Enter, Escape |

## Build (Windows)

1. Install MSYS2 UCRT64 and make sure `C:\msys64\ucrt64\bin` is on `PATH`.
2. A 64-bit MinGW raylib distribution is expected in `raylib/` (`include/raylib.h`, `lib/libraylib.a`).
3. Run `game/build.bat`.
4. Launch `game/build/Starfall.exe`.

Requires only raylib and Winsock2 (`-lws2_32`) — no other third-party libraries.

## Repository layout

```
starfall/
├── game/
│   ├── build.bat        # build script (g++, -std=c++17 -Wall -Wextra)
│   ├── build/           # output: Starfall.exe
│   ├── src/, include/   # game implementation and interfaces
│   ├── assets/          # optional shaders/audio/textures (procedural fallbacks exist)
│   └── docs/            # project plan and manual test checklist
├── graphics.md          # graphics roadmap and progress tracker
└── raylib/              # vendored raylib headers + libraries
```

## Testing

- **Automated:** `Starfall.exe --bench` runs 600 frames of scripted gameplay and prints frame-time stats (avg / p95 / max). The game also prints `STATE: <name>` to stdout on every state transition so drivers can follow progress from a redirected log.
- **Manual:** [`game/docs/test_cases.md`](game/docs/test_cases.md) lists the full manual checklist (visual and audio checks).

Latest benchmark (Intel UHD Graphics laptop): **avg 2.19 ms / ~456 FPS**, p95 2.36 ms — well inside the 16.7 ms frame budget.

## Future development

- **Gameplay**
  - Power-ups and new Survival hazards (asteroid types, elite waves, boss events)
  - Score persistence — local high-score table / leaderboard
  - Additional Versus rules: time limit, sudden death, alternate win conditions
  - Gamepad support and remappable controls
  - Practice/training mode with configurable bots
- **Networking**
  - Matchmaking beyond LAN (rendezvous server or NAT traversal)
  - Lag compensation (hit rewinding) and client-side interpolation smoothing improvements
  - Spectator mode and replay recording (snapshots already exist — store and replay them)
  - Encryption/authentication for non-LAN play
- **Graphics**
  - Sprite/texture pipeline alongside the procedural drawing (atlas + shader-based post FX chain)
  - Full-screen post-processing: motion blur, chromatic aberration on damage, color grading
  - Particle quality scaling by measured GPU cost; occlusion for the parallax layers
  - Ship paint customization (colors, trails, decals)
- **Audio**
  - Positional (panned) effects for LAN opponents and off-screen events
  - Dynamic music layers that react to hull remaining / wave intensity
- **Tooling & project**
  - Cross-platform build (CMake) and Linux/macOS support
  - Automated CI: build + headless smoke tests using `--bench` and the `STATE:` log
  - In-game settings menu (resolution, vsync, volume) with a config file
  - Level/wave scripting data files instead of hardcoded spawn tables

## Documentation

- [`game/README.md`](game/README.md) — detailed game rules, ship stats, networking notes
- [`graphics.md`](graphics.md) — graphics roadmap with completed progress tracker
- [`game/docs/project_plan.md`](game/docs/project_plan.md) — project plan
