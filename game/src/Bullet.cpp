#include "Bullet.h"
Bullet::Bullet(Vector2 startPosition, Vector2 velocityValue, Color colorValue, int owner, float radiusValue, int pierceValue)
    : position(startPosition), velocity(velocityValue), color(colorValue), ownerId(owner), pierce(pierceValue), radius(radiusValue) {
    PushTrail();
}
bool Bullet::TryPierce() {
    if (pierce > 0) {
        --pierce;
        return true;
    }
    return false;
}
void Bullet::PushTrail() {
    trail[trailHead] = position;
    trailHead = (trailHead + 1) % kTrailLength;
    if (trailCount < kTrailLength) ++trailCount;
}
void Bullet::Update(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    PushTrail();
}
void Bullet::SetPosition(Vector2 value) {
    const float dx = value.x - position.x;
    const float dy = value.y - position.y;
    if (dx * dx + dy * dy > 900.0f) {
        trailCount = 0;
        trailHead = 0;
    }
    position = value;
    PushTrail();
}
void Bullet::Draw() const {
    for (int i = 0; i < trailCount; ++i) {
        const int index = (trailHead - 1 - i + kTrailLength * 2) % kTrailLength;
        const float ratio = 1.0f - static_cast<float>(i) / static_cast<float>(kTrailLength);
        Color trailColor = color;
        trailColor.a = static_cast<unsigned char>(85.0f * ratio * ratio);
        DrawCircleV(trail[index], radius * (0.30f + 0.55f * ratio), trailColor);
    }
    Color glow = color;
    glow.a = 70;
    DrawCircleV(position, radius + 3.0f, glow);
    DrawCircleV(position, radius, color);
}
void Bullet::DrawGlow() const {
    Color halo = color;
    halo.a = 46;
    DrawCircleV(position, radius + 8.0f, halo);
    for (int i = 0; i < trailCount; ++i) {
        const int index = (trailHead - 1 - i + kTrailLength * 2) % kTrailLength;
        const float ratio = 1.0f - static_cast<float>(i) / static_cast<float>(kTrailLength);
        Color trailColor = color;
        trailColor.a = static_cast<unsigned char>(30.0f * ratio);
        DrawCircleV(trail[index], radius * 1.8f * ratio, trailColor);
    }
}
bool Bullet::IsOffScreen() const {
    return position.y < -radius || position.y > static_cast<float>(GetScreenHeight()) + radius ||
           position.x < -radius || position.x > static_cast<float>(GetScreenWidth()) + radius;
}
