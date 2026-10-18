#include "Bullet.h"
Bullet::Bullet(Vector2 startPosition, Vector2 velocityValue, Color colorValue, int owner, float radiusValue, int pierceValue)
    : position(startPosition), velocity(velocityValue), color(colorValue), ownerId(owner), pierce(pierceValue), radius(radiusValue) {}
bool Bullet::TryPierce() {
    if (pierce > 0) {
        --pierce;
        return true;
    }
    return false;
}
void Bullet::Update(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}
void Bullet::Draw() const {
    Color glow = color;
    glow.a = 70;
    DrawCircleV(position, radius + 3.0f, glow);
    DrawCircleV(position, radius, color);
}
bool Bullet::IsOffScreen() const {
    return position.y < -radius || position.y > static_cast<float>(GetScreenHeight()) + radius ||
           position.x < -radius || position.x > static_cast<float>(GetScreenWidth()) + radius;
}
