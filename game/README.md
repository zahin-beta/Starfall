# Starfall

Starfall is a C++17 2D space shooter built with raylib. It ships a single-player **Survival** mode plus a LAN **1v1 Versus** mode over UDP (host/join with LAN auto-discovery).

**Author:** Zahin Bin Hasan

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

---

## Game engine

There is no third-party engine — the game is a single raylib window driven by a hand-rolled loop in `Game`.

### Main loop

- `Game::Run()` opens a 960x720 window and runs a **fixed 60 Hz simulation step** (`net::kFixedStep`) with an accumulator, decoupled from rendering. This keeps host simulation and client prediction deterministic between snapshots.
- Each tick: `Update()` runs the current state handler, then `Draw()` renders. Network receive runs on a background thread and is drained (`PumpReliable` / `PumpUnreliable`) on the game thread — no locks held during gameplay logic.
- `Game::RunBenchmark()` (`--bench`) runs 600 scripted frames and prints avg/p95/max frame times.

### State machine

`Game::State` drives everything through a single `Update`/`Draw` dispatch:

`Menu → ShipSelect → (PortEntry → HostLobby) | (ClientLobby → Connecting) → Playing ⇄ Paused → GameOver / Disconnected → Menu`

Extra states: `ManualEntry` (type IP:port), `Forfeiting` (gives the opponent a few seconds when you quit mid-match).

### Modules

| Module | Files | Responsibility |
| --- | --- | --- |
| `Game` | `Game.h/.cpp` | State machine, modes, fixed-step loop, all networked gameplay glue |
| `Player` | `Player.h/.cpp` | Ship physics (thrust, drag, rotation, knockback, invulnerability, per-ship regen), input → velocity |
| `ShipCatalog` / `ShipRenderer` | `ShipCatalog.*`, `ShipRenderer.*` | Data-driven ship definitions and procedural silhouette drawing |
| `Bullet` / `Asteroid` / `Enemy` | `Bullet.*`, `Asteroid.*`, `Enemy.*` | Projectiles with fading trails and pierce, procedural rock variants (7–9 sides, craters, 3 palettes) |
| `Background` | `Background.*` | 3-layer parallax starfield, nebula render texture, planets — all seeded and wrapped |
| `ParticleSystem` | `ParticleSystem.*` | Bounded 640-particle pool: thruster flames, explosion debris, hit sparks, celebration |
| `GlowLayer` | `GlowLayer.*` | Half-res bloom buffer with `assets/shaders/glow_blur.fs`, additive fallback if shader missing |
| `CameraEffects` | `CameraEffects.*` | Restrained screen shake (damage/death triggers), applied around the world via `BeginMode2D` |
| `Collision` | `Collision.*` | Circle-vs-circle overlap tests (all hitboxes are circles) |
| `UI` | `UI.*` | Menus, HUD, pips, banners, lobby lists, pause/victory overlays |
| `AudioManager` | `AudioManager.*` | Runtime-synthesized SFX (shoot/explosion/hit/start/game-over) and a procedural music loop |
| `NetPeer` / `Discovery` | `NetPeer.*`, `Discovery.*`, `Protocol.h` | UDP transport, reliability, LAN host discovery |

### Survival flow

`StartSurvival()` resets score/hull and spawns waves on a timer (`spawnTimer`, difficulty ramps with `elapsedTime`). Each tick: move players, step bullets/asteroids, resolve collisions (`ResolveVersusCollisions` equivalent for solo in `UpdateSurvival`), score +100 per destroyed asteroid, emit particles/trails, shake camera on damage, and draw through the glow layer.

---

## Ship functions

Each ship is a `ShipDef` in `ShipCatalog.cpp` — plain data, no per-ship code branches. `Player` interprets the fields:

- **Movement** — `thrust`, `maxSpeed`, `turnSpeed`, `drag`, `reverseMultiplier` feed `Player::Update()`.
- **Combat** — `fireInterval` gates `Player::TryShoot()`, `bulletSpeed`/`bulletRadius` shape the projectile, `pierce` is copied onto each `Bullet` (decremented per hit until it passes through).
- **Survivability** — `health` sets hull; `invulnMultiplier` scales the post-hit invulnerability window; `knockbackTaken` scales impulse in `ApplyKnockback()`.
- **Passives** (implemented as the zero/one-off fields above, not special cases):
  - *Veteran Frame* → `invulnMultiplier < 1`
  - *Afterburner* → `reverseMultiplier = 1.0` (others 0.6)
  - *Reinforced Hull* → `knockbackTaken = 0.5`
  - *Hot Thrusters* → `speedBonusWhileThrusting = 0.20`, applied while `throttle > 0`
  - *Piercing Rounds* → `pierce = 1`
  - *Light Frame* → `regenDelay = 8.0` seconds, handled by `Player::UpdateRegen()`
- **Presentation** — `hullColor`, `accentColor`, and `shape` are consumed by `ShipRenderer` (silhouette, outline, cockpit, wingtip lights, accent-tinted exhaust).

To add a ship: append a `ShipDef`, a `ShipShape`, and a draw branch — the stats, HUD table, and sync all pick it up automatically.

---

## Networking basics

Everything lives in `Protocol.h` (wire format) and `NetPeer` (transport). Only raylib + Winsock2 are used.

### Transport

- **UDP sockets** with a small reliability layer. Two channels per packet:
  - **Reliable-ordered** (`PacketKind::Reliable`) — sequence numbers, ACK + 32-bit ACK bitfield piggybacked on every header, resend of unacked payloads, and a reorder buffer for delivery in order. Used for `Hello`, `Welcome`, `StartMatch`, `Disconnect`, `Pause`.
  - **Unreliable latest-wins** (`PacketKind::Unreliable`) — used for `Snapshot` and `Input`; stale data is worthless, so nothing is resent.
- Every packet carries `kMagic = 'STFL'`, `kVersion` (currently 6), and sequence/ack fields; foreign traffic is dropped.
- Packets are capped at `kMaxPacket` (1200 bytes); snapshots pack up to 40 bullets and 24 asteroids.
- A receiver thread loops on `recvfrom` and pushes payloads into thread-safe queues that the game thread drains. RTT is measured from reliable resends; the peer times out if nothing arrives.

### Simulation model

- **Host-authoritative.** The host runs the real simulation at the fixed 60 Hz step and broadcasts `SnapshotMsg` (tick, both players, bullets, asteroids, phase, wins) each tick.
- **Client** predicts its own ship locally and sends `InputMsg` — the last **8 inputs redundantly** (`kInputRedundancy`) so a single lost packet costs nothing. The snapshot's `ackedInputTick` tells the client what to drop from `inputHistory` and reconcile against.
- **Remote entities** are rendered interpolated/extrapolated between snapshots; the client derives visual-only effects (thruster particles, trails, hit sparks) locally — **no effect data is ever sent over the network**, keeping packets small and the protocol gameplay-only.
- Buttons are packed into 5 bits (`InputBits`) — rotate L/R, thrust, reverse, fire — one byte per input tick.

### Match flow

1. Client sends `HelloMsg` (name + chosen ship).
2. Host replies `WelcomeMsg` (assigned player id, rounds-to-win, both ship ids — so both screens render correct silhouettes).
3. Host sends `StartMatch` → both enter `Playing`.
4. Per tick: client → `InputMsg`, host → `SnapshotMsg`.
5. Round end / match end is carried in the snapshot (`phase`, `wins[2]`); `Pause` messages sync the pause menu.

---

## LAN connections

### Discovery

- While hosting, `Discovery` broadcasts a **beacon** every second to UDP port **45800** (`kDiscoveryPort`) — a small `BeaconPacket` with host name, game port, and player count.
- While in the join menu, `Discovery` listens on 45800 and collects hosts (`HostInfo`: name, IP, port, last-seen). Entries expire if beacons stop; the list is re-sorted each update.
- Game traffic uses a separate **configurable port** — default **45801**, or a random free port in **49152–65000** when hosting with port 0 (the OS picks; `NetPeer` tries up to 32 binds). The chosen port is shown in the lobby for manual connects.

### Connecting

1. **Auto:** highlight a discovered host in the join list and press Enter.
2. **Manual:** press `M`, type `IP:port` (`ManualEntry`), press Enter — works across subnets where broadcast discovery can't reach.
3. `StartJoin()` → `NetPeer::Join()` sends the handshake `Hello` (reliable, retried) and moves to `Connecting` until `Welcome` arrives or the timeout expires.
4. The client keeps a **disconnected → menu** path for any loss: `CONNECTION LOST` is shown and Enter returns to the menu; the host notices via timeout and forces a forfeit win.
5. Hosting rebinds cleanly after a match (`Close()` releases the socket and `SO_EXCLUSIVEADDRUSE` before the next `Host()`).

### Single-PC testing

Run two instances of `build\Starfall.exe`. On the first, choose **Host**; on the second choose **Join**, press `M`, and connect to `127.0.0.1` with the host's port.

---

## Build on Windows

1. Install MSYS2 UCRT64 and make sure `C:\msys64\ucrt64\bin` is on `PATH`.
2. Place a 64-bit MinGW raylib distribution in the sibling `raylib` folder (`include\raylib.h` and `lib\libraylib.a`).
3. Run `build.bat` from this folder.
4. Launch `build\Starfall.exe`.

## Testing

- **Automated:** `Starfall.exe --bench` runs 600 frames of scripted Survival gameplay and prints frame-time stats (avg / p95 / max). The game also prints `STATE: <name>` to stdout on every state transition so drivers can follow progress from a redirected log.
- **Manual:** [`docs/test_cases.md`](docs/test_cases.md) lists the full manual checklist (visual and audio checks).

## Project layout

- `src/`, `include/` — game implementation and interfaces
- `assets/` — optional textures/audio/fonts; procedural drawing and audio are used when assets are absent (except `assets/shaders/glow_blur.fs`, which has an additive-draw fallback)
- `docs/` — project plan and manual test checklist

## Notes

Audio (sound effects and looping music) is synthesized at runtime; no external art or sound files are required. Networking uses Winsock2 (`-lws2_32`) with no third-party libraries.
