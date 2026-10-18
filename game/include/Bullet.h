#pragma once
#include "raylib.h"
class Bullet {
public:
    Bullet(Vector2 startPosition, Vector2 velocity, Color color, int ownerId, float radiusValue = 4.0f, int pierceValue = 0);
    void Update(float deltaTime);
    void Draw() const;
    bool IsOffScreen() const;
    bool TryPierce();
    Vector2 GetPosition() const { return position; }
    Vector2 GetVelocity() const { return velocity; }
    float GetRadius() const { return radius; }
    int GetOwner() const { return ownerId; }
private:
    Vector2 position{};
    Vector2 velocity{};
    Color color{SKYBLUE};
    int ownerId = 0;
    int pierce = 0;
    float radius = 4.0f;
};
