#include "Background.h"
#include <random>

namespace {
constexpr int kLayerCount = 3;
constexpr int kStarsPerLayer[kLayerCount] = {70, 42, 22};
constexpr float kLayerSpeed[kLayerCount] = {7.0f, 15.0f, 28.0f};
constexpr float kLayerMinRadius[kLayerCount] = {0.6f, 0.9f, 1.3f};
constexpr float kLayerMaxRadius[kLayerCount] = {1.0f, 1.5f, 2.2f};
constexpr unsigned char kLayerMinAlpha[kLayerCount] = {70, 110, 155};
constexpr unsigned char kLayerMaxAlpha[kLayerCount] = {120, 175, 240};
constexpr float kMargin = 24.0f;

struct PlanetTone {
    Color base;
    Color shade;
    Color ring;
};

const PlanetTone kPlanetTones[] = {
    {{36, 52, 88, 255}, {24, 34, 60, 255}, {54, 72, 110, 255}},
    {{58, 44, 78, 255}, {40, 30, 56, 255}, {76, 60, 100, 255}},
};
constexpr int kPlanetToneCount = 2;

void DrawNebulaBlob(float x, float y, float radius, Color color) {
    for (int i = 0; i < 5; ++i) {
        const float ox = x + static_cast<float>(((i * 73) % 120) - 60);
        const float oy = y + static_cast<float>(((i * 131) % 90) - 45);
        const float r = radius * (0.55f + 0.11f * static_cast<float>(i % 3));
        DrawCircleGradient(Vector2{ox, oy}, r, color, Color{0, 0, 0, 0});
    }
}
}

void Background::Init(int width, int height) {
    std::mt19937 rng(0x57A41Fu);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    for (int layerIndex = 0; layerIndex < kLayerCount; ++layerIndex) {
        Layer& layer = layers[layerIndex];
        layer.stars.clear();
        layer.stars.reserve(kStarsPerLayer[layerIndex]);
        for (int i = 0; i < kStarsPerLayer[layerIndex]; ++i) {
            Star star;
            star.x = unit(rng) * static_cast<float>(width);
            star.y = unit(rng) * static_cast<float>(height);
            star.speed = kLayerSpeed[layerIndex] * (0.75f + 0.5f * unit(rng));
            star.radius = kLayerMinRadius[layerIndex] +
                          (kLayerMaxRadius[layerIndex] - kLayerMinRadius[layerIndex]) * unit(rng);
            star.maxAlpha = static_cast<unsigned char>(
                kLayerMinAlpha[layerIndex] +
                (kLayerMaxAlpha[layerIndex] - kLayerMinAlpha[layerIndex]) * unit(rng));
            layer.stars.push_back(star);
        }
    }

    nebulaTexture = LoadRenderTexture(width, height);
    if (nebulaTexture.id > 0) {
        BeginTextureMode(nebulaTexture);
        ClearBackground(BLANK);
        DrawNebulaBlob(static_cast<float>(width) * 0.22f, static_cast<float>(height) * 0.30f,
                       150.0f, Color{38, 56, 118, 26});
        DrawNebulaBlob(static_cast<float>(width) * 0.74f, static_cast<float>(height) * 0.68f,
                       175.0f, Color{74, 44, 110, 24});
        DrawNebulaBlob(static_cast<float>(width) * 0.55f, static_cast<float>(height) * 0.12f,
                       120.0f, Color{30, 70, 104, 18});
        EndTextureMode();
    }

    planets.clear();
    planets.push_back(Planet{static_cast<float>(width) * 0.78f, static_cast<float>(height) * 0.24f,
                             46.0f, 2.5f, 0, true});
    planets.push_back(Planet{static_cast<float>(width) * 0.16f, static_cast<float>(height) * 0.72f,
                             28.0f, 1.6f, 1, false});
    initialized = true;
}

void Background::Shutdown() {
    if (nebulaTexture.id > 0) {
        UnloadRenderTexture(nebulaTexture);
        nebulaTexture = RenderTexture2D{};
    }
    initialized = false;
}

void Background::Update(float deltaTime) {
    if (!initialized) return;
    const float height = static_cast<float>(GetScreenHeight());
    for (Layer& layer : layers) {
        for (Star& star : layer.stars) {
            star.y += star.speed * deltaTime;
            if (star.y > height + kMargin) star.y -= height + kMargin * 2.0f;
        }
    }
    nebulaOffsetY += 1.4f * deltaTime;
    planetOffsetY += 2.2f * deltaTime;
    if (nebulaOffsetY > height) nebulaOffsetY -= height;
    if (planetOffsetY > height) planetOffsetY -= height;
}

void Background::DrawPlanet(const Planet& planet, float offsetY, int height) const {
    const PlanetTone& tone = kPlanetTones[planet.palette % kPlanetToneCount];
    float y = planet.y + offsetY;
    for (int wrap = 0; wrap < 2; ++wrap) {
        const float drawY = wrap == 0 ? y : y - static_cast<float>(height);
        if (drawY < -planet.radius - 10.0f || drawY > static_cast<float>(height) + planet.radius + 10.0f) continue;
        const Vector2 center{planet.x, drawY};
        if (planet.ringed) {
            DrawRing(center, planet.radius + 7.0f, planet.radius + 10.0f, 24.0f, 156.0f, 24, tone.ring);
        }
        DrawCircleV(center, planet.radius, tone.base);
        DrawCircleSector(center, planet.radius, 90.0f, 270.0f, 32, tone.shade);
        const Color rim{static_cast<unsigned char>(tone.base.r + 14), static_cast<unsigned char>(tone.base.g + 16),
                        static_cast<unsigned char>(tone.base.b + 20), 255};
        DrawCircleLines(center.x, center.y, planet.radius, rim);
    }
}

void Background::Draw() const {
    ClearBackground(Color{4, 7, 18, 255});
    const int height = GetScreenHeight();
    if (nebulaTexture.id > 0) {
        const Rectangle source{0.0f, 0.0f, static_cast<float>(nebulaTexture.texture.width),
                               static_cast<float>(-nebulaTexture.texture.height)};
        Vector2 position{0.0f, nebulaOffsetY};
        DrawTextureRec(nebulaTexture.texture, source, position, WHITE);
        position.y -= static_cast<float>(height);
        DrawTextureRec(nebulaTexture.texture, source, position, WHITE);
    }
    for (const Planet& planet : planets) DrawPlanet(planet, planetOffsetY, height);
    for (const Layer& layer : layers) {
        for (const Star& star : layer.stars) {
            DrawCircleV(Vector2{star.x, star.y}, star.radius, Color{186, 214, 255, star.maxAlpha});
        }
    }
}
