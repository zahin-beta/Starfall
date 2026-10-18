#include "UI.h"
#include "ShipCatalog.h"
#include "ShipRenderer.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {
void Centered(const std::string& text, int y, int fontSize, Color color) {
    const char* value = text.c_str();
    DrawText(value, (GetScreenWidth() - MeasureText(value, fontSize)) / 2, y, fontSize, color);
}

float Clamp01(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

void HullSquares(int health, int maxHealth, Color filled, int startX, int y) {
    for (int i = 0; i < maxHealth; ++i) {
        const Color color = i < health ? filled : Color{73, 79, 96, 255};
        DrawRectangle(startX + i * 31, y, 22, 18, color);
    }
}

void StatBar(const char* label, const std::string& value, float normalized, int x, int y, Color fill) {
    DrawText(label, x, y, 17, LIGHTGRAY);
    DrawText(value.c_str(), x + 105, y, 17, RAYWHITE);
    const int barX = x + 165;
    DrawRectangle(barX, y + 3, 165, 13, Color{36, 44, 66, 255});
    DrawRectangle(barX, y + 3, static_cast<int>(165.0f * Clamp01(normalized)), 13, fill);
}

float RateNormalized(float fireInterval) {
    const float rate = 1.0f / fireInterval;
    return Clamp01((rate - 1.80f) / (6.70f - 1.80f));
}
}

void UI::DrawMenu() const {
    Centered("STARFALL", 150, 64, RAYWHITE);
    Centered("NETWORKED SPACE BATTLE", 224, 20, SKYBLUE);
    Centered("1   Survival", 315, 26, GOLD);
    Centered("2   Host LAN Match", 360, 26, SKYBLUE);
    Centered("3   Join LAN Match", 405, 26, SKYBLUE);
    Centered("Rotate A / D    Thrust W    Reverse S    Fire Space", 475, 20, LIGHTGRAY);
    Centered("Press 1, 2 or 3 to pick a mode, then choose your ship", 530, 22, RAYWHITE);
}

void UI::DrawShipSelect(int selected) const {
    const ShipDef* ships = GetAllShips();
    const ShipDef& ship = ships[selected];

    Centered("SELECT YOUR SHIP", 78, 44, GOLD);
    Centered(ship.name, 148, 34, Color{ship.hullColor.r, ship.hullColor.g, ship.hullColor.b, 255});

    const float bob = static_cast<float>(GetScreenHeight()) * 0.42f + std::sin(GetTime() * 2.0f) * 6.0f;
    ShipRenderer::DrawShip(ship, Vector2{static_cast<float>(GetScreenWidth()) * 0.26f, bob},
                           std::sin(GetTime() * 0.9f) * 14.0f, 34.0f, 0.55f, false, false);

    const int columnX = static_cast<int>(GetScreenWidth()) * 0.52f;
    const Color fill = Color{ship.hullColor.r, ship.hullColor.g, ship.hullColor.b, 255};
    StatBar("HULL", std::to_string(ship.health), (ship.health - 2) / 3.0f, columnX, 230, fill);
    StatBar("SPEED", "", Clamp01((ship.maxSpeed - 250.0f) / 230.0f), columnX, 268, fill);
    StatBar("TURN", "", Clamp01((ship.turnSpeed - 150.0f) / 190.0f), columnX, 306, fill);
    StatBar("FIRE RATE", "", RateNormalized(ship.fireInterval), columnX, 344, fill);
    StatBar("RANGE", "", Clamp01((ship.bulletSpeed - 620.0f) / 430.0f), columnX, 382, fill);

    DrawText(ship.passiveName, columnX, 438, 22, GOLD);
    DrawText(ship.blurb, columnX, 472, 16, RAYWHITE);

    const int centre = GetScreenWidth() / 2;
    for (int i = 0; i < kShipCount; ++i) {
        const Color color = (i == selected) ? fill : Color{73, 79, 96, 255};
        DrawCircle(centre - 58 + i * 23, 566, 7.0f, color);
    }

    Centered("LEFT / RIGHT choose    ENTER confirm    ESC back", 618, 20, LIGHTGRAY);
}

void UI::DrawHud(int score, int health, int maxHealth) const {
    DrawRectangle(0, 0, GetScreenWidth(), 58, Color{8, 14, 31, 205});
    DrawText(TextFormat("SCORE  %06i", score), 22, 17, 22, RAYWHITE);
    DrawText("HULL", GetScreenWidth() - 178 - (maxHealth - 3) * 31, 18, 20, LIGHTGRAY);
    HullSquares(health, maxHealth, Color{255, 91, 112, 255},
                GetScreenWidth() - 115 - (maxHealth - 3) * 31, 20);
}

void UI::DrawGameOver(int score) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 190});
    Centered("SHIP LOST", 235, 52, Color{255, 100, 119, 255});
    Centered(TextFormat("FINAL SCORE  %06i", score), 320, 26, RAYWHITE);
    Centered("Press ENTER to try again", 410, 24, GOLD);
    Centered("Press ESC to exit", 455, 20, LIGHTGRAY);
}

void UI::DrawPortEntry(const std::string& text) const {
    Centered("HOST LAN MATCH", 160, 44, GOLD);
    Centered("Game port: " + text + "|", 280, 32, SKYBLUE);
    Centered("Type digits    ENTER start    BACKSPACE delete    ESC back", 360, 20, LIGHTGRAY);
    Centered("Valid range 1024 - 65535", 405, 18, GRAY);
}

void UI::DrawHostLobby(const std::string& address, const std::string& detail) const {
    Centered("HOSTING LAN MATCH", 150, 44, GOLD);
    Centered("Your address: " + address, 235, 26, SKYBLUE);
    Centered(detail, 315, 24, RAYWHITE);
    Centered("Waiting for another player...", 365, 22, LIGHTGRAY);
    Centered("Press ESC to cancel", 455, 20, GRAY);
}

void UI::DrawClientLobby(const std::vector<std::string>& items, int selected) const {
    Centered("JOIN LAN MATCH", 150, 44, GOLD);
    if (items.empty()) {
        Centered("Searching for hosts on this network...", 250, 24, LIGHTGRAY);
        Centered("Press M to enter an IP manually", 300, 20, GRAY);
    } else {
        for (size_t i = 0; i < items.size(); ++i) {
            const Color color = (static_cast<int>(i) == selected) ? GOLD : RAYWHITE;
            Centered(items[i], 245 + static_cast<int>(i) * 34, 24, color);
        }
    }
    Centered("UP / DOWN select    ENTER join    M manual    ESC back", 585, 20, LIGHTGRAY);
}

void UI::DrawManualEntry(const std::string& ip, const std::string& port, bool editingIp) const {
    Centered("DIRECT CONNECT", 160, 44, GOLD);
    Centered(std::string("IP:   ") + ip + (editingIp ? "|" : ""), 270, 30, editingIp ? GOLD : RAYWHITE);
    Centered(std::string("PORT: ") + port + (editingIp ? "" : "|"), 325, 30, editingIp ? RAYWHITE : GOLD);
    Centered("ENTER connect    TAB switch field    ESC back", 430, 20, LIGHTGRAY);
}

void UI::DrawConnecting(const std::string& detail) const {
    Centered("CONNECTING", 250, 52, SKYBLUE);
    Centered(detail, 335, 24, LIGHTGRAY);
    Centered("Press ESC to cancel", 415, 20, GRAY);
}

void UI::DrawVersusHud(int healthP1, int maxHealthP1, const char* nameP1,
                       int healthP2, int maxHealthP2, const char* nameP2,
                       int winsP1, int winsP2) const {
    const int width = GetScreenWidth();
    DrawRectangle(0, 0, width, 58, Color{8, 14, 31, 205});

    const Color p1 = Color{77, 210, 255, 255};
    const Color p2 = Color{255, 150, 60, 255};
    DrawText("P1", 22, 8, 20, p1);
    DrawText(nameP1, 22, 32, 15, LIGHTGRAY);
    HullSquares(healthP1, maxHealthP1, p1, 118, 16);

    const int p2NameWidth = MeasureText(nameP2, 15);
    DrawText("P2", width - 22 - MeasureText("P2", 20), 8, 20, p2);
    DrawText(nameP2, width - 22 - p2NameWidth, 32, 15, LIGHTGRAY);
    HullSquares(healthP2, maxHealthP2, p2, width - 52 - (maxHealthP2 - 1) * 31, 16);

    const int centre = width / 2;
    for (int i = 0; i < 2; ++i) {
        DrawCircle(centre - 55 + i * 18, 26, 7.0f, i < winsP1 ? p1 : Color{73, 79, 96, 255});
        DrawCircle(centre + 55 - i * 18, 26, 7.0f, i < winsP2 ? p2 : Color{73, 79, 96, 255});
    }
    DrawText("FIRST TO 2", centre - MeasureText("FIRST TO 2", 16) / 2, 36, 16, LIGHTGRAY);
}

void UI::DrawOverlay(const std::string& title, const std::string& subtitle) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 180});
    Centered(title, 250, 52, Color{255, 214, 90, 255});
    Centered(subtitle, 335, 26, RAYWHITE);
    Centered("Press ENTER for menu", 415, 22, LIGHTGRAY);
}

void UI::DrawPause(int selected, bool versus) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 205});
    Centered("PAUSED", 235, 56, GOLD);

    const char* options[2] = {"RESUME", versus ? "FORFEIT MATCH" : "BACK TO MENU"};
    for (int i = 0; i < 2; ++i) {
        const bool active = (i == selected);
        std::string label = active ? (">  " + std::string(options[i]) + "  <") : std::string("     ") + options[i];
        Centered(label, 345 + i * 50, 30, active ? RAYWHITE : Color{140, 148, 168, 255});
    }

    Centered("UP / DOWN select    ENTER confirm    ESC resume", 600, 20, LIGHTGRAY);
}

void UI::DrawDisconnected(const std::string& reason) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 195});
    Centered("CONNECTION LOST", 250, 50, Color{255, 100, 119, 255});
    Centered(reason, 335, 24, RAYWHITE);
    Centered("Press ENTER for menu", 415, 22, LIGHTGRAY);
}

void UI::DrawBanner(const std::string& text) const {
    const char* value = text.c_str();
    const int size = 40;
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    DrawRectangle(0, height / 2 - 45, width, 90, Color{8, 14, 31, 160});
    DrawText(value, (width - MeasureText(value, size)) / 2, height / 2 - size / 2, size, GOLD);
}
