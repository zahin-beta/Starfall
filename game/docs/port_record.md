# Starfall Host Port Selection Record

## Summary

Hosting previously always tried to bind the fixed port `45801`. If that port was already in use (another Starfall instance, or any other process), `NetPeer::Host` failed, the user was dumped silently back to the main menu, and re-entering the host flow prefilled `45801` again. This made it look like the port was "stuck" at 45801 and broke LAN sessions with more than one host.

Hosting now binds a **random free UDP port** on every session. A host no longer needs to pick or remember a port; the port is chosen automatically and shown in the lobby so other players on the LAN can join it (the discovery beacon already advertises the bound port).

## Constants (`game/include/Protocol.h`)

| Constant | Value | Meaning |
|---|---|---|
| `kDiscoveryPort` | 45800 | LAN discovery beacon port (unchanged) |
| `kDefaultGamePort` | 45801 | Legacy default; only used as the manual-connect prefill |
| `kGamePortMin` | 49152 | Start of the random search range (Windows default dynamic-port start) |
| `kGamePortMax` | 65000 | End of the random search range |
| `kPortBindAttempts` | 32 | How many random candidates to try before giving up |

## How port selection works now (`NetPeer::Host`)

`NetPeer::Host(uint16_t port)` now takes `0` to mean "auto-pick a random free port".

- `port == 0`: pick a random candidate in `[kGamePortMin, kGamePortMax]` (seeded from `std::random_device`, so every process gets a different sequence), try to bind, and retry on the next random candidate if it fails. Up to `kPortBindAttempts` (32) tries.
- `port != 0`: try to bind exactly that port, once (kept for a future manual-override path).
- The actually-bound port is stored and exposed via `NetPeer::GetBoundPort()`.
- `boundPort` is reset to `0` in `NetPeer::Close()` so a stale value is never reported after teardown.

## Why `SO_REUSEADDR` was replaced with `SO_EXCLUSIVEADDRUSE`

The host socket previously set `SO_REUSEADDR`. On Windows, two sockets that both set `SO_REUSEADDR` can bind the *same* UDP port; delivery becomes indeterminate (packets may go to either socket). That silently hides port collisions, which is exactly the failure mode that matters when multiple hosts run on one LAN.

The host socket now sets `SO_EXCLUSIVEADDRUSE` instead, so a taken port fails the bind loudly and the random scan moves on to the next candidate. This is safe for UDP rehosting: UDP has no `TIME_WAIT`, so rebinding the same port immediately after `Close()` still succeeds.

## Game flow changes (`Game.cpp`)

- Pressing `2` (Host) then confirming a ship now calls `StartHosting()` directly with the default `port = 0`, so a free port is picked automatically. The user is no longer sent through the port-entry screen.
- `StartHosting` records the bound port in `portText` and passes `net.GetBoundPort()` to `discovery.StartHost(...)` so the LAN beacon advertises the port that was actually bound.
- On bind failure, the user is shown the `Disconnected` screen with a visible error message instead of being silently returned to the menu.

## What did not change

- LAN discovery (`Discovery.cpp`) is untouched. The beacon still carries the bound `gamePort`, so clients join whatever port the host actually got.
- The wire protocol (`PacketHeader`, `BeaconPacket`, handshake, snapshots) is unchanged; `kVersion` is still `6`.
- Manual IP+port connect (`M` in the client lobby) still works; its default prefill is now derived from `kDefaultGamePort` instead of a hardcoded literal.

## Why random instead of sequential (45801, 45802, ...)

A sequential scan from a fixed start would make every host on a LAN drift upward in lock-step (first host gets 45801, second 45802, and so on), which is predictable and clusters hosts together. Random selection over the ~16k-port ephemeral range spreads independent hosts apart, so many players can host on the same network without stepping on each other. Collision is still possible but is retried against a fresh random candidate each attempt.

## Verification

- `build.bat` compiles and links cleanly.
- Host a match: the lobby now shows your `ip:<random-port>` with a port in `[49152, 65000]`.
- Occupy the port from a separate process, then host again: a *different* random port is chosen and shown.
- Two instances host simultaneously: each gets a distinct port; both appear in a third instance's discovery list and can be joined.
