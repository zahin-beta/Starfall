#pragma once
#include <string>
#include <vector>

class UI {
public:
    void DrawMenu() const;
    void DrawShipSelect(int selected) const;
    void DrawHud(int score, int health, int maxHealth) const;
    void DrawGameOver(int score) const;
    void DrawPortEntry(const std::string& text) const;
    void DrawHostLobby(const std::string& address, const std::string& detail) const;
    void DrawClientLobby(const std::vector<std::string>& items, int selected) const;
    void DrawManualEntry(const std::string& ip, const std::string& port, bool editingIp) const;
    void DrawConnecting(const std::string& detail) const;
    void DrawVersusHud(int healthP1, int maxHealthP1, const char* nameP1,
                       int healthP2, int maxHealthP2, const char* nameP2,
                       int winsP1, int winsP2) const;
    void DrawOverlay(const std::string& title, const std::string& subtitle) const;
    void DrawPause(int selected, bool versus) const;
    void DrawDisconnected(const std::string& reason) const;
    void DrawBanner(const std::string& text) const;
};
