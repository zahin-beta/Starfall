#include "UI.h"
#include "ShipCatalog.h"
#include "ShipRenderer.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {
constexpr Color kPanel{8, 14, 31, 214};
constexpr Color kPanelEdge{38, 68, 118, 255};
constexpr Color kAccent{96, 200, 255, 255};
constexpr Color kGold{255, 210, 88, 255};
constexpr Color kText{232, 240, 255, 255};
constexpr Color kMuted{142, 154, 180, 255};
constexpr Color kDanger{255, 100, 119, 255};
constexpr Color kEmpty{30, 38, 58, 255};

void Centered(const std::string& text, int y, int fontSize, Color color) {
    const char* value = text.c_str();
    DrawText(value, (GetScreenWidth() - MeasureText(value, fontSize)) / 2, y, fontSize, color);
}

float Clamp01(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

void Panel(int x, int y, int w, int h) {
    DrawRectangle(x, y, w, h, kPanel);
    DrawRectangleLines(x, y, w, h, kPanelEdge);
    DrawRectangle(x + 1, y + 1, w - 2, 2, kAccent);
}

void HullPips(int health, int maxHealth, Color filled, int startX, int y) {
    for (int i = 0; i < maxHealth; ++i) {
        const int x = startX + i * 26;
        if (i < health) {
            DrawRectangle(x, y, 20, 14, filled);
            DrawRectangle(x, y, 20, 3, Color{255, 255, 255, 70});
        } else {
            DrawRectangle(x, y, 20, 14, kEmpty);
            DrawRectangleLines(x, y, 20, 14, Color{64, 74, 98, 255});
        }
    }
}

void StatBar(const char* label, const std::string& value, float normalized, int x, int y, Color fill) {
    DrawText(label, x, y, 17, kMuted);
    DrawText(value.c_str(), x + 105, y, 17, kText);
    const int barX = x + 165;
    DrawRectangle(barX, y + 3, 165, 13, kEmpty);
    DrawRectangle(barX, y + 3, static_cast<int>(165.0f * Clamp01(normalized)), 13, fill);
    DrawRectangleLines(barX, y + 3, 165, 13, Color{52, 64, 92, 255});
}

float RateNormalized(float fireInterval) {
    const float rate = 1.0f / fireInterval;
    return Clamp01((rate - 1.80f) / (6.70f - 1.80f));
}
}

void UI::DrawMenu() const {
    const int width = GetScreenWidth();
    DrawText("STARFALL", (width - MeasureText("STARFALL", 68)) / 2, 130, 68, kText);
    const int titleWidth = MeasureText("STARFALL", 68);
    DrawRectangle((width - titleWidth) / 2, 202, titleWidth, 3, kAccent);
    Centered("NETWORKED SPACE BATTLE", 220, 18, kAccent);

    Panel(width / 2 - 190, 294, 380, 150);
    Centered("[ 1 ]   SURVIVAL", 316, 26, kGold);
    Centered("[ 2 ]   HOST LAN MATCH", 354, 26, kText);
    Centered("[ 3 ]   JOIN LAN MATCH", 392, 26, kText);

    Centered("ROTATE A / D     THRUST W     REVERSE S     FIRE SPACE", 492, 18, kMuted);
    Centered("PRESS 1, 2 OR 3 TO PICK A MODE, THEN CHOOSE YOUR SHIP", 524, 18, kMuted);
}

void UI::DrawShipSelect(int selected) const {
    const ShipDef* ships = GetAllShips();
    const ShipDef& ship = ships[selected];
    const Color fill = Color{ship.hullColor.r, ship.hullColor.g, ship.hullColor.b, 255};

    Centered("SELECT YOUR SHIP", 56, 40, kText);
    Centered(ship.name, 108, 30, fill);

    Panel(28, 176, 300, 384);
    const float bob = static_cast<float>(GetScreenHeight()) * 0.42f + std::sin(GetTime() * 2.0f) * 6.0f;
    ShipRenderer::DrawShip(ship, Vector2{static_cast<float>(GetScreenWidth()) * 0.185f, bob},
                           std::sin(GetTime() * 0.9f) * 14.0f, 34.0f, 0.55f, false, false);

    const int columnX = static_cast<int>(GetScreenWidth()) * 0.52f;
    StatBar("HULL", std::to_string(ship.health), (ship.health - 2) / 3.0f, columnX, 230, fill);
    StatBar("SPEED", "", Clamp01((ship.maxSpeed - 250.0f) / 230.0f), columnX, 268, fill);
    StatBar("TURN", "", Clamp01((ship.turnSpeed - 150.0f) / 190.0f), columnX, 306, fill);
    StatBar("FIRE RATE", "", RateNormalized(ship.fireInterval), columnX, 344, fill);
    StatBar("RANGE", "", Clamp01((ship.bulletSpeed - 620.0f) / 430.0f), columnX, 382, fill);

    DrawRectangle(columnX, 432, 165, 2, fill);
    DrawText(ship.passiveName, columnX, 444, 22, kGold);
    DrawText(ship.blurb, columnX, 476, 16, kText);

    const int centre = GetScreenWidth() / 2;
    for (int i = 0; i < kShipCount; ++i) {
        const Color color = (i == selected) ? fill : Color{73, 79, 96, 255};
        DrawCircle(centre - 58 + i * 23, 588, 7.0f, color);
    }

    Centered("LEFT / RIGHT choose    ENTER confirm    ESC back", 628, 20, kMuted);
}

void UI::DrawHud(int score, int health, int maxHealth) const {
    const int width = GetScreenWidth();
    DrawRectangle(0, 0, width, 56, kPanel);
    DrawRectangle(0, 54, width, 2, kPanelEdge);
    DrawText("SURVIVAL", 22, 8, 14, kAccent);
    DrawText(TextFormat("%06i", score), 22, 24, 24, kText);

    const int pipWidth = maxHealth * 26;
    const int pipX = width - pipWidth - 32;
    DrawText("HULL", pipX - 62, 20, 18, kMuted);
    HullPips(health, maxHealth, kDanger, pipX, 21);
}

void UI::DrawGameOver(int score) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 195});
    Panel(GetScreenWidth() / 2 - 240, 205, 480, 285);
    Centered("SHIP LOST", 235, 52, kDanger);
    Centered(TextFormat("FINAL SCORE  %06i", score), 320, 26, kText);
    Centered("Press ENTER to try again", 410, 22, kGold);
    Centered("Press ESC to exit", 452, 18, kMuted);
}

void UI::DrawPortEntry(const std::string& text) const {
    Centered("HOST LAN MATCH", 150, 44, kText);
    Panel(GetScreenWidth() / 2 - 190, 250, 380, 100);
    Centered("GAME PORT   " + text + "|", 280, 30, kAccent);
    Centered("TYPE DIGITS   ENTER START   BACKSPACE DELETE   ESC BACK", 402, 18, kMuted);
    Centered("VALID RANGE 1024 - 65535", 438, 18, kMuted);
}

void UI::DrawHostLobby(const std::string& address, const std::string& detail) const {
    Centered("HOSTING LAN MATCH", 140, 44, kText);
    Panel(GetScreenWidth() / 2 - 230, 220, 460, 150);
    Centered("YOUR ADDRESS   " + address, 246, 24, kAccent);
    Centered(detail, 300, 22, kText);
    Centered("WAITING FOR ANOTHER PLAYER...", 336, 18, kMuted);
    Centered("Press ESC to cancel", 452, 18, kMuted);
}

void UI::DrawClientLobby(const std::vector<std::string>& items, int selected) const {
    Centered("JOIN LAN MATCH", 130, 44, kText);
    Panel(GetScreenWidth() / 2 - 260, 215, 520, 230);
    if (items.empty()) {
        Centered("SEARCHING FOR HOSTS ON THIS NETWORK...", 262, 22, kMuted);
        Centered("PRESS M TO ENTER AN IP MANUALLY", 306, 18, kMuted);
    } else {
        for (size_t i = 0; i < items.size(); ++i) {
            const bool active = (static_cast<int>(i) == selected);
            const Color color = active ? kGold : kMuted;
            const std::string label = active ? (">  " + items[i] + "  <") : items[i];
            Centered(label, 248 + static_cast<int>(i) * 34, 22, color);
        }
    }
    Centered("UP / DOWN select    ENTER join    M manual    ESC back", 588, 18, kMuted);
}

void UI::DrawManualEntry(const std::string& ip, const std::string& port, bool editingIp) const {
    Centered("DIRECT CONNECT", 160, 44, kText);
    Panel(GetScreenWidth() / 2 - 230, 245, 460, 125);
    Centered(std::string("IP     ") + ip + (editingIp ? "|" : ""), 272, 30, editingIp ? kGold : kText);
    Centered(std::string("PORT   ") + port + (editingIp ? "" : "|"), 322, 30, editingIp ? kText : kGold);
    Centered("ENTER connect    TAB switch field    ESC back", 428, 18, kMuted);
}

void UI::DrawConnecting(const std::string& detail) const {
    Centered("CONNECTING", 250, 52, kAccent);
    Centered(detail, 335, 24, kMuted);
    Centered("Press ESC to cancel", 415, 18, kMuted);
}

void UI::DrawVersusHud(int healthP1, int maxHealthP1, const char* nameP1,
                       int healthP2, int maxHealthP2, const char* nameP2,
                       int winsP1, int winsP2) const {
    const int width = GetScreenWidth();
    DrawRectangle(0, 0, width, 56, kPanel);
    DrawRectangle(0, 54, width, 2, kPanelEdge);

    const Color p1 = Color{77, 210, 255, 255};
    const Color p2 = Color{255, 150, 60, 255};

    DrawText("P1", 22, 6, 18, p1);
    DrawText(nameP1, 22, 29, 14, kMuted);
    HullPips(healthP1, maxHealthP1, p1, 110, 19);

    const int p2NameWidth = MeasureText(nameP2, 14);
    DrawText("P2", width - 22 - MeasureText("P2", 18), 6, 18, p2);
    DrawText(nameP2, width - 22 - p2NameWidth, 29, 14, kMuted);
    const int p2Pips = maxHealthP2 * 26;
    HullPips(healthP2, maxHealthP2, p2, width - 32 - p2Pips, 19);

    const int centre = width / 2;
    for (int i = 0; i < 2; ++i) {
        DrawCircle(centre - 55 + i * 18, 22, 8.0f, i < winsP1 ? p1 : kEmpty);
        DrawCircle(centre + 55 - i * 18, 22, 8.0f, i < winsP2 ? p2 : kEmpty);
    }
    DrawText("FIRST TO 2", centre - MeasureText("FIRST TO 2", 15) / 2, 38, 15, kMuted);
}

void UI::DrawOverlay(const std::string& title, const std::string& subtitle) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 185});
    Panel(GetScreenWidth() / 2 - 240, 225, 480, 240);
    Centered(title, 258, 52, kGold);
    Centered(subtitle, 338, 26, kText);
    Centered("Press ENTER for menu", 415, 20, kMuted);
}

void UI::DrawPause(int selected, bool versus) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 205});
    Panel(GetScreenWidth() / 2 - 200, 205, 400, 240);
    Centered("PAUSED", 232, 52, kText);

    const char* options[2] = {"RESUME", versus ? "FORFEIT MATCH" : "BACK TO MENU"};
    for (int i = 0; i < 2; ++i) {
        const bool active = (i == selected);
        const std::string label = active ? (">  " + std::string(options[i]) + "  <") : std::string("     ") + options[i];
        Centered(label, 330 + i * 50, 28, active ? kGold : kMuted);
    }

    Centered("UP / DOWN select    ENTER confirm    ESC resume", 590, 18, kMuted);
}

void UI::DrawDisconnected(const std::string& reason) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 200});
    Panel(GetScreenWidth() / 2 - 240, 225, 480, 240);
    Centered("CONNECTION LOST", 258, 48, kDanger);
    Centered(reason, 338, 24, kText);
    Centered("Press ENTER for menu", 418, 20, kMuted);
}

void UI::DrawBanner(const std::string& text) const {
    const char* value = text.c_str();
    const int size = 40;
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    DrawRectangle(0, height / 2 - 45, width, 90, Color{8, 14, 31, 170});
    DrawRectangle(0, height / 2 - 45, width, 2, kAccent);
    DrawRectangle(0, height / 2 + 43, width, 2, kAccent);
    DrawText(value, (width - MeasureText(value, size)) / 2, height / 2 - size / 2, size, kGold);
}
