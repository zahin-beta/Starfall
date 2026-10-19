#include "CameraEffects.h"
#include <cmath>

void CameraEffects::Update(float deltaTime) {
    if (timeLeft <= 0.0f) {
        offset = Vector2{0.0f, 0.0f};
        return;
    }
    timeLeft -= deltaTime;
    if (timeLeft <= 0.0f) {
        timeLeft = 0.0f;
        offset = Vector2{0.0f, 0.0f};
        return;
    }
    const float falloff = timeLeft / duration;
    const float strength = magnitude * falloff;
    const float time = static_cast<float>(GetTime()) * 47.0f;
    offset = Vector2{std::sin(time * 1.7f) * strength, std::cos(time * 2.3f) * strength};
}

void CameraEffects::Shake(float value, float seconds) {
    if (value <= 0.0f || seconds <= 0.0f) return;
    if (value > magnitude || timeLeft <= 0.0f) {
        magnitude = value;
        duration = seconds;
    }
    if (seconds > timeLeft) timeLeft = seconds;
}
