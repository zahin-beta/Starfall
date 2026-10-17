# Starfall Test Cases

Run a fresh build with build.bat, then launch build\Starfall.exe. Check each item manually.

| Test | Steps | Expected result |
|---|---|---|
| Build and launch | Run build.bat, then open the executable. | The build succeeds and the menu appears. |
| Menu start | Press Enter. | A new run begins with score 0 and three hull markers. |
| Movement | Use WASD and arrow keys separately in all directions. | The ship moves smoothly and diagonally without leaving the window. |
| Shooting | Hold Space during play. | Bullets appear above the ship and travel upward. |
| Asteroid waves | Wait during a run. | Asteroids appear at the top and move through the play area. |
| Bullet collision | Shoot an asteroid. | The bullet and asteroid disappear, and score increases by 100 once. |
| Player damage | Let an asteroid hit the ship. | Hull decreases by one; brief invulnerability prevents repeated instant damage. |
| Game over | Take three hits. | Gameplay ends and final score/restart prompt appears. |
| Restart | Press Enter on the game-over screen. | A clean run starts with full hull, zero score, and no old entities. |
| Exit | Press Escape. | The window closes cleanly. |
| Long run cleanup | Play long enough for bullets and asteroids to leave the screen. | Off-screen entities are removed and the game continues. |
