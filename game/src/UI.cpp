#include "UI.h"
#include "raylib.h"
namespace {
void DrawCentered(const char* text, int y, int fontSize, Color color) {
    DrawText(text, (GetScreenWidth() - MeasureText(text, fontSize)) / 2, y, fontSize, color);
}
}
void UI::DrawMenu() const {
    DrawCentered("STARFALL", 190, 64, RAYWHITE);
    DrawCentered("A SPACE SURVIVAL GAME", 264, 20, SKYBLUE);
    DrawCentered("Move: WASD or Arrow Keys", 365, 22, LIGHTGRAY);
    DrawCentered("Shoot: Space", 400, 22, LIGHTGRAY);
    DrawCentered("Press ENTER to launch", 490, 24, GOLD);
}
void UI::DrawHud(int score, int health) const {
    DrawRectangle(0, 0, GetScreenWidth(), 58, Color{8, 14, 31, 205});
    DrawText(TextFormat("SCORE  %06i", score), 22, 17, 22, RAYWHITE);
    DrawText("HULL", GetScreenWidth() - 178, 18, 20, LIGHTGRAY);
    for (int i = 0; i < 3; ++i) {
        const Color color = i < health ? Color{255, 91, 112, 255} : Color{73, 79, 96, 255};
        DrawRectangle(GetScreenWidth() - 115 + i * 31, 20, 22, 18, color);
    }
}
void UI::DrawGameOver(int score) const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{3, 6, 18, 190});
    DrawCentered("SHIP LOST", 235, 52, Color{255, 100, 119, 255});
    DrawCentered(TextFormat("FINAL SCORE  %06i", score), 320, 26, RAYWHITE);
    DrawCentered("Press ENTER to try again", 410, 24, GOLD);
    DrawCentered("Press ESC to exit", 455, 20, LIGHTGRAY);
}
