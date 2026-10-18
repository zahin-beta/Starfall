#pragma once
#include "InputCommand.h"
#include "ShipCatalog.h"
#include "raylib.h"

struct PlayerControls {
    int rotateLeft = KEY_A;
    int rotateRight = KEY_D;
    int thrust = KEY_W;
    int reverse = KEY_S;
    int fire = KEY_SPACE;
};

struct PlayerConfig {
    int id = 0;
    Vector2 spawn{};
    float aimAngle = 0.0f;
    Color color{77, 210, 255, 255};
    PlayerControls controls{};
    const ShipDef* ship = nullptr;
};

class Player {
public:
    Player() = default;
    explicit Player(const PlayerConfig& config);
    void Reset();
    void Update(float deltaTime, const InputCommand& input);
    void UpdateRegen(float deltaTime);
    InputCommand ReadKeyboard() const;
    void Draw() const;
    bool TryShoot();
    void TakeDamage();
    void ApplyKnockback(Vector2 impulse);
    Vector2 GetPosition() const { return position; }
    Vector2 GetVelocity() const { return velocity; }
    Vector2 AimDirection() const;
    float GetRadius() const { return radius; }
    float GetAimAngle() const { return aimAngle; }
    int GetHealth() const { return health; }
    int GetMaxHealth() const { return Def().health; }
    int GetId() const { return config.id; }
    Color GetColor() const { return config.color; }
    Vector2 GetKnockback() const { return knockback; }
    float GetInvulnerability() const { return invulnerabilityTimer; }
    float GetThrottle() const { return throttle; }
    bool IsReversing() const { return reversing; }
    const ShipDef& Def() const;
    const char* GetShipName() const { return Def().name; }
    void SetPosition(Vector2 value) { position = value; }
    void SetVelocity(Vector2 value) { velocity = value; }
    void SetAimAngle(float value) { aimAngle = value; }
    void SetHealth(int value) { health = value; }
    void SetKnockback(Vector2 value) { knockback = value; }
    void SetInvulnerability(float value) { invulnerabilityTimer = value; }
    void SetThrottle(float value) { throttle = value; }
    void SetReversing(bool value) { reversing = value; }
private:
    PlayerConfig config{};
    Vector2 position{};
    Vector2 velocity{};
    Vector2 knockback{};
    float aimAngle = 0.0f;
    float radius = kShipRadius;
    float throttle = 0.0f;
    float shootCooldown = 0.0f;
    float invulnerabilityTimer = 0.0f;
    float regenTimer = 0.0f;
    bool reversing = false;
    int health = 3;
};
