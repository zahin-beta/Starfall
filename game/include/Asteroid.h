#pragma once
#include "raylib.h"
class Asteroid {
public:
    Asteroid(Vector2 startPosition, Vector2 velocity, float radius);
    void Update(float deltaTime);
    void Draw() const;
    bool IsOffScreen() const;
    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    float GetRotation() const { return rotation; }
    void SetRotation(float value) { rotation = value; }
private:
    Vector2 position{}, velocity{};
    float radius = 20.0f, rotation = 0.0f;
};
