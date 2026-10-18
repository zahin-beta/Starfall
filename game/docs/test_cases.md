# Starfall Test Cases

Run a fresh build with build.bat, then launch build\Starfall.exe. Check each item manually.

## Single player (Survival)

| Test | Steps | Expected result |
|---|---|---|
| Build and launch | Run build.bat, then open the executable. | The build succeeds and the menu appears. |
| Start survival | Press 1 on the menu. | A new run begins with score 0 and three hull markers. |
| Rotation | Hold A, then D. | The ship rotates left and right in place. |
| Thrust and reverse | Hold W, then S. | The ship accelerates forward along its facing, then decelerates/reverses. |
| Firing | Hold Space. | Bullets travel along the ship's facing direction. |
| Asteroid waves | Wait during a run. | Asteroids appear at the top and move through the play area. |
| Bullet collision | Shoot an asteroid. | The bullet and asteroid disappear, and score increases by 100 once. |
| Player damage | Let an asteroid hit the ship. | Hull decreases by one; brief invulnerability prevents repeated instant damage. |
| Game over and restart | Take three hits, then press Enter. | Game over shows, then a clean run restarts with full hull and zero score. |
| Exit | Press Escape. | The window closes cleanly. |

## Ships

| Test | Steps | Expected result |
|---|---|---|
| Ship select screen | Press 1 (or host/join) on the menu. | A ship-select screen lists all six ships with stats and passives. |
| Pick and browse | Use Left/Right (or A/D) to change ship, Enter to confirm. | The highlighted ship changes; confirming starts the match with that ship. |
| Distinct silhouettes | Cycle all six ships. | Each ship draws a distinct, fully filled hull with outline, cockpit, and wingtip lights (no missing/blank hulls). |
| Per-ship stats | Fly Striker, then Bulwark. | Striker is noticeably faster/turns differently; Bulwark is slow with five hull squares on the HUD. |
| Lancer pierce | Play Lancer and shoot a cluster of asteroids. | A single shot passes through and destroys one extra asteroid behind the first. |
| Wraith regen | Play Wraith, take a hit, then avoid damage for 8s. | One hull square is restored. |
| Invuln shield | Take a hit. | A pulsing shield ring appears around the ship instead of the ship blinking out. |

## Networking / Versus

| Test | Steps | Expected result |
|---|---|---|
| Host lobby | Press 2, type a port, press Enter. | The lobby shows your IP:port and a waiting message. |
| Discovery | On a second instance press 3. | The host appears in the join list. |
| Manual connect | On a second instance press 3, then M, enter 127.0.0.1 and the host port. | The client connects and the match starts on both screens. |
| State sync | Move and rotate on one machine. | Both screens show the same ship positions. |
| Ship sync | Pick different ships on host and client. | Each screen renders the correct silhouette for both ships. |
| Flame sync | Thrust on one machine. | The remote ship's engine flame animates on the other screen. |
| Versus firing | Shoot the opponent's ship. | Their hull drops on both screens; a hit sound plays. |
| Ship bounce | Fly both ships into each other. | The ships separate and bounce apart. |
| Asteroids in versus | Watch the field. | Asteroids spawn less often than Survival and damage either ship. |
| Round end | Reduce a ship to 0 hull. | "ROUND OVER" appears, then positions and asteroids reset. |
| Best of 3 | Win two rounds. | A "P1 WINS" or "P2 WINS" overlay appears; Enter returns to the menu. |
| Prediction feel | Move your own ship while connected. | Your ship responds instantly with no visible lag. |
| Disconnect | Close the host instance mid-match. | The client shows "CONNECTION LOST"; Enter returns to the menu. |
| Port reuse | Host again on the same port after a match. | Hosting succeeds again (SO_REUSEADDR). |

## Audio

| Test | Steps | Expected result |
|---|---|---|
| Audio cues | Shoot, explode an asteroid, take a hit, and end a match. | Distinct shoot, explosion, hit, and end sounds play. |
| Background music | Play through a full loop. | Ambient music plays and loops without an audible click. |
| No audio device | Launch without a working audio device. | The game runs normally with no sound and does not crash. |
