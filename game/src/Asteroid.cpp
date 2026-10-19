#include "Asteroid.h"
#include <cmath>

namespace {
struct Tone {
    Color fill;
    Color edge;
    Color crater;
};

const Tone kTones[] = {
    {{139, 119, 105, 255}, {205, 174, 145, 255}, {90, 81, 77, 255}},
    {{112, 122, 141, 255}, {168, 182, 208, 255}, {72, 80, 96, 255}},
    {{126, 112, 124, 255}, {186, 164, 180, 255}, {82, 74, 84, 255}},
};
constexpr int kToneCount = 3;
}

Asteroid::Asteroid(Vector2 startPosition, Vector2 velocityValue, float radiusValue)
    : position(startPosition), velocity(velocityValue), radius(radiusValue) {
    sides = GetRandomValue(7, 9);
    palette = GetRandomValue(0, kToneCount - 1);
    spinSpeed = static_cast<float>(GetRandomValue(-55, 55));
    if (spinSpeed >= 0.0f && spinSpeed < 18.0f) spinSpeed = 18.0f + static_cast<float>(GetRandomValue(0, 20));
    for (int i = 0; i < 10; ++i) vertexScale[i] = 1.0f;
    for (int i = 0; i < sides; ++i) {
        const float roll = static_cast<float>(GetRandomValue(0, 100)) / 100.0f;
        vertexScale[i] = 0.74f + 0.30f * roll;
    }
    for (int i = 0; i < 2; ++i) {
        const float angle = static_cast<float>(GetRandomValue(0, 359)) * 0.0174532925f;
        const float dist = static_cast<float>(GetRandomValue(20, 55)) / 100.0f;
        craters[i] = Vector2{std::cos(angle) * radius * dist, std::sin(angle) * radius * dist};
        craterRadius[i] = radius * (static_cast<float>(GetRandomValue(10, 22)) / 100.0f);
    }
}

void Asteroid::Update(float deltaTime) {
    position.x += velocity.x * deltaTime; position.y += velocity.y * deltaTime;
    rotation += spinSpeed * deltaTime;
}

void Asteroid::Draw() const {
    const Tone& tone = kTones[palette];
    Vector2 points[11];
    points[0] = position;
    const float rotationRad = rotation * 0.0174532925f;
    const float step = 2.0f * 3.14159265f / static_cast<float>(sides);
    for (int i = 0; i < sides; ++i) {
        const float angle = rotationRad + static_cast<float>(sides - i) * step;
        const float r = radius * vertexScale[i];
        points[i + 1] = Vector2{position.x + std::cos(angle) * r, position.y + std::sin(angle) * r};
    }
    points[sides + 1] = points[1];
    DrawTriangleFan(points, sides + 1, tone.fill);
    DrawLineStrip(points, sides + 2, tone.edge);
    for (int i = 0; i < 2; ++i) {
        DrawCircleV(Vector2{position.x + craters[i].x, position.y + craters[i].y}, craterRadius[i], tone.crater);
    }
}

bool Asteroid::IsOffScreen() const {
    return position.y - radius > GetScreenHeight() + 20.0f ||
           position.x + radius < -20.0f || position.x - radius > GetScreenWidth() + 20.0f;
}
