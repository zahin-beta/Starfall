#pragma once
#include "Asteroid.h"
#include "AudioManager.h"
#include "Bullet.h"
#include "Discovery.h"
#include "Enemy.h"
#include "NetPeer.h"
#include "Player.h"
#include "Protocol.h"
#include "UI.h"
#include <cstdint>
#include <deque>
#include <string>
#include <utility>
#include <vector>

class Game {
public:
    Game();
    void Run();

private:
    enum class Mode { Survival, Host, Client };
    enum class State { Menu, ShipSelect, PortEntry, HostLobby, ClientLobby, ManualEntry, Connecting, Playing, Paused, Forfeiting, GameOver, Disconnected };

    static constexpr int kScreenWidth = 960;
    static constexpr int kScreenHeight = 720;
    static constexpr float kStep = net::kFixedStep;
    static constexpr int kRoundsToWin = 2;

    void Update(float deltaTime);
    void Draw() const;
    void DrawBackground() const;
    void DrawVersusField() const;

    void UpdateSurvival(float deltaTime);
    void DrawSurvival() const;
    void ResetSurvival();

    void UpdatePause(float deltaTime);
    void ResumeFromPause();
    void ForfeitMatch();
    void SendPause(bool paused);

    void UpdateMenu();
    void UpdateShipSelect();
    void UpdatePortEntry();
    void UpdateHostLobby(float deltaTime);
    void UpdateClientLobby(float deltaTime);
    void UpdateManualEntry();
    void UpdateConnecting(float deltaTime);
    void UpdateVersus(float deltaTime);

    void StartSurvival();
    void StartHosting(uint16_t port);
    void StartJoin(const std::string& ip, uint16_t port);
    void SendHello();
    void ReturnToMenu();

    void PumpReliable();
    void PumpUnreliable();
    void HostStep();
    void ClientStep();
    void SendSnapshot();
    void SendInput();
    void ApplySnapshot(const net::SnapshotMsg& snapshot);
    void CaptureSnapshot(net::SnapshotMsg& snapshot) const;
    void StartMatch();
    void ResetRound();
    void ResolveVersusCollisions();
    void SpawnVersusAsteroids(int count);

    InputCommand LocalHostInput() const;
    static InputCommand DecodeButtons(uint8_t bits);
    static uint8_t EncodeButtons(const InputCommand& command);
    PlayerConfig MakePlayerConfig(int id, int shipId) const;
    void SpawnBullet(const Player& player);

    Mode mode = Mode::Survival;
    State state = State::Menu;

    Player players[2];
    UI ui;
    AudioManager audio;

    net::NetPeer net;
    net::Discovery discovery;

    std::vector<Bullet> bullets;
    std::vector<Asteroid> asteroids;
    std::vector<Enemy> enemies;
    float spawnTimer = 0.0f;
    float elapsedTime = 0.0f;
    int score = 0;

    std::vector<Bullet> vsBullets;
    std::vector<Bullet> remoteBullets;
    std::vector<Asteroid> vsAsteroids;
    int localId = 0;
    int hostShipId = 0;
    int clientShipId = 0;
    uint32_t serverTick = 0;
    uint32_t clientTick = 0;
    uint32_t lastClientTickSeen = 0;
    uint8_t latestClientButtons = 0;
    std::deque<std::pair<uint32_t, InputCommand>> inputHistory;
    net::MatchPhase phase = net::MatchPhase::Playing;
    int wins[2] = {0, 0};
    float phaseTimer = 0.0f;
    float vsAccumulator = 0.0f;
    float vsSpawnTimer = 0.0f;
    int previousHealth[2] = {3, 3};
    int previousAsteroidCount = 0;

    std::string portText = "45801";
    std::string manualIp = "127.0.0.1";
    std::string manualPort = "45801";
    bool manualEditingIp = true;
    std::vector<std::string> lobbyLines;
    int lobbySelected = 0;
    int shipSelectId = 0;
    int pauseSelection = 0;
    float forfeitTimer = 0.0f;
    bool exitRequested = false;
    std::string statusText;
    std::string endNote;
};
