# Starfall

Starfall is a C++17 2D space shooter built with raylib. It ships a single-player **Survival** mode plus a LAN **1v1 Versus** mode over UDP (host/join with LAN auto-discovery).

## Controls

- Rotate: A / D
- Thrust: W
- Reverse / brake: S
- Fire: Space

## Ships

Pick a ship before every match (Survival and Versus). All ships share the same hitbox size; they differ in stats and each has one unique passive.

| Ship | Hull | Top speed | Turn | Fire int. | Bullet spd | Passive |
| --- | --- | --- | --- | --- | --- | --- |
| Interceptor | 3 | 360 | 220 | 0.22s | 720 | Veteran Frame — shorter invulnerability after a hit |
| Striker | 2 | 480 | 200 | 0.20s | 800 | Afterburner — reverse thrust hits full power |
| Bulwark | 5 | 250 | 150 | 0.42s | 620 | Reinforced Hull — takes half knockback |
| Viper | 3 | 375 | 340 | 0.15s | 640 | Hot Thrusters — +20% top speed while thrusting |
| Lancer | 3 | 330 | 165 | 0.55s | 1050 | Piercing Rounds — shots pierce one asteroid |
| Wraith | 4 | 400 | 235 | 0.30s | 700 | Light Frame — regains 1 hull after 8s without damage |

## Modes

Press a number on the main menu:

1. **Survival** — endless asteroid waves; score points, three hull points, difficulty ramps over time.
2. **Host LAN Match** — pick a game port, then wait for an opponent. The host runs the authoritative simulation.
3. **Join LAN Match** — auto-discovers hosts on your LAN; press `M` to enter an IP and port manually.

## Networking

- **Transport:** UDP with a small hand-rolled reliability layer — a reliable-ordered channel for control messages, unreliable latest-wins snapshots, and redundant input packets for inputs.
- **Architecture:** host-authoritative state sync. The client predicts its own ship and reconciles it against host snapshots, and renders the opponent interpolated.
- **Rules:** best-of-3 rounds, per-ship hull values, free-aim rotate/thrust movement, ships bounce on contact, asteroids act as shared hazards (reduced spawn rate versus Survival). The chosen ship is exchanged during the handshake so both ends render the right silhouette.
- **Ports:** discovery beacon on UDP 45800, game traffic on a configurable port (default 45801). Allow the game through Windows Firewall when prompted.

### Testing on a single PC

Run two instances of `build\Starfall.exe`. On the first, choose **Host**; on the second choose **Join**, press `M`, and connect to `127.0.0.1` with the host's port.

## Build on Windows

1. Install MSYS2 UCRT64 and make sure `C:\msys64\ucrt64\bin` is on `PATH`.
2. Place a 64-bit MinGW raylib distribution in the sibling `raylib` folder (`include\raylib.h` and `lib\libraylib.a`).
3. Run `build.bat` from this folder.
4. Launch `build\Starfall.exe`.

## Project layout

- `src/`, `include/` — game implementation and interfaces
- `assets/` — optional textures/audio/fonts; procedural drawing and audio are used when assets are absent
- `docs/` — project plan and manual test checklist

## Notes

Audio (sound effects and looping music) is synthesized at runtime; no external art or sound files are required. Networking uses Winsock2 (`-lws2_32`) with no third-party libraries.
