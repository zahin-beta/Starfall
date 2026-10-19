#pragma once
#include "raylib.h"

class CameraEffects {
public:
    void Update(float deltaTime);
    void Shake(float magnitude, float duration);
    Vector2 GetOffset() const { return offset; }
    bool IsShaking() const { return timeLeft > 0.0f; }

private:
    float magnitude = 0.0f;
    float timeLeft = 0.0f;
    float duration = 0.0f;
    Vector2 offset{};
};
