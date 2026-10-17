#include "Player.h"
#include <algorithm>
#include <cmath>
namespace { constexpr float kPlayerMargin = 24.0f; }
Player::Player(int screenWidth, int screenHeight) { Reset(screenWidth, screenHeight); }
void Player::Reset(int screenWidth, int screenHeight) {
    position = {screenWidth * 0.5f, screenHeight * 0.78f};
    shootCooldown = 0.0f; invulnerabilityTimer = 0.0f; health = 3;
}
void Player::Update(float deltaTime) {
    Vector2 direction{0.0f, 0.0f};
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction.y += 1.0f;
    if (direction.x != 0.0f || direction.y != 0.0f) {
        const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        direction.x /= length; direction.y /= length;
        position.x += direction.x * speed * deltaTime;
        position.y += direction.y * speed * deltaTime;
    }
    position.x = std::clamp(position.x, kPlayerMargin, static_cast<float>(GetScreenWidth()) - kPlayerMargin);
    position.y = std::clamp(position.y, kPlayerMargin, static_cast<float>(GetScreenHeight()) - kPlayerMargin);
    shootCooldown = std::max(0.0f, shootCooldown - deltaTime);
    invulnerabilityTimer = std::max(0.0f, invulnerabilityTimer - deltaTime);
}
void Player::Draw() const {
    if (invulnerabilityTimer > 0.0f && static_cast<int>(GetTime() * 12.0) % 2 == 0) return;
    DrawTriangle({position.x, position.y - radius - 5.0f},
                 {position.x - radius, position.y + radius},
                 {position.x + radius, position.y + radius}, Color{77, 210, 255, 255});
    DrawTriangle({position.x, position.y - 9.0f}, {position.x - 8.0f, position.y + 11.0f},
                 {position.x + 8.0f, position.y + 11.0f}, Color{225, 250, 255, 255});
    DrawCircleV({position.x, position.y + radius - 2.0f}, 5.0f, ORANGE);
}
bool Player::TryShoot() {
    if (shootCooldown > 0.0f) return false;
    shootCooldown = 0.22f; return true;
}
void Player::TakeDamage() {
    if (invulnerabilityTimer > 0.0f || health <= 0) return;
    --health; invulnerabilityTimer = 1.1f;
}
