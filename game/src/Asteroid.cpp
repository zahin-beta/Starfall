#include "Asteroid.h"
Asteroid::Asteroid(Vector2 startPosition, Vector2 velocityValue, float radiusValue)
    : position(startPosition), velocity(velocityValue), radius(radiusValue) {}
void Asteroid::Update(float deltaTime) {
    position.x += velocity.x * deltaTime; position.y += velocity.y * deltaTime;
    rotation += 35.0f * deltaTime;
}
void Asteroid::Draw() const {
    DrawPoly(position, 8, radius, rotation, Color{139, 119, 105, 255});
    DrawPolyLines(position, 8, radius, rotation, Color{205, 174, 145, 255});
    DrawCircleV({position.x - radius * 0.28f, position.y - radius * 0.12f}, radius * 0.18f, Color{90, 81, 77, 255});
    DrawCircleV({position.x + radius * 0.3f, position.y + radius * 0.25f}, radius * 0.12f, Color{90, 81, 77, 255});
}
bool Asteroid::IsOffScreen() const {
    return position.y - radius > GetScreenHeight() + 20.0f ||
           position.x + radius < -20.0f || position.x - radius > GetScreenWidth() + 20.0f;
}
