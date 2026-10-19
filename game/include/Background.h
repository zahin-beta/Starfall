#pragma once
#include "raylib.h"
#include <vector>

class Background {
public:
    void Init(int width, int height);
    void Shutdown();
    void Update(float deltaTime);
    void Draw() const;

private:
    struct Star {
        float x = 0.0f;
        float y = 0.0f;
        float speed = 0.0f;
        float radius = 1.0f;
        unsigned char maxAlpha = 255;
    };
    struct Layer {
        std::vector<Star> stars;
    };
    struct Planet {
        float x = 0.0f;
        float y = 0.0f;
        float radius = 0.0f;
        float speed = 0.0f;
        int palette = 0;
        bool ringed = false;
    };

    void DrawPlanet(const Planet& planet, float offsetY, int height) const;

    Layer layers[3];
    std::vector<Planet> planets;
    RenderTexture2D nebulaTexture{};
    float nebulaOffsetY = 0.0f;
    float planetOffsetY = 0.0f;
    bool initialized = false;
};
