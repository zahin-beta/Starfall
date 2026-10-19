# Starfall — Graphics Upgrade Roadmap

> **Project:** Starfall  
> **Engine/Framework:** C++ with raylib  
> **Target:** Improve visual quality and game feel while keeping the project 2D, maintainable, and suitable for a university project.  
> **Approach:** Upgrade incrementally; preserve gameplay and test performance after each phase.

---

## 1. Vision

Give Starfall a polished **2D sci-fi identity** through distinctive ships, a layered space environment, responsive combat effects, and a consistent futuristic HUD.

**Recommended art direction:** Premium 2D sci-fi with restrained neon accents. Use dark backgrounds and controlled glow so ships, bullets, and UI remain readable during PvP.

## 2. Priorities at a Glance

| Priority | Upgrade | Difficulty | Expected impact |
|---|---|---:|---|
| P0 | Parallax starfield | Easy–Medium | High |
| P0 | Distinct ship visuals for all six ships | Medium | Very high |
| P0 | Projectile trails and thruster particles | Medium | High |
| P0 | Explosion and hit effects | Medium | Very high |
| P1 | Futuristic HUD and menu polish | Medium | High |
| P1 | Textured/varied asteroids | Medium | High |
| P1 | Ship-selection previews and transitions | Medium | Medium–High |
| P2 | Nebulae and distant planets | Medium | Medium–High |
| P2 | Glow/bloom using render textures or shaders | Hard | High, if subtle |
| P3 | Advanced shader effects and extra polish | Hard | Variable |

P0 items should be completed before adding advanced shaders.

---

## 3. Upgrade Specifications

### 3.1 Parallax Starfield

**Goal:** Make the space environment feel deeper without distracting from gameplay.

- Render multiple star layers at different speeds and sizes.
- Keep distant stars dim and small; make nearer stars slightly brighter.
- Use deterministic positions or a seeded random generator so the background is stable.
- Wrap stars around screen edges rather than recreating the entire field every frame.
- Keep background movement subtle, especially in PvP.

**Acceptance criteria**
- The background feels layered while remaining unobtrusive.
- No visible gaps appear when stars wrap.
- Background rendering does not cause noticeable frame-rate drops.

### 3.2 Distinct Ship Visuals

Create a recognizable silhouette for each ship. Do not rely only on color to communicate ship identity.

| Ship | Visual direction |
|---|---|
| Interceptor | Compact hull, sharp wings, bright blue engine accents |
| Striker | Angular aggressive hull, red/orange weapon accents |
| Bulwark | Broad armored hull, heavy plating, sturdy silhouette |
| Viper | Sleek narrow hull, green engine trails |
| Lancer | Long central cannon, focused piercing-energy effect |
| Wraith | Dark hull, violet accents, subtle regeneration particles |

- Prefer sprites with transparent backgrounds and consistent scale/origin.
- Keep ship rotation centered around its actual pivot.
- Use the same visual scale conventions for all ships.
- Add a simple fallback drawing path if a texture fails to load.
- Make team/player identity readable independently of ship color.

**Acceptance criteria**
- All six ships are recognizable at normal gameplay scale.
- Sprite orientation matches the movement and aiming direction.
- The ships remain readable against all backgrounds.

### 3.3 Thruster Particles

- Emit particles from the rear of the ship while thrusting.
- Vary particle lifetime, size, and velocity slightly.
- Use ship-specific accent colors sparingly.
- Stop or reduce emission when the ship is not thrusting.
- Cap the number of active particles and reuse particle objects where practical.

**Acceptance criteria**
- Particles follow the ship's movement and rotation correctly.
- Particles fade out smoothly.
- Particle count stays bounded during long matches.

### 3.4 Projectile Trails and Weapon Effects

- Add a short, fading trail to ordinary projectiles.
- Keep trails short enough that they do not obscure nearby players.
- Give the Lancer's piercing projectile a distinct visual treatment.
- Use brief muzzle flashes or firing sparks.
- Make the visual effect agree with actual projectile direction and collision behavior.

**Acceptance criteria**
- Projectiles remain easy to track.
- Trails do not imply a hit before collision is registered.
- Effects remain readable when both players fire at once.

### 3.5 Explosions, Hits, and Damage Feedback

- Spawn a short burst of particles when an asteroid or ship is destroyed.
- Add brief hit sparks at collision points where available.
- Flash a ship briefly after taking damage, respecting the game's invulnerability logic.
- Add small, restrained screen shake for major impacts only.
- Consider smoke/sparks at low health if it does not overwhelm the screen.

**Acceptance criteria**
- Damage and destruction are immediately understandable.
- Effects do not block player movement or important HUD information.
- Screen shake is subtle and can be reduced or disabled if necessary.

### 3.6 Asteroid Visual Variety

- Use several asteroid silhouettes or texture variants.
- Vary scale and rotation speed within controlled ranges.
- Keep collision shapes consistent with the actual asteroid size.
- Avoid visual details that make the collision boundary misleading.
- Consider different surface tones rather than excessive glow.

**Acceptance criteria**
- Asteroids do not all look identical.
- Visual size and collision size remain reasonably aligned.
- The screen stays readable in asteroid-heavy moments.

### 3.7 Futuristic HUD and Menus

- Establish a consistent typography, spacing, and panel style.
- Use a restrained palette: dark panels, bright text, and a few accent colors.
- Improve health display, player/ship labels, score, and connection status.
- Add hover/selected states to buttons.
- Use short fades or slides between menu states where appropriate.
- Make host/client status and network errors easy to distinguish.

**Acceptance criteria**
- Text remains legible at the game's target resolution.
- Important combat information is visible at a glance.
- Menus use consistent alignment and spacing.
- Network status is clear during multiplayer sessions.

### 3.8 Space Environment

After core effects are complete:

- Add one or two subtle nebula layers.
- Add distant planets as low-contrast background elements.
- Use parallax movement sparingly.
- Keep backgrounds behind gameplay entities.
- Consider separate background palettes for future maps or rounds.

**Acceptance criteria**
- Background elements never compete with bullets, ships, or UI.
- The game remains playable on a small window.

### 3.9 Glow and Shaders

Add this only after sprite, particle, and HUD work is stable.

- Prototype glow on a small set of objects first.
- If needed, render glowing objects to a `RenderTexture2D`, blur/composite them, and draw the normal scene clearly above or alongside the glow.
- Consider GLSL shaders only for effects that materially improve the game.
- Provide a simple non-shader fallback.
- Avoid full-screen heavy effects by default.

**Acceptance criteria**
- The game remains smooth on the target laptop's integrated graphics.
- Glow improves readability and atmosphere rather than washing out the scene.
- The game still works if an optional shader cannot be loaded.

---

## 4. Implementation Phases

### Phase 0 — Baseline and Asset Setup

1. Build and run the current project before changes.
2. Record the normal frame rate and test a busy scene.
3. Organize assets into folders, for example:
   - `assets/ships/`
   - `assets/asteroids/`
   - `assets/effects/`
   - `assets/backgrounds/`
   - `assets/ui/`
4. Define a consistent sprite scale, pivot, and visual palette.
5. Add a centralized asset-loading/unloading path.

**Deliverable:** A stable baseline and organized asset structure.

### Phase 1 — Core Visual Foundation

1. Implement the parallax starfield.
2. Add ship sprites for all six ships.
3. Add asteroid variants.
4. Improve the HUD and menu styling.

**Deliverable:** The game has a consistent visual identity even without advanced effects.

### Phase 2 — Combat Feel

1. Add thruster particles.
2. Add projectile trails and muzzle flashes.
3. Add hit sparks and destruction bursts.
4. Add restrained screen shake.
5. Verify effects with both players active.

**Deliverable:** Combat feels responsive and visually understandable.

### Phase 3 — Environment and Presentation

1. Add subtle nebulae and distant planets.
2. Improve ship-selection previews.
3. Add menu transitions and victory/defeat presentation.
4. Tune colors and contrast across gameplay states.

**Deliverable:** The entire game—not only combat—feels polished.

### Phase 4 — Advanced Rendering

1. Prototype glow/bloom on a limited set of effects.
2. Measure performance and compare against the baseline.
3. Add shader effects only if they provide a clear improvement.
4. Keep fallbacks for unsupported or failed shader loading.

**Deliverable:** Optional advanced effects with acceptable performance.

---

## 5. Suggested Code Organization

Adapt this to the project's existing structure rather than rewriting working systems.

```text
src/
  graphics/
    AssetManager.h / AssetManager.cpp
    ParticleSystem.h / ParticleSystem.cpp
    Background.h / Background.cpp
    Effects.h / Effects.cpp
    CameraEffects.h / CameraEffects.cpp
  ui/
    UI.h / UI.cpp

assets/
  ships/
  asteroids/
  effects/
  backgrounds/
  ui/
  shaders/
```

Suggested responsibilities:

- **AssetManager:** Load each texture/font/sound once and unload resources safely.
- **ParticleSystem:** Update, spawn, and draw bounded particle collections.
- **Background:** Draw and update star layers, nebulae, and distant planets.
- **Effects:** Coordinate explosions, muzzle flashes, hit sparks, and projectile trails.
- **CameraEffects:** Manage small, time-limited screen shake.
- **UI:** Keep HUD and menu styling consistent.

Avoid creating duplicate asset loaders if the project already has a suitable system.

---

## 6. Performance and Multiplayer Rules

- Keep simulation, collision, and network state authoritative; graphics effects must not change gameplay.
- Do not send individual particles or purely visual trails over the network.
- Derive visual effects locally from gameplay events when possible.
- Keep particle counts and lifetimes bounded.
- Load textures and shaders outside the per-frame update loop.
- Unload resources when the game closes or when they are no longer needed.
- Avoid blocking disk or network operations inside rendering code.
- Test host and client views independently; visual effects should not alter hitboxes or movement.
- Use a simple fallback when an asset or shader fails to load.
- Test window resizing and the minimum supported resolution.

**Performance target:** Aim for stable 60 FPS at the intended resolution. Treat this as a target to measure, not a guarantee.

---

## 7. Testing Checklist

- [ ] Project builds and runs before and after each phase.
- [ ] All six ship types render and rotate correctly.
- [ ] Ship selection displays the correct ship.
- [ ] Background stars wrap without gaps or flicker.
- [ ] Thruster particles appear only under the intended conditions.
- [ ] Projectile trails follow the correct direction.
- [ ] Hits, invulnerability, and destruction effects match gameplay events.
- [ ] HUD text remains readable during combat.
- [ ] Host/client gameplay remains synchronized.
- [ ] Graphics effects do not affect collision or network messages.
- [ ] Performance remains acceptable during heavy effects.
- [ ] Textures, fonts, render textures, and shaders are unloaded safely.
- [ ] Missing assets do not crash the game.

---

## 8. Definition of Done

The graphics upgrade is complete when:

1. Starfall has a consistent, recognizable 2D sci-fi art direction.
2. All six ships have distinct silhouettes.
3. The background has depth without distracting from combat.
4. Combat includes clear thrust, firing, hit, and destruction feedback.
5. HUD and menus look consistent and remain readable.
6. Host/client gameplay behaves as before.
7. Performance has been tested and no major regressions remain.
8. The project builds from a clean checkout with the required assets documented.

---

## 9. Recommended First Milestone

Implement only these first:

1. Parallax starfield.
2. Six ship sprites.
3. Thruster particles.
4. Projectile trails.
5. Explosion particles.
6. Basic HUD polish.

**Do not start with bloom or complex shaders.** These six improvements provide a strong visible upgrade while keeping the project manageable. Once they work, move on to background art and advanced rendering.

## 10. Progress Tracker

| Task | Status | Notes |
|---|---|---|
| Record baseline performance | Done | `Starfall.exe --bench` — 600 scripted frames; avg 2.19 ms / p95 2.36 ms on Intel UHD laptop |
| Organize graphics assets | N/A | Rendering is fully procedural; no texture assets used |
| Parallax starfield | Done | `Background.h/.cpp` — 3 seeded layers, wrapped drift, low contrast |
| Six ship sprites | Done | Distinct procedural silhouettes in `ShipRenderer.cpp`, accent-tinted exhaust |
| Asteroid variants | Done | `Asteroid.cpp` — 7–9 sides, per-vertex jitter, 3 tone palettes, craters, varied spin |
| Thruster particles | Done | `ParticleSystem` — bounded pool, accent-tinted, thrust/reverse emission |
| Projectile trails | Done | 6-sample fading trail; client reuses remote bullets to preserve trails |
| Hit and explosion effects | Done | Spawned from actual gameplay events on host and derived locally on client |
| HUD/menu polish | Done | Consistent panels, accent palette, restyled pips/menus in `UI.cpp` |
| Nebula/planet layers | Done | Prerendered nebula texture with parallax wrap + 2 distant planets in `Background` |
| Glow/shader prototype | Done | `GlowLayer` half-res bloom, `assets/shaders/glow_blur.fs` with additive fallback |
| Host/client visual test | Done | Automated LAN run: both reach gameplay, client detects host loss (23/23 checks pass); effects stay local-only |
| Final performance pass | Done | `--bench` after all effects: avg 2.19 ms (~456 FPS), well under 16.7 ms frame budget |
