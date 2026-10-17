#pragma once
#include "raylib.h"
class Enemy {
public:
    Enemy() = default;
    void Reset() { active = false; }
    void Update(float deltaTime);
    void Draw() const;
    bool IsActive() const { return active; }
private:
    Vector2 position{};
    bool active = false;
};
