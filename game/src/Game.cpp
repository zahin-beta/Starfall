#include "Game.h"
#include "Collision.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace {
Vector2 Add(Vector2 a, Vector2 b) { return Vector2{a.x + b.x, a.y + b.y}; }
Vector2 Scale(Vector2 value, float factor) { return Vector2{value.x * factor, value.y * factor}; }
float Length(Vector2 value) { return std::sqrt(value.x * value.x + value.y * value.y); }
Vector2 Normalize(Vector2 value) {
    const float length = Length(value);
    if (length <= 0.0001f) return Vector2{0.0f, 1.0f};
    return Vector2{value.x / length, value.y / length};
}
float AngleFromVelocity(Vector2 velocity) { return std::atan2(velocity.x, -velocity.y) * RAD2DEG; }
Vector2 VelocityFromAngle(float degrees, float speed) {
    const float radians = degrees * DEG2RAD;
    return Vector2{std::sin(radians) * speed, -std::cos(radians) * speed};
}
const char* PlayerLabel(int id) { return id == 0 ? "P1" : "P2"; }
}

Game::Game() {
    players[0] = Player(MakePlayerConfig(0, 0));
    players[1] = Player(MakePlayerConfig(1, 0));
}

void Game::Run() {
    InitWindow(kScreenWidth, kScreenHeight, "Starfall");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    audio.Initialize();
    while (!WindowShouldClose() && !exitRequested) {
        const float deltaTime = GetFrameTime();
        Update(deltaTime);
        Draw();
    }
    net.Close();
    discovery.Stop();
    audio.Shutdown();
    CloseWindow();
}

PlayerConfig Game::MakePlayerConfig(int id, int shipId) const {
    PlayerConfig config;
    config.id = id;
    config.ship = &GetShip(shipId);
    config.color = config.ship->hullColor;
    if (id == 0) {
        config.spawn = Vector2{kScreenWidth * 0.5f, kScreenHeight * 0.78f};
        config.aimAngle = 0.0f;
    } else {
        config.spawn = Vector2{kScreenWidth * 0.5f, kScreenHeight * 0.22f};
        config.aimAngle = 180.0f;
    }
    return config;
}

void Game::SpawnBullet(const Player& player) {
    const ShipDef& ship = player.Def();
    const Vector2 direction = player.AimDirection();
    const Vector2 muzzle = Add(player.GetPosition(), Scale(direction, player.GetRadius() + 6.0f));
    vsBullets.emplace_back(muzzle, Scale(direction, ship.bulletSpeed), player.GetColor(), player.GetId(),
                           ship.bulletRadius, ship.pierce);
}

InputCommand Game::DecodeButtons(uint8_t bits) {
    InputCommand command;
    command.rotateLeft = (bits & net::BitRotateLeft) != 0;
    command.rotateRight = (bits & net::BitRotateRight) != 0;
    command.thrust = (bits & net::BitThrust) != 0;
    command.reverse = (bits & net::BitReverse) != 0;
    command.fire = (bits & net::BitFire) != 0;
    return command;
}

uint8_t Game::EncodeButtons(const InputCommand& command) {
    uint8_t bits = 0;
    if (command.rotateLeft) bits |= net::BitRotateLeft;
    if (command.rotateRight) bits |= net::BitRotateRight;
    if (command.thrust) bits |= net::BitThrust;
    if (command.reverse) bits |= net::BitReverse;
    if (command.fire) bits |= net::BitFire;
    return bits;
}

InputCommand Game::LocalHostInput() const {
    return players[0].ReadKeyboard();
}

void Game::Update(float deltaTime) {
    switch (state) {
        case State::Menu: UpdateMenu(); break;
        case State::ShipSelect: UpdateShipSelect(); break;
        case State::PortEntry: UpdatePortEntry(); break;
        case State::HostLobby: UpdateHostLobby(deltaTime); break;
        case State::ClientLobby: UpdateClientLobby(deltaTime); break;
        case State::ManualEntry: UpdateManualEntry(); break;
        case State::Connecting: UpdateConnecting(deltaTime); break;
        case State::Playing:
            if (IsKeyPressed(KEY_ESCAPE)) {
                if (mode != Mode::Survival) SendPause(true);
                state = State::Paused;
                pauseSelection = 0;
                audio.SetMusicVolume(0.18f);
            } else if (mode == Mode::Survival) {
                UpdateSurvival(deltaTime);
            } else {
                UpdateVersus(deltaTime);
            }
            break;
        case State::Paused: UpdatePause(deltaTime); break;
        case State::Forfeiting:
            net.Update(deltaTime);
            forfeitTimer -= deltaTime;
            if (forfeitTimer <= 0.0f) ReturnToMenu();
            break;
        case State::GameOver:
            if (IsKeyPressed(KEY_ESCAPE)) exitRequested = true;
            else if (IsKeyPressed(KEY_ENTER)) {
                if (mode == Mode::Survival) { ResetSurvival(); state = State::Playing; }
                else ReturnToMenu();
            }
            break;
        case State::Disconnected:
            if (IsKeyPressed(KEY_ENTER)) ReturnToMenu();
            break;
    }
}

void Game::UpdatePause(float deltaTime) {
    if (mode != Mode::Survival) {
        net.Update(deltaTime);
        PumpReliable();
        if (state != State::Paused) return;
        std::vector<uint8_t> discarded;
        while (net.PollUnreliable(discarded)) {}
    }

    constexpr int optionCount = 2;
    if (IsKeyPressed(KEY_UP)) pauseSelection = (pauseSelection + optionCount - 1) % optionCount;
    if (IsKeyPressed(KEY_DOWN)) pauseSelection = (pauseSelection + 1) % optionCount;
    if (IsKeyPressed(KEY_ESCAPE)) { ResumeFromPause(); return; }
    if (IsKeyPressed(KEY_ENTER)) {
        if (pauseSelection == 0) {
            ResumeFromPause();
        } else if (mode == Mode::Survival) {
            ReturnToMenu();
        } else {
            ForfeitMatch();
        }
    }
}

void Game::ResumeFromPause() {
    if (mode != Mode::Survival) SendPause(false);
    state = State::Playing;
    audio.SetMusicVolume(0.5f);
}

void Game::SendPause(bool paused) {
    net::PauseMsg message;
    message.type = static_cast<uint8_t>(net::AppMsg::Pause);
    message.paused = paused ? 1 : 0;
    net.SendReliable(&message, sizeof(message));
}

void Game::ForfeitMatch() {
    net::DisconnectMsg message;
    message.type = static_cast<uint8_t>(net::AppMsg::Disconnect);
    message.reason = static_cast<uint8_t>(net::DisconnectReason::Forfeit);
    net.SendReliable(&message, sizeof(message));
    forfeitTimer = 0.6f;
    audio.SetMusicVolume(0.2f);
    state = State::Forfeiting;
}

void Game::UpdateMenu() {
    if (IsKeyPressed(KEY_ESCAPE)) exitRequested = true;
    if (IsKeyPressed(KEY_ONE)) {
        mode = Mode::Survival;
        shipSelectId = 0;
        state = State::ShipSelect;
    } else if (IsKeyPressed(KEY_TWO)) {
        mode = Mode::Host;
        shipSelectId = 0;
        state = State::ShipSelect;
    } else if (IsKeyPressed(KEY_THREE)) {
        mode = Mode::Client;
        shipSelectId = 0;
        state = State::ShipSelect;
    }
}

void Game::UpdateShipSelect() {
    if (IsKeyPressed(KEY_LEFT)) shipSelectId = (shipSelectId + kShipCount - 1) % kShipCount;
    if (IsKeyPressed(KEY_RIGHT)) shipSelectId = (shipSelectId + 1) % kShipCount;
    if (IsKeyPressed(KEY_ENTER)) {
        if (mode == Mode::Survival) {
            StartSurvival();
        } else if (mode == Mode::Host) {
            portText = "45801";
            state = State::PortEntry;
        } else {
            discovery.StartClient();
            lobbyLines.clear();
            lobbySelected = 0;
            state = State::ClientLobby;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) state = State::Menu;
}

void Game::UpdatePortEntry() {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= '0' && key <= '9' && portText.size() < 5) portText += static_cast<char>(key);
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !portText.empty()) portText.pop_back();
    if (IsKeyPressed(KEY_ENTER) && !portText.empty()) {
        const int value = std::atoi(portText.c_str());
        if (value >= 1024 && value <= 65535) StartHosting(static_cast<uint16_t>(value));
    }
    if (IsKeyPressed(KEY_ESCAPE)) state = State::Menu;
}

void Game::UpdateHostLobby(float deltaTime) {
    discovery.Update(deltaTime);
    net.Update(deltaTime);
    PumpReliable();
}

void Game::UpdateClientLobby(float deltaTime) {
    discovery.Update(deltaTime);
    net.Update(deltaTime);
    lobbyLines.clear();
    const auto& hosts = discovery.GetHosts();
    for (const auto& host : hosts) {
        lobbyLines.push_back(host.name + "   " + host.ip + ":" + std::to_string(host.port));
    }
    if (!hosts.empty()) {
        if (lobbySelected >= static_cast<int>(hosts.size())) lobbySelected = 0;
        if (IsKeyPressed(KEY_UP)) lobbySelected = (lobbySelected + static_cast<int>(hosts.size()) - 1) % static_cast<int>(hosts.size());
        if (IsKeyPressed(KEY_DOWN)) lobbySelected = (lobbySelected + 1) % static_cast<int>(hosts.size());
        if (IsKeyPressed(KEY_ENTER)) StartJoin(hosts[lobbySelected].ip, hosts[lobbySelected].port);
    } else {
        lobbySelected = 0;
    }
    if (IsKeyPressed(KEY_M)) {
        manualIp = "127.0.0.1";
        manualPort = "45801";
        manualEditingIp = true;
        state = State::ManualEntry;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        discovery.Stop();
        state = State::Menu;
    }
}

void Game::UpdateManualEntry() {
    int key = GetCharPressed();
    while (key > 0) {
        if (manualEditingIp) {
            if ((key >= '0' && key <= '9') || key == '.' || key == ':') {
                if (manualIp.size() < 20) manualIp += static_cast<char>(key);
            }
        } else {
            if (key >= '0' && key <= '9' && manualPort.size() < 5) manualPort += static_cast<char>(key);
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (manualEditingIp) { if (!manualIp.empty()) manualIp.pop_back(); }
        else { if (!manualPort.empty()) manualPort.pop_back(); }
    }
    if (IsKeyPressed(KEY_TAB)) manualEditingIp = !manualEditingIp;
    if (IsKeyPressed(KEY_ENTER)) {
        const int value = std::atoi(manualPort.c_str());
        if (value >= 1024 && value <= 65535) StartJoin(manualIp, static_cast<uint16_t>(value));
    }
    if (IsKeyPressed(KEY_ESCAPE)) state = State::ClientLobby;
}

void Game::UpdateConnecting(float deltaTime) {
    net.Update(deltaTime);
    SendHello();
    PumpReliable();
    if (net.GetStatus() == net::Status::Error || net.PeerTimedOut()) {
        net.Close();
        state = State::Disconnected;
        statusText = "Could not reach the host.";
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        net.Close();
        discovery.Stop();
        state = State::Menu;
    }
}

void Game::UpdateVersus(float deltaTime) {
    net.Update(deltaTime);
    PumpReliable();
    PumpUnreliable();

    if (mode == Mode::Host) {
        vsAccumulator += deltaTime;
        int steps = 0;
        while (vsAccumulator >= kStep && steps < 5) {
            HostStep();
            vsAccumulator -= kStep;
            ++steps;
        }
        SendSnapshot();
    } else {
        vsAccumulator += deltaTime;
        int steps = 0;
        while (vsAccumulator >= kStep && steps < 5) {
            ClientStep();
            vsAccumulator -= kStep;
            ++steps;
        }
        SendInput();
    }

    if (net.GetStatus() == net::Status::Error || net.PeerTimedOut()) {
        state = State::Disconnected;
        statusText = "Connection to opponent lost.";
        return;
    }
}

void Game::StartSurvival() {
    mode = Mode::Survival;
    localId = 0;
    endNote.clear();
    players[0] = Player(MakePlayerConfig(0, shipSelectId));
    players[1] = Player(MakePlayerConfig(1, 0));
    ResetSurvival();
    state = State::Playing;
    audio.PlayStart();
    audio.StartMusic();
    audio.SetMusicVolume(0.5f);
}

void Game::ResetSurvival() {
    players[0].Reset();
    bullets.clear();
    asteroids.clear();
    enemies.clear();
    spawnTimer = 0.45f;
    elapsedTime = 0.0f;
    score = 0;
}

void Game::StartHosting(uint16_t port) {
    mode = Mode::Host;
    localId = 0;
    hostShipId = shipSelectId;
    if (!net.Host(port)) {
        statusText = "Failed to open port " + std::to_string(port) + ". Try another port.";
        state = State::Menu;
        return;
    }
    discovery.StartHost("Starfall", port);
    statusText = "Waiting for a player to join...";
    state = State::HostLobby;
}

void Game::StartJoin(const std::string& ip, uint16_t port) {
    mode = Mode::Client;
    localId = 1;
    clientShipId = shipSelectId;
    if (!net.Join(ip, port)) {
        statusText = "Could not create socket to " + ip;
        state = State::ClientLobby;
        return;
    }
    discovery.Stop();
    statusText = "Connecting to " + ip + ":" + std::to_string(port) + "...";
    state = State::Connecting;
}

void Game::SendHello() {
    net::HelloMsg message;
    message.type = static_cast<uint8_t>(net::AppMsg::Hello);
    message.shipId = static_cast<uint8_t>(shipSelectId);
    std::strncpy(message.name, "Player", sizeof(message.name) - 1);
    net.SendReliable(&message, sizeof(message));
}

void Game::ReturnToMenu() {
    net.Close();
    discovery.Stop();
    mode = Mode::Survival;
    state = State::Menu;
}

void Game::PumpReliable() {
    std::vector<uint8_t> message;
    while (net.PollReliable(message)) {
        if (message.empty()) continue;
        const uint8_t type = message[0];
        if (type == static_cast<uint8_t>(net::AppMsg::Hello) && mode == Mode::Host && state == State::HostLobby) {
            if (message.size() >= sizeof(net::HelloMsg)) {
                net::HelloMsg hello;
                std::memcpy(&hello, message.data(), sizeof(hello));
                clientShipId = hello.shipId;
            }
            if (clientShipId >= kShipCount) clientShipId = 0;
            players[0] = Player(MakePlayerConfig(0, hostShipId));
            players[1] = Player(MakePlayerConfig(1, clientShipId));
            net::WelcomeMsg welcome;
            welcome.type = static_cast<uint8_t>(net::AppMsg::Welcome);
            welcome.playerId = 1;
            welcome.roundsToWin = static_cast<uint8_t>(kRoundsToWin);
            welcome.shipIds[0] = static_cast<uint8_t>(hostShipId);
            welcome.shipIds[1] = static_cast<uint8_t>(clientShipId);
            net.SendReliable(&welcome, sizeof(welcome));
            discovery.Stop();
            StartMatch();
            state = State::Playing;
        } else if (type == static_cast<uint8_t>(net::AppMsg::Welcome) && mode == Mode::Client) {
            if (message.size() >= sizeof(net::WelcomeMsg)) {
                net::WelcomeMsg welcome;
                std::memcpy(&welcome, message.data(), sizeof(welcome));
                localId = welcome.playerId;
                hostShipId = welcome.shipIds[0];
                clientShipId = welcome.shipIds[1];
            }
            if (hostShipId >= kShipCount) hostShipId = 0;
            if (clientShipId >= kShipCount) clientShipId = 0;
            players[0] = Player(MakePlayerConfig(0, hostShipId));
            players[1] = Player(MakePlayerConfig(1, clientShipId));
            StartMatch();
            state = State::Playing;
        } else if (type == static_cast<uint8_t>(net::AppMsg::Disconnect)) {
            uint8_t reason = static_cast<uint8_t>(net::DisconnectReason::Left);
            if (message.size() >= sizeof(net::DisconnectMsg)) {
                net::DisconnectMsg info;
                std::memcpy(&info, message.data(), sizeof(info));
                reason = info.reason;
            }
            if (reason == static_cast<uint8_t>(net::DisconnectReason::Forfeit)) {
                wins[localId] = kRoundsToWin;
                endNote = "OPPONENT FORFEITED";
                net.Close();
                discovery.Stop();
                audio.PlayGameOver();
                state = State::GameOver;
            } else {
                state = State::Disconnected;
                statusText = "Opponent left the match.";
            }
        } else if (type == static_cast<uint8_t>(net::AppMsg::Pause)) {
            if (message.size() >= sizeof(net::PauseMsg)) {
                net::PauseMsg info;
                std::memcpy(&info, message.data(), sizeof(info));
                if (info.paused != 0) {
                    if (state == State::Playing) {
                        pauseSelection = 0;
                        audio.SetMusicVolume(0.18f);
                        state = State::Paused;
                    }
                } else {
                    if (state == State::Paused) {
                        audio.SetMusicVolume(0.5f);
                        state = State::Playing;
                    }
                }
            }
        }
    }
}

void Game::PumpUnreliable() {
    std::vector<uint8_t> message;
    while (net.PollUnreliable(message)) {
        if (message.empty()) continue;
        const uint8_t type = message[0];
        if (type == static_cast<uint8_t>(net::AppMsg::Input) && mode == Mode::Host &&
            message.size() >= sizeof(net::InputMsg)) {
            net::InputMsg input;
            std::memcpy(&input, message.data(), sizeof(input));
            for (int i = 0; i < net::kInputRedundancy; ++i) {
                if (input.entries[i].tick > lastClientTickSeen) {
                    lastClientTickSeen = input.entries[i].tick;
                    latestClientButtons = input.entries[i].buttons;
                }
            }
        } else if (type == static_cast<uint8_t>(net::AppMsg::Snapshot) && mode == Mode::Client &&
                   message.size() >= sizeof(net::SnapshotMsg)) {
            net::SnapshotMsg snapshot;
            std::memcpy(&snapshot, message.data(), sizeof(snapshot));
            ApplySnapshot(snapshot);
        }
    }
}

void Game::StartMatch() {
    endNote.clear();
    pauseSelection = 0;
    wins[0] = 0;
    wins[1] = 0;
    serverTick = 0;
    clientTick = 0;
    lastClientTickSeen = 0;
    latestClientButtons = 0;
    inputHistory.clear();
    vsAccumulator = 0.0f;
    phase = net::MatchPhase::Playing;
    previousHealth[0] = players[0].GetHealth();
    previousHealth[1] = players[1].GetHealth();
    previousAsteroidCount = 0;
    ResetRound();
    audio.PlayStart();
    audio.StartMusic();
    audio.SetMusicVolume(0.5f);
}

void Game::ResetRound() {
    players[0].Reset();
    players[1].Reset();
    vsBullets.clear();
    remoteBullets.clear();
    vsAsteroids.clear();
    vsSpawnTimer = 2.0f;
    SpawnVersusAsteroids(3);
    phase = net::MatchPhase::Playing;
    previousHealth[0] = players[0].GetHealth();
    previousHealth[1] = players[1].GetHealth();
    previousAsteroidCount = static_cast<int>(vsAsteroids.size());
}

void Game::SpawnVersusAsteroids(int count) {
    for (int i = 0; i < count; ++i) {
        const float x = static_cast<float>(GetRandomValue(40, kScreenWidth - 40));
        const float radius = static_cast<float>(GetRandomValue(18, 30));
        const float speedY = static_cast<float>(GetRandomValue(70, 120));
        const float drift = static_cast<float>(GetRandomValue(-35, 35));
        vsAsteroids.emplace_back(Vector2{x, -radius}, Vector2{drift, speedY}, radius);
    }
}

void Game::HostStep() {
    const InputCommand hostInput = LocalHostInput();
    const InputCommand clientInput = DecodeButtons(latestClientButtons);
    players[0].Update(kStep, hostInput);
    players[1].Update(kStep, clientInput);
    players[0].UpdateRegen(kStep);
    players[1].UpdateRegen(kStep);

    if (hostInput.fire && players[0].TryShoot()) { SpawnBullet(players[0]); audio.PlayShoot(); }
    if (clientInput.fire && players[1].TryShoot()) { SpawnBullet(players[1]); audio.PlayShoot(); }

    vsSpawnTimer -= kStep;
    if (vsSpawnTimer <= 0.0f) {
        SpawnVersusAsteroids(1);
        vsSpawnTimer = 2.6f;
    }

    for (auto& bullet : vsBullets) bullet.Update(kStep);
    for (auto& asteroid : vsAsteroids) asteroid.Update(kStep);
    ResolveVersusCollisions();

    vsBullets.erase(std::remove_if(vsBullets.begin(), vsBullets.end(),
        [](const Bullet& bullet) { return bullet.IsOffScreen(); }), vsBullets.end());
    vsAsteroids.erase(std::remove_if(vsAsteroids.begin(), vsAsteroids.end(),
        [](const Asteroid& asteroid) { return asteroid.IsOffScreen(); }), vsAsteroids.end());

    if (phase == net::MatchPhase::Playing) {
        int dead = -1;
        if (players[0].GetHealth() <= 0) dead = 0;
        else if (players[1].GetHealth() <= 0) dead = 1;
        if (dead >= 0) {
            wins[1 - dead] += 1;
            phase = net::MatchPhase::RoundOver;
            phaseTimer = 2.0f;
            audio.PlayGameOver();
        }
    } else if (phase == net::MatchPhase::RoundOver) {
        phaseTimer -= kStep;
        if (phaseTimer <= 0.0f) {
            if (wins[0] >= kRoundsToWin || wins[1] >= kRoundsToWin) {
                phase = net::MatchPhase::MatchOver;
                audio.PlayGameOver();
                state = State::GameOver;
            } else {
                ResetRound();
            }
        }
    }
    ++serverTick;
}

void Game::ClientStep() {
    const InputCommand command = players[1].ReadKeyboard();
    ++clientTick;
    inputHistory.push_back({clientTick, command});
    players[localId].Update(kStep, command);
    if (command.fire && players[localId].TryShoot()) { SpawnBullet(players[localId]); audio.PlayShoot(); }
    for (auto& bullet : vsBullets) bullet.Update(kStep);
    vsBullets.erase(std::remove_if(vsBullets.begin(), vsBullets.end(),
        [](const Bullet& bullet) { return bullet.IsOffScreen(); }), vsBullets.end());
}

void Game::SendInput() {
    net::InputMsg message;
    message.type = static_cast<uint8_t>(net::AppMsg::Input);
    int count = 0;
    for (auto it = inputHistory.rbegin(); it != inputHistory.rend() && count < net::kInputRedundancy; ++it, ++count) {
        message.entries[count].tick = it->first;
        message.entries[count].buttons = EncodeButtons(it->second);
    }
    if (count == 0) {
        message.entries[0].tick = clientTick;
        message.entries[0].buttons = 0;
        count = 1;
    }
    for (int i = count; i < net::kInputRedundancy; ++i) message.entries[i] = message.entries[count - 1];
    net.SendUnreliable(&message, sizeof(message));
}

void Game::CaptureSnapshot(net::SnapshotMsg& snapshot) const {
    std::memset(&snapshot, 0, sizeof(snapshot));
    snapshot.type = static_cast<uint8_t>(net::AppMsg::Snapshot);
    snapshot.serverTick = serverTick;
    snapshot.ackedInputTick = lastClientTickSeen;
    snapshot.phase = static_cast<uint8_t>(phase);
    snapshot.wins[0] = static_cast<uint8_t>(wins[0]);
    snapshot.wins[1] = static_cast<uint8_t>(wins[1]);
    for (int i = 0; i < 2; ++i) {
        const Vector2 position = players[i].GetPosition();
        const Vector2 velocity = players[i].GetVelocity();
        const Vector2 knockback = players[i].GetKnockback();
        snapshot.players[i].x = position.x;
        snapshot.players[i].y = position.y;
        snapshot.players[i].aimAngle = players[i].GetAimAngle();
        snapshot.players[i].vx = velocity.x;
        snapshot.players[i].vy = velocity.y;
        snapshot.players[i].knockbackX = knockback.x;
        snapshot.players[i].knockbackY = knockback.y;
        snapshot.players[i].invulnerability = players[i].GetInvulnerability();
        snapshot.players[i].health = static_cast<int16_t>(players[i].GetHealth());
        uint8_t thrustBits = 0;
        if (players[i].GetThrottle() > 0.25f) thrustBits |= 1;
        if (players[i].IsReversing()) thrustBits |= 2;
        snapshot.players[i].thrusting = thrustBits;
    }
    int bulletCount = static_cast<int>(vsBullets.size());
    if (bulletCount > net::kMaxBullets) bulletCount = net::kMaxBullets;
    snapshot.bulletCount = static_cast<uint8_t>(bulletCount);
    for (int i = 0; i < bulletCount; ++i) {
        const Vector2 position = vsBullets[i].GetPosition();
        snapshot.bullets[i].x = position.x;
        snapshot.bullets[i].y = position.y;
        snapshot.bullets[i].angle = AngleFromVelocity(vsBullets[i].GetVelocity());
        snapshot.bullets[i].owner = static_cast<uint8_t>(vsBullets[i].GetOwner());
    }
    int asteroidCount = static_cast<int>(vsAsteroids.size());
    if (asteroidCount > net::kMaxAsteroids) asteroidCount = net::kMaxAsteroids;
    snapshot.asteroidCount = static_cast<uint8_t>(asteroidCount);
    for (int i = 0; i < asteroidCount; ++i) {
        const Vector2 position = vsAsteroids[i].GetPosition();
        snapshot.asteroids[i].x = position.x;
        snapshot.asteroids[i].y = position.y;
        snapshot.asteroids[i].radius = vsAsteroids[i].GetRadius();
        snapshot.asteroids[i].rotation = vsAsteroids[i].GetRotation();
    }
}

void Game::SendSnapshot() {
    net::SnapshotMsg snapshot;
    CaptureSnapshot(snapshot);
    net.SendUnreliable(&snapshot, sizeof(snapshot));
}

void Game::ApplySnapshot(const net::SnapshotMsg& snapshot) {
    const net::PlayerNet& remote = snapshot.players[0];
    players[0].SetPosition(Vector2{remote.x, remote.y});
    players[0].SetAimAngle(remote.aimAngle);
    players[0].SetVelocity(Vector2{remote.vx, remote.vy});
    players[0].SetKnockback(Vector2{remote.knockbackX, remote.knockbackY});
    players[0].SetHealth(remote.health);
    players[0].SetInvulnerability(remote.invulnerability);
    players[0].SetThrottle((remote.thrusting & 1) != 0 ? 1.0f : 0.0f);
    players[0].SetReversing((remote.thrusting & 2) != 0);

    const net::PlayerNet& own = snapshot.players[localId];
    players[localId].SetPosition(Vector2{own.x, own.y});
    players[localId].SetAimAngle(own.aimAngle);
    players[localId].SetVelocity(Vector2{own.vx, own.vy});
    players[localId].SetKnockback(Vector2{own.knockbackX, own.knockbackY});
    players[localId].SetHealth(own.health);
    players[localId].SetInvulnerability(own.invulnerability);

    while (!inputHistory.empty() && inputHistory.front().first <= snapshot.ackedInputTick) inputHistory.pop_front();
    for (const auto& entry : inputHistory) players[localId].Update(kStep, entry.second);

    remoteBullets.clear();
    for (int i = 0; i < snapshot.bulletCount; ++i) {
        if (snapshot.bullets[i].owner == static_cast<uint8_t>(localId)) continue;
        const int owner = snapshot.bullets[i].owner;
        remoteBullets.emplace_back(Vector2{snapshot.bullets[i].x, snapshot.bullets[i].y},
                                   VelocityFromAngle(snapshot.bullets[i].angle, players[owner].Def().bulletSpeed),
                                   players[owner].GetColor(), owner);
    }

    vsAsteroids.clear();
    for (int i = 0; i < snapshot.asteroidCount; ++i) {
        vsAsteroids.emplace_back(Vector2{snapshot.asteroids[i].x, snapshot.asteroids[i].y},
                                 Vector2{0.0f, 0.0f}, snapshot.asteroids[i].radius);
        vsAsteroids.back().SetRotation(snapshot.asteroids[i].rotation);
    }

    const int health0 = snapshot.players[0].health;
    const int health1 = snapshot.players[localId].health;
    if (health1 < previousHealth[1] || health0 < previousHealth[0]) audio.PlayHit();
    previousHealth[0] = health0;
    previousHealth[1] = health1;

    const int asteroidCount = snapshot.asteroidCount;
    if (asteroidCount < previousAsteroidCount) audio.PlayExplosion();
    previousAsteroidCount = asteroidCount;

    const auto newPhase = static_cast<net::MatchPhase>(snapshot.phase);
    if (newPhase != phase) {
        if (newPhase == net::MatchPhase::RoundOver || newPhase == net::MatchPhase::MatchOver) audio.PlayGameOver();
        if (newPhase == net::MatchPhase::MatchOver) state = State::GameOver;
        phase = newPhase;
    }
    wins[0] = snapshot.wins[0];
    wins[1] = snapshot.wins[1];
}

void Game::ResolveVersusCollisions() {
    std::vector<bool> bulletDead(vsBullets.size(), false);
    std::vector<bool> asteroidDead(vsAsteroids.size(), false);

    for (size_t i = 0; i < vsBullets.size(); ++i) {
        for (size_t j = 0; j < vsAsteroids.size(); ++j) {
            if (Collision::CirclesOverlap(vsBullets[i].GetPosition(), vsBullets[i].GetRadius(),
                                          vsAsteroids[j].GetPosition(), vsAsteroids[j].GetRadius())) {
                asteroidDead[j] = true;
                audio.PlayExplosion();
                if (!vsBullets[i].TryPierce()) bulletDead[i] = true;
                break;
            }
        }
    }

    for (size_t i = 0; i < vsBullets.size(); ++i) {
        if (bulletDead[i]) continue;
        const int target = vsBullets[i].GetOwner() == 0 ? 1 : 0;
        if (Collision::CirclesOverlap(vsBullets[i].GetPosition(), vsBullets[i].GetRadius(),
                                      players[target].GetPosition(), players[target].GetRadius())) {
            if (players[target].GetHealth() > 0) {
                players[target].TakeDamage();
                audio.PlayHit();
            }
            bulletDead[i] = true;
        }
    }

    for (size_t j = 0; j < vsAsteroids.size(); ++j) {
        if (asteroidDead[j]) continue;
        for (int t = 0; t < 2; ++t) {
            if (Collision::CirclesOverlap(players[t].GetPosition(), players[t].GetRadius(),
                                          vsAsteroids[j].GetPosition(), vsAsteroids[j].GetRadius())) {
                players[t].TakeDamage();
            }
        }
    }

    {
        const Vector2 difference = Vector2{players[0].GetPosition().x - players[1].GetPosition().x,
                                           players[0].GetPosition().y - players[1].GetPosition().y};
        const float distance = Length(difference);
        const float minimum = players[0].GetRadius() + players[1].GetRadius();
        if (distance < minimum && distance > 0.0001f) {
            const Vector2 normal = Normalize(difference);
            const float overlap = minimum - distance;
            players[0].SetPosition(Add(players[0].GetPosition(), Scale(normal, overlap * 0.5f)));
            players[1].SetPosition(Add(players[1].GetPosition(), Scale(normal, -overlap * 0.5f)));
            players[0].ApplyKnockback(Scale(normal, 240.0f));
            players[1].ApplyKnockback(Scale(normal, -240.0f));
        }
    }

    std::vector<Bullet> keptBullets;
    for (size_t i = 0; i < vsBullets.size(); ++i) if (!bulletDead[i]) keptBullets.push_back(vsBullets[i]);
    vsBullets.swap(keptBullets);

    std::vector<Asteroid> keptAsteroids;
    for (size_t j = 0; j < vsAsteroids.size(); ++j) if (!asteroidDead[j]) keptAsteroids.push_back(vsAsteroids[j]);
    vsAsteroids.swap(keptAsteroids);
}

void Game::UpdateSurvival(float deltaTime) {
    elapsedTime += deltaTime;
    const InputCommand input = players[0].ReadKeyboard();
    players[0].Update(deltaTime, input);
    players[0].UpdateRegen(deltaTime);
    if (input.fire && players[0].TryShoot()) {
        const ShipDef& ship = players[0].Def();
        const Vector2 direction = players[0].AimDirection();
        const Vector2 muzzle = Add(players[0].GetPosition(), Scale(direction, players[0].GetRadius() + 6.0f));
        bullets.emplace_back(muzzle, Scale(direction, ship.bulletSpeed), players[0].GetColor(), players[0].GetId(),
                             ship.bulletRadius, ship.pierce);
        audio.PlayShoot();
    }
    spawnTimer -= deltaTime;
    if (spawnTimer <= 0.0f) {
        const float x = static_cast<float>(GetRandomValue(28, kScreenWidth - 28));
        const float radius = static_cast<float>(GetRandomValue(17, 32));
        const float speedY = static_cast<float>(GetRandomValue(105, 180)) + std::min(75.0f, elapsedTime * 1.5f);
        const float drift = static_cast<float>(GetRandomValue(-45, 45));
        asteroids.emplace_back(Vector2{x, -radius}, Vector2{drift, speedY}, radius);
        const float difficulty = std::min(0.85f, elapsedTime * 0.003f);
        spawnTimer = std::max(0.48f, 1.15f - difficulty) * GetRandomValue(78, 122) / 100.0f;
    }
    for (auto& bullet : bullets) bullet.Update(deltaTime);
    for (auto& asteroid : asteroids) asteroid.Update(deltaTime);
    for (auto& enemy : enemies) enemy.Update(deltaTime);

    for (auto bullet = bullets.begin(); bullet != bullets.end();) {
        bool consumed = false;
        for (auto asteroid = asteroids.begin(); asteroid != asteroids.end(); ++asteroid) {
            if (Collision::CirclesOverlap(bullet->GetPosition(), bullet->GetRadius(),
                                          asteroid->GetPosition(), asteroid->GetRadius())) {
                asteroids.erase(asteroid);
                score += 100;
                audio.PlayExplosion();
                consumed = !bullet->TryPierce();
                break;
            }
        }
        if (consumed) bullet = bullets.erase(bullet);
        else ++bullet;
    }
    const int healthBefore = players[0].GetHealth();
    for (const auto& asteroid : asteroids) {
        if (Collision::CirclesOverlap(players[0].GetPosition(), players[0].GetRadius(),
                                      asteroid.GetPosition(), asteroid.GetRadius())) players[0].TakeDamage();
    }
    if (players[0].GetHealth() < healthBefore) audio.PlayHit();

    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& bullet) { return bullet.IsOffScreen(); }), bullets.end());
    asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(),
        [](const Asteroid& asteroid) { return asteroid.IsOffScreen(); }), asteroids.end());

    if (players[0].GetHealth() <= 0) { state = State::GameOver; audio.PlayGameOver(); audio.SetMusicVolume(0.2f); }
}

void Game::DrawBackground() const {
    ClearBackground(Color{5, 9, 24, 255});
    for (int i = 0; i < 75; ++i) {
        const int x = (i * 137 + 41) % kScreenWidth;
        const int y = (i * 223 + 17) % kScreenHeight;
        const int radius = (i % 9 == 0) ? 2 : 1;
        const unsigned char alpha = static_cast<unsigned char>(105 + (i * 31) % 145);
        DrawCircle(x, y, static_cast<float>(radius), Color{180, 211, 255, alpha});
    }
}

void Game::DrawSurvival() const {
    for (const auto& bullet : bullets) bullet.Draw();
    for (const auto& asteroid : asteroids) asteroid.Draw();
    for (const auto& enemy : enemies) enemy.Draw();
    players[0].Draw();
}

void Game::DrawVersusField() const {
    for (const auto& bullet : vsBullets) bullet.Draw();
    for (const auto& bullet : remoteBullets) bullet.Draw();
    for (const auto& asteroid : vsAsteroids) asteroid.Draw();
    players[0].Draw();
    players[1].Draw();
    ui.DrawVersusHud(players[0].GetHealth(), players[0].GetMaxHealth(), players[0].GetShipName(),
                     players[1].GetHealth(), players[1].GetMaxHealth(), players[1].GetShipName(),
                     wins[0], wins[1]);
}

void Game::Draw() const {
    BeginDrawing();
    DrawBackground();
    switch (state) {
        case State::Menu: ui.DrawMenu(); break;
        case State::ShipSelect: ui.DrawShipSelect(shipSelectId); break;
        case State::PortEntry: ui.DrawPortEntry(portText); break;
        case State::HostLobby:
            ui.DrawHostLobby(net::GetLocalIp() + ":" + portText, statusText);
            break;
        case State::ClientLobby: ui.DrawClientLobby(lobbyLines, lobbySelected); break;
        case State::ManualEntry: ui.DrawManualEntry(manualIp, manualPort, manualEditingIp); break;
        case State::Connecting: ui.DrawConnecting(statusText); break;
        case State::Playing:
            if (mode == Mode::Survival) {
                DrawSurvival();
                ui.DrawHud(score, players[0].GetHealth(), players[0].GetMaxHealth());
            } else {
                DrawVersusField();
                if (phase == net::MatchPhase::RoundOver) ui.DrawBanner("ROUND OVER");
            }
            break;
        case State::Paused:
            if (mode == Mode::Survival) {
                DrawSurvival();
                ui.DrawHud(score, players[0].GetHealth(), players[0].GetMaxHealth());
            } else {
                DrawVersusField();
            }
            ui.DrawPause(pauseSelection, mode != Mode::Survival);
            break;
        case State::Forfeiting:
            if (mode == Mode::Survival) DrawSurvival();
            else DrawVersusField();
            ui.DrawBanner("FORFEITED - RETURNING TO MENU");
            break;
        case State::GameOver:
            if (mode == Mode::Survival) {
                DrawSurvival();
                ui.DrawGameOver(score);
            } else {
                DrawVersusField();
                const int winner = wins[0] >= kRoundsToWin ? 0 : 1;
                ui.DrawOverlay(std::string(PlayerLabel(winner)) + " WINS", endNote);
            }
            break;
        case State::Disconnected: ui.DrawDisconnected(statusText); break;
    }
    EndDrawing();
}
