#include "Game.h"
#include "Collision.h"
#include "raylib.h"
#include <algorithm>
Game::Game() : player(kScreenWidth, kScreenHeight) {}
void Game::Run() {
    InitWindow(kScreenWidth, kScreenHeight, "Starfall");
    SetTargetFPS(60);
    audio.Initialize();
    while (!WindowShouldClose()) {
        const float deltaTime = GetFrameTime();
        Update(deltaTime);
        Draw();
    }
    audio.Shutdown();
    CloseWindow();
}
void Game::ResetRun() {
    player.Reset(kScreenWidth, kScreenHeight);
    bullets.clear(); asteroids.clear(); enemies.clear();
    spawnTimer = 0.45f; elapsedTime = 0.0f; score = 0;
    state = State::Playing;
}
void Game::Update(float deltaTime) {
    if (state == State::Menu) {
        if (IsKeyPressed(KEY_ENTER)) ResetRun();
        return;
    }
    if (state == State::GameOver) {
        if (IsKeyPressed(KEY_ENTER)) ResetRun();
        return;
    }
    UpdatePlaying(deltaTime);
}
void Game::UpdatePlaying(float deltaTime) {
    elapsedTime += deltaTime;
    player.Update(deltaTime);
    if (IsKeyDown(KEY_SPACE) && player.TryShoot()) {
        bullets.emplace_back(Vector2{player.GetPosition().x, player.GetPosition().y - 24.0f});
        audio.PlayShoot();
    }
    spawnTimer -= deltaTime;
    if (spawnTimer <= 0.0f) {
        SpawnAsteroid();
        const float difficulty = std::min(0.85f, elapsedTime * 0.003f);
        spawnTimer = std::max(0.48f, 1.15f - difficulty) * GetRandomValue(78, 122) / 100.0f;
    }
    for (auto& bullet : bullets) bullet.Update(deltaTime);
    for (auto& asteroid : asteroids) asteroid.Update(deltaTime);
    for (auto& enemy : enemies) enemy.Update(deltaTime);
    ResolveCollisions();
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& bullet) { return bullet.IsOffScreen(); }), bullets.end());
    asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(),
        [](const Asteroid& asteroid) { return asteroid.IsOffScreen(); }), asteroids.end());
    if (player.GetHealth() <= 0) state = State::GameOver;
}
void Game::SpawnAsteroid() {
    const float x = static_cast<float>(GetRandomValue(28, kScreenWidth - 28));
    const float radius = static_cast<float>(GetRandomValue(17, 32));
    const float speedY = static_cast<float>(GetRandomValue(105, 180)) + std::min(75.0f, elapsedTime * 1.5f);
    const float drift = static_cast<float>(GetRandomValue(-45, 45));
    asteroids.emplace_back(Vector2{x, -radius}, Vector2{drift, speedY}, radius);
}
void Game::ResolveCollisions() {
    for (auto bullet = bullets.begin(); bullet != bullets.end();) {
        bool hit = false;
        for (auto asteroid = asteroids.begin(); asteroid != asteroids.end(); ++asteroid) {
            if (Collision::CirclesOverlap(bullet->GetPosition(), bullet->GetRadius(),
                                          asteroid->GetPosition(), asteroid->GetRadius())) {
                asteroids.erase(asteroid);
                score += 100;
                hit = true;
                audio.PlayExplosion();
                break;
            }
        }
        if (hit) bullet = bullets.erase(bullet);
        else ++bullet;
    }
    for (const auto& asteroid : asteroids) {
        if (Collision::CirclesOverlap(player.GetPosition(), 17.0f,
                                      asteroid.GetPosition(), asteroid.GetRadius())) player.TakeDamage();
    }
}
void Game::DrawBackground() const {
    ClearBackground(Color{5, 9, 24, 255});
    for (int i = 0; i < 75; ++i) {
        const int x = (i * 137 + 41) % kScreenWidth;
        const int y = (i * 223 + 17) % kScreenHeight;
        const int radius = (i % 9 == 0) ? 2 : 1;
        const unsigned char alpha = static_cast<unsigned char>(105 + (i * 31) % 145);
        DrawCircle(x, y, static_cast<float>(radius), Color{180, 211, 255, alpha});
    }
}
void Game::Draw() const {
    BeginDrawing();
    DrawBackground();
    if (state == State::Menu) {
        ui.DrawMenu();
    } else {
        for (const auto& bullet : bullets) bullet.Draw();
        for (const auto& asteroid : asteroids) asteroid.Draw();
        for (const auto& enemy : enemies) enemy.Draw();
        if (state == State::Playing) player.Draw();
        ui.DrawHud(score, player.GetHealth());
        if (state == State::GameOver) ui.DrawGameOver(score);
    }
    EndDrawing();
}
