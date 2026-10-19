#include "GlowLayer.h"
#include "rlgl.h"

void GlowLayer::Init(int width, int height) {
    texture = LoadRenderTexture(width / 2, height / 2);
    if (texture.id == 0) {
        active = false;
        return;
    }
    const char* shaderPaths[] = {"assets/shaders/glow_blur.fs", "../assets/shaders/glow_blur.fs"};
    shaderLoaded = false;
    for (const char* path : shaderPaths) {
        if (FileExists(path)) {
            blurShader = LoadShader(nullptr, path);
            if (blurShader.id > 0) {
                shaderLoaded = true;
                break;
            }
        }
    }
    active = true;
}

void GlowLayer::Shutdown() {
    if (shaderLoaded) {
        UnloadShader(blurShader);
        blurShader = Shader{};
        shaderLoaded = false;
    }
    if (texture.id > 0) {
        UnloadRenderTexture(texture);
        texture = RenderTexture2D{};
    }
    active = false;
}

void GlowLayer::Begin() const {
    if (!active) return;
    BeginTextureMode(texture);
    ClearBackground(BLANK);
}

void GlowLayer::End() const {
    if (!active) return;
    EndTextureMode();
}

void GlowLayer::Draw() const {
    if (!active) return;
    const Rectangle source{0.0f, 0.0f, static_cast<float>(texture.texture.width),
                           static_cast<float>(-texture.texture.height)};
    const Rectangle dest{0.0f, 0.0f, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    rlSetBlendMode(RL_BLEND_ADDITIVE);
    if (shaderLoaded) {
        BeginShaderMode(blurShader);
        DrawTexturePro(texture.texture, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        EndShaderMode();
    } else {
        const Vector2 offsets[5] = {{0.0f, 0.0f}, {1.6f, 0.0f}, {-1.6f, 0.0f}, {0.0f, 1.6f}, {0.0f, -1.6f}};
        for (const Vector2& offset : offsets) {
            Rectangle shifted = dest;
            shifted.x += offset.x;
            shifted.y += offset.y;
            DrawTexturePro(texture.texture, source, shifted, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        }
    }
    rlSetBlendMode(RL_BLEND_ALPHA);
}
