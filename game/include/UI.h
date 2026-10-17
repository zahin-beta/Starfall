#pragma once
class UI {
public:
    void DrawMenu() const;
    void DrawHud(int score, int health) const;
    void DrawGameOver(int score) const;
};
