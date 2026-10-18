#pragma once
#include <cstdint>
#include <cstddef>

namespace net {

constexpr uint32_t kMagic = 0x5354464Cu; // 'STFL'
constexpr uint16_t kVersion = 6;
constexpr uint16_t kDiscoveryPort = 45800;
constexpr uint16_t kDefaultGamePort = 45801;
constexpr uint16_t kGamePortMin = 49152; // ephemeral range start (Windows default dynamic start)
constexpr uint16_t kGamePortMax = 65000;
constexpr int kPortBindAttempts = 32;
constexpr int kMaxPacket = 1200;
constexpr int kInputRedundancy = 8;
constexpr int kMaxBullets = 40;
constexpr int kMaxAsteroids = 24;
constexpr float kFixedStep = 1.0f / 60.0f;

enum class PacketKind : uint8_t {
    Reliable = 1,
    Unreliable = 2,
};

enum class AppMsg : uint8_t {
    Hello = 1,
    Welcome,
    StartMatch,
    Disconnect,
    Input,
    Snapshot,
    Pause,
};

enum InputBits {
    BitRotateLeft = 1 << 0,
    BitRotateRight = 1 << 1,
    BitThrust = 1 << 2,
    BitReverse = 1 << 3,
    BitFire = 1 << 4,
};

enum class MatchPhase : uint8_t {
    Playing = 0,
    RoundOver,
    MatchOver,
};

enum class DisconnectReason : uint8_t {
    Left = 0,
    Forfeit,
};

#pragma pack(push, 1)
struct PacketHeader {
    uint32_t magic;
    uint16_t version;
    uint8_t kind;
    uint16_t seq;
    uint16_t ack;
    uint32_t ackBits;
};

struct BeaconPacket {
    uint32_t magic;
    uint16_t version;
    char name[32];
    uint16_t gamePort;
    uint8_t players;
};

struct InputEntry {
    uint32_t tick;
    uint8_t buttons;
};

struct InputMsg {
    uint8_t type;
    InputEntry entries[kInputRedundancy];
};

struct PlayerNet {
    float x;
    float y;
    float aimAngle;
    float vx;
    float vy;
    float knockbackX;
    float knockbackY;
    float invulnerability;
    int16_t health;
    uint8_t thrusting;
};

struct BulletNet {
    float x;
    float y;
    float angle;
    uint8_t owner;
};

struct AsteroidNet {
    float x;
    float y;
    float radius;
    float rotation;
};

struct SnapshotMsg {
    uint8_t type;
    uint32_t serverTick;
    uint32_t ackedInputTick;
    uint8_t phase;
    uint8_t wins[2];
    PlayerNet players[2];
    uint8_t bulletCount;
    uint8_t asteroidCount;
    BulletNet bullets[kMaxBullets];
    AsteroidNet asteroids[kMaxAsteroids];
};

struct HelloMsg {
    uint8_t type;
    uint8_t shipId;
    char name[24];
};

struct WelcomeMsg {
    uint8_t type;
    uint8_t playerId;
    uint8_t roundsToWin;
    uint8_t shipIds[2];
};

struct DisconnectMsg {
    uint8_t type;
    uint8_t reason;
};

struct PauseMsg {
    uint8_t type;
    uint8_t paused;
};
#pragma pack(pop)

}
