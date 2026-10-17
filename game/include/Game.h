#pragma once
#include "Asteroid.h"
#include "AudioManager.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"
#include "UI.h"
#include <vector>
class Game {
public:
    Game();
    void Run();
private:
    enum class State { Menu, Playing, GameOver };
    void ResetRun();
    void Update(float deltaTime);
    void UpdatePlaying(float deltaTime);
    void Draw() const;
    void DrawBackground() const;
    void SpawnAsteroid();
    void ResolveCollisions();
    static constexpr int kScreenWidth = 960;
    static constexpr int kScreenHeight = 720;
    State state = State::Menu;
    Player player;
    UI ui;
    AudioManager audio;
    std::vector<Bullet> bullets;
    std::vector<Asteroid> asteroids;
    std::vector<Enemy> enemies;
    float spawnTimer = 0.0f, elapsedTime = 0.0f;
    int score = 0;
};
