#pragma once
#include "raylib.h"
class Player {
public:
    explicit Player(int screenWidth, int screenHeight);
    void Reset(int screenWidth, int screenHeight);
    void Update(float deltaTime);
    void Draw() const;
    bool TryShoot();
    void TakeDamage();
    Vector2 GetPosition() const { return position; }
    int GetHealth() const { return health; }
private:
    Vector2 position{};
    float radius = 18.0f, speed = 330.0f, shootCooldown = 0.0f, invulnerabilityTimer = 0.0f;
    int health = 3;
};
