#pragma once
#include "raylib.h"
class Bullet {
public:
    explicit Bullet(Vector2 startPosition);
    void Update(float deltaTime);
    void Draw() const;
    bool IsOffScreen() const;
    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
private:
    Vector2 position{};
    float speed = 620.0f, radius = 4.0f;
};
