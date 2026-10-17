#include "Bullet.h"
Bullet::Bullet(Vector2 startPosition) : position(startPosition) {}
void Bullet::Update(float deltaTime) { position.y -= speed * deltaTime; }
void Bullet::Draw() const {
    DrawCircleV(position, radius + 2.0f, Color{72, 205, 255, 70});
    DrawCircleV(position, radius, SKYBLUE);
}
bool Bullet::IsOffScreen() const { return position.y < -radius; }
