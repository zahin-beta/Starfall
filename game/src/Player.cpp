#include "Player.h"
#include "ShipRenderer.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kPlayerMargin = 24.0f;
constexpr float kDegToRad = 0.01745329251994329577f;
constexpr float kBaseInvulnerability = 1.1f;

Vector2 Add(Vector2 a, Vector2 b) { return Vector2{a.x + b.x, a.y + b.y}; }
Vector2 Scale(Vector2 value, float factor) { return Vector2{value.x * factor, value.y * factor}; }
}

const ShipDef& Player::Def() const { return GetShip(config.ship ? static_cast<int>(config.ship->id) : 0); }

Player::Player(const PlayerConfig& playerConfig) : config(playerConfig) { Reset(); }

Vector2 Player::AimDirection() const {
    const float radians = aimAngle * kDegToRad;
    return Vector2{std::sin(radians), -std::cos(radians)};
}

void Player::Reset() {
    const ShipDef& ship = Def();
    position = config.spawn;
    velocity = Vector2{0.0f, 0.0f};
    knockback = Vector2{0.0f, 0.0f};
    aimAngle = config.aimAngle;
    shootCooldown = 0.0f;
    invulnerabilityTimer = 0.0f;
    regenTimer = 0.0f;
    throttle = 0.0f;
    reversing = false;
    health = ship.health;
}

InputCommand Player::ReadKeyboard() const {
    InputCommand command;
    command.rotateLeft = IsKeyDown(config.controls.rotateLeft);
    command.rotateRight = IsKeyDown(config.controls.rotateRight);
    command.thrust = IsKeyDown(config.controls.thrust);
    command.reverse = IsKeyDown(config.controls.reverse);
    command.fire = IsKeyDown(config.controls.fire);
    return command;
}

void Player::Update(float deltaTime, const InputCommand& input) {
    const ShipDef& ship = Def();

    float turn = 0.0f;
    if (input.rotateLeft) turn -= 1.0f;
    if (input.rotateRight) turn += 1.0f;
    aimAngle += turn * ship.turnSpeed * deltaTime;

    const Vector2 direction = AimDirection();
    const float throttleTarget = input.thrust ? 1.0f : 0.0f;
    throttle += (throttleTarget - throttle) * std::min(1.0f, 14.0f * deltaTime);
    if (throttle < 0.001f) throttle = 0.0f;
    reversing = input.reverse;

    if (input.thrust) velocity = Add(velocity, Scale(direction, ship.thrust * deltaTime));
    if (input.reverse) velocity = Add(velocity, Scale(direction, -ship.thrust * ship.reverseMultiplier * deltaTime));

    velocity = Scale(velocity, std::max(0.0f, 1.0f - ship.drag * deltaTime));
    float maxSpeed = ship.maxSpeed;
    if (ship.speedBonusWhileThrusting > 0.0f) maxSpeed *= 1.0f + ship.speedBonusWhileThrusting * throttle;
    const float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if (speed > maxSpeed) velocity = Scale(velocity, maxSpeed / speed);

    knockback = Scale(knockback, std::max(0.0f, 1.0f - 4.5f * deltaTime));

    position = Add(position, Scale(Add(velocity, knockback), deltaTime));
    position.x = std::clamp(position.x, kPlayerMargin, static_cast<float>(GetScreenWidth()) - kPlayerMargin);
    position.y = std::clamp(position.y, kPlayerMargin, static_cast<float>(GetScreenHeight()) - kPlayerMargin);

    shootCooldown = std::max(0.0f, shootCooldown - deltaTime);
    invulnerabilityTimer = std::max(0.0f, invulnerabilityTimer - deltaTime);
}

void Player::UpdateRegen(float deltaTime) {
    const ShipDef& ship = Def();
    if (ship.regenDelay <= 0.0f || health >= ship.health || health <= 0) {
        regenTimer = 0.0f;
        return;
    }
    if (invulnerabilityTimer > 0.0f) {
        regenTimer = 0.0f;
        return;
    }
    regenTimer += deltaTime;
    if (regenTimer >= ship.regenDelay) {
        regenTimer = 0.0f;
        ++health;
    }
}

void Player::Draw() const {
    ShipRenderer::DrawShip(Def(), position, aimAngle, radius, throttle, reversing, invulnerabilityTimer > 0.0f);
}

bool Player::TryShoot() {
    if (shootCooldown > 0.0f) return false;
    shootCooldown = Def().fireInterval;
    return true;
}

void Player::TakeDamage() {
    if (invulnerabilityTimer > 0.0f || health <= 0) return;
    --health;
    regenTimer = 0.0f;
    invulnerabilityTimer = kBaseInvulnerability * Def().invulnMultiplier;
}

void Player::ApplyKnockback(Vector2 impulse) {
    knockback = Add(knockback, Scale(impulse, Def().knockbackTaken));
}
