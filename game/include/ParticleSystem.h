#pragma once
#include "raylib.h"
#include <vector>

class ParticleSystem {
public:
    void Update(float deltaTime);
    void Draw() const;
    void DrawGlow() const;
    void Clear();
    void EmitThruster(Vector2 position, Vector2 direction, Color color, float strength);
    void EmitExplosion(Vector2 position, Color color, float scale);
    void EmitHitSpark(Vector2 position, Color color);
    void EmitCelebration(Vector2 position, Color color);

private:
    struct Particle {
        Vector2 position{};
        Vector2 velocity{};
        float life = 0.0f;
        float maxLife = 0.0f;
        float size = 1.0f;
        Color color{255, 255, 255, 255};
        bool alive = false;
    };
    void Spawn(const Particle& particle);

    std::vector<Particle> particles;
    std::size_t nextSlot = 0;
    static constexpr std::size_t kMaxParticles = 640;
};
