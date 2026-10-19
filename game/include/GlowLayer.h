#pragma once
#include "raylib.h"

class GlowLayer {
public:
    void Init(int width, int height);
    void Shutdown();
    void Begin() const;
    void End() const;
    void Draw() const;
    bool IsActive() const { return active; }

private:
    RenderTexture2D texture{};
    Shader blurShader{};
    bool shaderLoaded = false;
    bool active = false;
};
