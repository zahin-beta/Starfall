# Starfall Project Plan

## Goal

Deliver a C++17 2D space shooter on raylib with two playable modes: a single-player Survival loop and a LAN 1v1 Versus mode over UDP with host/join, auto-discovery, client prediction, and host-authoritative state sync.

## Phases

1. **Core loop (done)** — menu, rotate/thrust ship, off-screen-trimmed bullets, drifting asteroids, collision resolution, scoring, hull, game over/restart.
2. **Audio (done)** — procedural sound effects and a looping music bed synthesized at runtime; no external assets required.
3. **Network foundation (done)** — UDP Winsock2 peer with a reliability layer: reliable-ordered control channel, unreliable latest-wins snapshots, and redundant input packets. LAN discovery beacons (UDP 45800) plus manual IP/port entry.
4. **Versus gameplay (done)** — two ships with free aim, bullet combat, ship-on-contact bounce, shared asteroid hazards at a reduced rate, best-of-3 rounds with round/match end states.
5. **Prediction and reconciliation (done)** — the client predicts its own ship and bullets and reconciles against host snapshots by replaying unacknowledged input; the opponent is rendered interpolated.
6. **Polish and verification (done)** — mode-specific HUDs, banners, disconnection handling, README and test checklist updates.
7. **Ship roster (done)** — six selectable ships (catalog of stats, colors, and one passive each) with bespoke silhouettes, chosen on a pre-match ship-select screen for both modes and exchanged during the Versus handshake.

## Architecture

- `Game` owns state, mode dispatch, fixed-step simulation, spawning, collisions, and scoring.
- `Player`, `Bullet`, `Asteroid` own their motion and drawing; shared `InputCommand` drives both live input and prediction replay.
- `NetPeer` wraps the UDP socket and the reliability layer; `Discovery` handles beacons and host lists.
- `Protocol` defines the wire format; `UI` renders every screen; `Audio` synthesizes SFX and music.
- `ShipCatalog` holds the ship stat/passive table; `ShipRenderer` draws each silhouette (hull, outline, flame, invuln shield). Note: `DrawTriangle` back-face-culls, so ship geometry is submitted with normalized winding.
- Roles: the host is Player 0 (bottom, aim 0°) and simulates authoritatively; the client is Player 1 (top, aim 180°).

## Deferred

Power-ups, pause, particles, sprite assets, and saved high score remain out of scope.

## Build and delivery

Use `build.bat` with the sibling 64-bit MinGW raylib distribution and the MSYS2 UCRT64 compiler. Winsock2 (`-lws2_32`) is the only platform dependency beyond raylib. Keep project source and documentation under `game/`.
