#include "ParticleSystem.h"
#include <cmath>
#include <random>

namespace {
std::mt19937& Rng() {
    static std::mt19937 rng(0x57A41Fu);
    return rng;
}

float Range(float min, float max) {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(Rng());
}

Color Blend(Color a, Color b, float t) {
    return Color{
        static_cast<unsigned char>(a.r + (b.r - a.r) * t),
        static_cast<unsigned char>(a.g + (b.g - a.g) * t),
        static_cast<unsigned char>(a.b + (b.b - a.b) * t),
        static_cast<unsigned char>(a.a + (b.a - a.a) * t)};
}
}

void ParticleSystem::Spawn(const Particle& particle) {
    if (particles.size() < kMaxParticles) {
        particles.push_back(particle);
        return;
    }
    particles[nextSlot] = particle;
    nextSlot = (nextSlot + 1) % kMaxParticles;
}

void ParticleSystem::Update(float deltaTime) {
    for (Particle& particle : particles) {
        if (!particle.alive) continue;
        particle.life -= deltaTime;
        if (particle.life <= 0.0f) {
            particle.alive = false;
            continue;
        }
        const float drag = 1.0f - 2.2f * deltaTime;
        particle.velocity.x *= drag > 0.0f ? drag : 0.0f;
        particle.velocity.y *= drag > 0.0f ? drag : 0.0f;
        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;
    }
}

void ParticleSystem::Draw() const {
    for (const Particle& particle : particles) {
        if (!particle.alive) continue;
        const float ratio = particle.life / particle.maxLife;
        const float fade = ratio * ratio;
        Color color = particle.color;
        color.a = static_cast<unsigned char>(color.a * fade);
        DrawCircleV(particle.position, particle.size * (0.35f + 0.65f * ratio), color);
    }
}

void ParticleSystem::DrawGlow() const {
    for (const Particle& particle : particles) {
        if (!particle.alive) continue;
        const float ratio = particle.life / particle.maxLife;
        const float fade = ratio * ratio;
        Color color = particle.color;
        color.a = static_cast<unsigned char>(color.a * fade * 0.45f);
        DrawCircleV(particle.position, particle.size * 2.4f * (0.4f + 0.6f * ratio), color);
    }
}

void ParticleSystem::Clear() {
    for (Particle& particle : particles) particle.alive = false;
    nextSlot = 0;
}

void ParticleSystem::EmitThruster(Vector2 position, Vector2 direction, Color color, float strength) {
    const float clamped = strength < 0.0f ? 0.0f : (strength > 1.0f ? 1.0f : strength);
    const int count = clamped > 0.55f ? 2 : 1;
    for (int i = 0; i < count; ++i) {
        Particle particle;
        const float spread = Range(-28.0f, 28.0f);
        particle.position = Vector2{position.x + Range(-1.5f, 1.5f), position.y + Range(-1.5f, 1.5f)};
        const float speed = Range(55.0f, 130.0f) * (0.55f + 0.65f * clamped);
        particle.velocity = Vector2{direction.x * speed + -direction.y * spread,
                                    direction.y * speed + direction.x * spread};
        particle.maxLife = Range(0.16f, 0.34f);
        particle.life = particle.maxLife;
        particle.size = Range(1.5f, 3.0f);
        particle.color = Blend(color, Color{255, 170, 90, 255}, 0.30f);
        particle.alive = true;
        Spawn(particle);
    }
}

void ParticleSystem::EmitExplosion(Vector2 position, Color color, float scale) {
    const float clamped = scale < 0.4f ? 0.4f : (scale > 2.5f ? 2.5f : scale);
    const int count = static_cast<int>(12.0f * clamped) + 6;
    for (int i = 0; i < count; ++i) {
        Particle particle;
        const float angle = Range(0.0f, 2.0f * 3.14159265f);
        const float speed = Range(40.0f, 230.0f) * clamped;
        particle.position = Vector2{position.x + Range(-2.0f, 2.0f) * clamped,
                                    position.y + Range(-2.0f, 2.0f) * clamped};
        particle.velocity = Vector2{std::cos(angle) * speed, std::sin(angle) * speed};
        particle.maxLife = Range(0.30f, 0.75f);
        particle.life = particle.maxLife;
        particle.size = Range(1.4f, 3.2f) * clamped;
        particle.color = (i % 4 == 0) ? Blend(color, Color{255, 245, 220, 255}, 0.55f) : color;
        particle.alive = true;
        Spawn(particle);
    }
}

void ParticleSystem::EmitHitSpark(Vector2 position, Color color) {
    for (int i = 0; i < 7; ++i) {
        Particle particle;
        const float angle = Range(0.0f, 2.0f * 3.14159265f);
        const float speed = Range(80.0f, 210.0f);
        particle.position = position;
        particle.velocity = Vector2{std::cos(angle) * speed, std::sin(angle) * speed};
        particle.maxLife = Range(0.12f, 0.26f);
        particle.life = particle.maxLife;
        particle.size = Range(1.1f, 2.2f);
        particle.color = Blend(color, Color{255, 255, 245, 255}, 0.45f);
        particle.alive = true;
        Spawn(particle);
    }
}

void ParticleSystem::EmitCelebration(Vector2 position, Color color) {
    for (int i = 0; i < 10; ++i) {
        Particle particle;
        const float angle = Range(-2.6f, -0.5f);
        const float speed = Range(50.0f, 150.0f);
        particle.position = Vector2{position.x + Range(-14.0f, 14.0f), position.y + Range(-6.0f, 6.0f)};
        particle.velocity = Vector2{std::cos(angle) * speed, std::sin(angle) * speed};
        particle.maxLife = Range(0.5f, 1.1f);
        particle.life = particle.maxLife;
        particle.size = Range(1.3f, 2.6f);
        particle.color = (i % 3 == 0) ? Color{255, 255, 235, 255} : color;
        particle.alive = true;
        Spawn(particle);
    }
}
