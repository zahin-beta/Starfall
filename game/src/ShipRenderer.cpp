#include "ShipRenderer.h"
#include <cmath>

namespace {
struct Frame {
    Vector2 origin{};
    Vector2 forward{};
    Vector2 right{};
    float radius = 18.0f;
};

struct Tri {
    float ax, ay, bx, by, cx, cy;
};

Vector2 P(const Frame& f, float lateral, float forward) {
    return Vector2{f.origin.x + (f.right.x * lateral + f.forward.x * forward) * f.radius,
                   f.origin.y + (f.right.y * lateral + f.forward.y * forward) * f.radius};
}

Vector2 Ps(const Frame& f, float lateral, float forward, float scale) {
    const Vector2 p = P(f, lateral, forward);
    return Vector2{f.origin.x + (p.x - f.origin.x) * scale,
                   f.origin.y + (p.y - f.origin.y) * scale};
}

unsigned char Clamp255(float value) {
    if (value < 0.0f) return 0;
    if (value > 255.0f) return 255;
    return static_cast<unsigned char>(value);
}

Color Darken(Color color, float factor) {
    return Color{Clamp255(color.r * factor), Clamp255(color.g * factor), Clamp255(color.b * factor), color.a};
}

Color Alpha(Color color, unsigned char value) {
    color.a = value;
    return color;
}

Color Blend(Color a, Color b, float t) {
    return Color{Clamp255(a.r + (b.r - a.r) * t), Clamp255(a.g + (b.g - a.g) * t),
                 Clamp255(a.b + (b.b - a.b) * t), Clamp255(a.a + (b.a - a.a) * t)};
}

// raylib's DrawTriangle back-face-culls: triangles wound clockwise in the
// y-down screen space are dropped. Normalize every triangle to the winding
// that survives so ship geometry always renders regardless of authored order.
void FillTriColored(Vector2 a, Vector2 b, Vector2 c, Color color) {
    const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross > 0.0f) {
        const Vector2 tmp = b;
        b = c;
        c = tmp;
    }
    DrawTriangle(a, b, c, color);
}

void FillTris(const Frame& f, const Tri* tris, int count, float scale, Color color) {
    for (int i = 0; i < count; ++i) {
        const Tri& t = tris[i];
        if (scale == 1.0f) {
            FillTriColored(P(f, t.ax, t.ay), P(f, t.bx, t.by), P(f, t.cx, t.cy), color);
        } else {
            FillTriColored(Ps(f, t.ax, t.ay, scale), Ps(f, t.bx, t.by, scale), Ps(f, t.cx, t.cy, scale), color);
        }
    }
}

void FillQuad(const Frame& f, float aL, float aF, float bL, float bF, float cL, float cF, float dL, float dF, Color color) {
    FillTriColored(P(f, aL, aF), P(f, bL, bF), P(f, cL, cF), color);
    FillTriColored(P(f, aL, aF), P(f, cL, cF), P(f, dL, dF), color);
}

void DrawFlame(const Frame& f, float throttle, bool reversing, Color accent) {
    if (throttle > 0.01f) {
        const float flicker = 0.80f + 0.14f * std::sin(GetTime() * 47.0f) + 0.06f * std::sin(GetTime() * 91.0f);
        const float length = (0.42f + 1.40f * throttle) * f.radius * flicker;
        const float halfWidth = 0.30f * f.radius * (0.65f + 0.35f * throttle);
        const float rear = -0.92f;

        FillTriColored(P(f, -halfWidth, rear), P(f, halfWidth, rear),
                       P(f, 0.0f, rear - length / f.radius), Alpha(Blend(Color{255, 132, 44, 255}, accent, 0.22f),
                                                                   Clamp255(60.0f + 150.0f * throttle)));

        const float midLength = length * 0.60f;
        const float midWidth = halfWidth * 0.60f;
        FillTriColored(P(f, -midWidth, rear), P(f, midWidth, rear),
                       P(f, 0.0f, rear - midLength / f.radius), Alpha(Blend(Color{255, 212, 116, 255}, accent, 0.15f),
                                                                     Clamp255(100.0f + 130.0f * throttle)));

        const float coreLength = length * 0.30f;
        const float coreWidth = halfWidth * 0.34f;
        FillTriColored(P(f, -coreWidth, rear), P(f, coreWidth, rear),
                       P(f, 0.0f, rear - coreLength / f.radius), Alpha(Color{255, 250, 236, 255},
                                                                      Clamp255(120.0f + 120.0f * throttle)));
    }
    if (reversing) {
        const float pulse = 0.5f + 0.5f * std::sin(GetTime() * 40.0f);
        DrawCircleV(P(f, 0.0f, 1.10f), 2.5f + 2.0f * pulse, Alpha(Color{176, 214, 255, 255}, 90));
    }
}

void DrawShield(Vector2 position, float radius) {
    const float pulse = 0.5f + 0.5f * std::sin(GetTime() * 14.0f);
    DrawCircleLines(position.x, position.y, radius + 5.0f, Alpha(Color{190, 236, 255, 255},
                                                                 Clamp255(60.0f + 90.0f * pulse)));
    for (int i = 0; i < 3; ++i) {
        const float start = GetTime() * 95.0f + i * 120.0f;
        DrawRing(position, radius + 3.5f, radius + 7.5f, start, start + 68.0f, 0,
                 Alpha(Color{222, 246, 255, 255}, Clamp255(110.0f + 90.0f * pulse)));
    }
}

void DrawTipLights(const Frame& f, float lateral, float forward, Color color) {
    const bool blink = static_cast<int>(GetTime() * 2.2) % 2 == 0;
    DrawCircleV(P(f, lateral, forward), 2.4f, Alpha(color, blink ? 255 : 90));
}

void DrawCockpit(const Frame& f, float forward, float width, float length, unsigned char alpha) {
    FillQuad(f, 0.0f, forward, width, forward - length * 0.55f, width * 0.72f, forward - length,
             -width * 0.72f, forward - length, Alpha(Color{12, 22, 44, 255}, alpha));
    FillQuad(f, 0.0f, forward, -width, forward - length * 0.55f, -width * 0.72f, forward - length,
             width * 0.72f, forward - length, Alpha(Color{12, 22, 44, 255}, alpha));
    DrawCircleV(P(f, -width * 0.35f, forward - length * 0.35f), 1.6f, Alpha(Color{225, 246, 255, 255}, alpha));
}

void DrawInterceptor(const Frame& f, Color hull, Color accent, Color outline, unsigned char alpha) {
    const Tri hullTris[] = {
        {0.00f, 1.15f, 0.95f, -0.50f, 0.00f, -0.42f},
        {0.00f, 1.15f, 0.00f, -0.42f, -0.95f, -0.50f},
        {0.95f, -0.50f, 0.34f, -0.70f, 0.00f, -0.42f},
        {-0.95f, -0.50f, 0.00f, -0.42f, -0.34f, -0.70f},
    };
    FillTris(f, hullTris, 4, 1.16f, outline);
    FillTris(f, hullTris, 4, 1.0f, Alpha(hull, alpha));

    const Tri spine[] = {{0.00f, 1.02f, 0.15f, -0.28f, -0.15f, -0.28f}};
    FillTris(f, spine, 1, 1.0f, Alpha(accent, alpha));
    DrawCockpit(f, 0.68f, 0.13f, 0.52f, alpha);
    DrawTipLights(f, 0.95f, -0.50f, Alpha(accent, alpha));
    DrawTipLights(f, -0.95f, -0.50f, Alpha(accent, alpha));
}

void DrawStriker(const Frame& f, Color hull, Color accent, Color outline, unsigned char alpha) {
    const Tri hullTris[] = {
        {0.00f, 1.25f, 0.17f, 0.35f, 0.20f, -0.25f},
        {0.00f, 1.25f, 0.20f, -0.25f, 0.78f, -0.88f},
        {0.00f, 1.25f, 0.78f, -0.88f, 0.22f, -0.60f},
        {0.00f, 1.25f, 0.22f, -0.60f, 0.00f, -0.92f},
        {0.00f, 1.25f, -0.17f, 0.35f, -0.20f, -0.25f},
        {0.00f, 1.25f, -0.20f, -0.25f, -0.78f, -0.88f},
        {0.00f, 1.25f, -0.78f, -0.88f, -0.22f, -0.60f},
        {0.00f, 1.25f, -0.22f, -0.60f, 0.00f, -0.92f},
    };
    FillTris(f, hullTris, 8, 1.14f, outline);
    FillTris(f, hullTris, 8, 1.0f, Alpha(hull, alpha));

    const Tri spine[] = {{0.00f, 1.16f, 0.08f, -0.52f, -0.08f, -0.52f}};
    FillTris(f, spine, 1, 1.0f, Alpha(accent, alpha));
    DrawCockpit(f, 0.62f, 0.10f, 0.44f, alpha);
    DrawTipLights(f, 0.78f, -0.88f, Alpha(accent, alpha));
    DrawTipLights(f, -0.78f, -0.88f, Alpha(accent, alpha));
}

void DrawBulwark(const Frame& f, Color hull, Color accent, Color outline, unsigned char alpha) {
    const Tri hullTris[] = {
        {0.00f, 1.00f, 1.05f, 0.05f, 1.05f, -0.50f},
        {0.00f, 1.00f, 1.05f, -0.50f, 0.55f, -0.90f},
        {0.00f, 1.00f, 0.55f, -0.90f, 0.00f, -0.72f},
        {0.00f, 1.00f, -1.05f, 0.05f, -1.05f, -0.50f},
        {0.00f, 1.00f, -1.05f, -0.50f, -0.55f, -0.90f},
        {0.00f, 1.00f, -0.55f, -0.90f, 0.00f, -0.72f},
    };
    FillTris(f, hullTris, 6, 1.14f, outline);
    FillTris(f, hullTris, 6, 1.0f, Alpha(hull, alpha));

    const Color plate = Alpha(Darken(hull, 0.72f), alpha);
    FillQuad(f, 0.60f, -0.02f, 1.00f, -0.02f, 1.00f, -0.42f, 0.60f, -0.42f, plate);
    FillQuad(f, -0.60f, -0.02f, -1.00f, -0.02f, -1.00f, -0.42f, -0.60f, -0.42f, plate);

    const Tri spine[] = {{0.00f, 0.88f, 0.24f, -0.52f, -0.24f, -0.52f}};
    FillTris(f, spine, 1, 1.0f, Alpha(accent, alpha));
    DrawCockpit(f, 0.52f, 0.17f, 0.46f, alpha);
    DrawTipLights(f, 1.05f, -0.20f, Alpha(accent, alpha));
    DrawTipLights(f, -1.05f, -0.20f, Alpha(accent, alpha));
}

void DrawViper(const Frame& f, Color hull, Color accent, Color outline, unsigned char alpha) {
    const Tri hullTris[] = {
        {0.00f, 1.20f, 0.16f, 0.15f, 0.18f, -0.70f},
        {0.00f, 1.20f, 0.18f, -0.70f, 0.00f, -0.88f},
        {0.00f, 1.20f, -0.16f, 0.15f, -0.18f, -0.70f},
        {0.00f, 1.20f, -0.18f, -0.70f, 0.00f, -0.88f},
    };
    const Tri wingTris[] = {
        {0.10f, 0.05f, 1.00f, -0.45f, 0.20f, -0.55f},
        {-0.10f, 0.05f, -1.00f, -0.45f, -0.20f, -0.55f},
    };
    FillTris(f, hullTris, 4, 1.16f, outline);
    FillTris(f, wingTris, 2, 1.16f, outline);
    FillTris(f, wingTris, 2, 1.0f, Alpha(Darken(hull, 0.86f), alpha));
    FillTris(f, hullTris, 4, 1.0f, Alpha(hull, alpha));

    const Tri spine[] = {{0.00f, 1.06f, 0.08f, -0.48f, -0.08f, -0.48f}};
    FillTris(f, spine, 1, 1.0f, Alpha(accent, alpha));
    DrawCockpit(f, 0.58f, 0.09f, 0.40f, alpha);
    DrawTipLights(f, 1.00f, -0.45f, Alpha(accent, alpha));
    DrawTipLights(f, -1.00f, -0.45f, Alpha(accent, alpha));
}

void DrawLancer(const Frame& f, Color hull, Color accent, Color outline, unsigned char alpha) {
    const Tri hullTris[] = {
        {0.00f, 1.30f, 0.11f, 0.35f, 0.13f, -0.20f},
        {0.00f, 1.30f, 0.13f, -0.20f, 0.16f, -0.68f},
        {0.00f, 1.30f, 0.16f, -0.68f, 0.58f, -0.72f},
        {0.00f, 1.30f, 0.58f, -0.72f, 0.00f, -0.95f},
        {0.00f, 1.30f, -0.11f, 0.35f, -0.13f, -0.20f},
        {0.00f, 1.30f, -0.13f, -0.20f, -0.16f, -0.68f},
        {0.00f, 1.30f, -0.16f, -0.68f, -0.58f, -0.72f},
        {0.00f, 1.30f, -0.58f, -0.72f, 0.00f, -0.95f},
    };
    FillTris(f, hullTris, 8, 1.12f, outline);
    FillTris(f, hullTris, 8, 1.0f, Alpha(hull, alpha));

    const Tri spine[] = {{0.00f, 1.22f, 0.05f, -0.58f, -0.05f, -0.58f}};
    FillTris(f, spine, 1, 1.0f, Alpha(accent, alpha));
    DrawCockpit(f, 0.44f, 0.08f, 0.34f, alpha);
    DrawTipLights(f, 0.58f, -0.72f, Alpha(accent, alpha));
    DrawTipLights(f, -0.58f, -0.72f, Alpha(accent, alpha));
}

void DrawWraith(const Frame& f, Color hull, Color accent, Color outline, unsigned char alpha) {
    const Tri hullTris[] = {
        {0.00f, 1.15f, 0.88f, -0.28f, 0.48f, -0.75f},
        {0.00f, 1.15f, 0.48f, -0.75f, 0.00f, -0.55f},
        {0.00f, 1.15f, -0.88f, -0.28f, -0.48f, -0.75f},
        {0.00f, 1.15f, -0.48f, -0.75f, 0.00f, -0.55f},
    };
    FillTris(f, hullTris, 4, 1.14f, outline);
    FillTris(f, hullTris, 4, 1.0f, Alpha(hull, alpha));

    const Tri inner[] = {
        {0.00f, 0.86f, 0.38f, -0.26f, 0.00f, -0.42f},
        {0.00f, 0.86f, 0.00f, -0.42f, -0.38f, -0.26f},
    };
    FillTris(f, inner, 2, 1.0f, Alpha(accent, alpha));
    DrawCircleLines(P(f, 0.0f, -0.62f).x, P(f, 0.0f, -0.62f).y, 0.34f * f.radius,
                    Alpha(accent, alpha));
    DrawCockpit(f, 0.40f, 0.11f, 0.36f, alpha);
    DrawTipLights(f, 0.88f, -0.28f, Alpha(accent, alpha));
    DrawTipLights(f, -0.88f, -0.28f, Alpha(accent, alpha));
}
}

void ShipRenderer::DrawShip(const ShipDef& ship, Vector2 position, float aimAngle, float radius,
                            float throttle, bool reversing, bool invulnerable) {
    const float radians = aimAngle * DEG2RAD;
    Frame frame;
    frame.origin = position;
    frame.forward = Vector2{std::sin(radians), -std::cos(radians)};
    frame.right = Vector2{-frame.forward.y, frame.forward.x};
    frame.radius = radius;

    const Color outline = invulnerable ? Color{206, 242, 255, 255} : Color{6, 11, 26, 255};
    const unsigned char alpha = invulnerable ? 150 : 255;
    const Color hull = Alpha(ship.hullColor, alpha);
    const Color accent = Alpha(ship.accentColor, alpha);

    DrawFlame(frame, throttle, reversing, ship.accentColor);

    switch (ship.shape) {
        case ShapeInterceptor: DrawInterceptor(frame, hull, accent, outline, alpha); break;
        case ShapeStriker: DrawStriker(frame, hull, accent, outline, alpha); break;
        case ShapeBulwark: DrawBulwark(frame, hull, accent, outline, alpha); break;
        case ShapeViper: DrawViper(frame, hull, accent, outline, alpha); break;
        case ShapeLancer: DrawLancer(frame, hull, accent, outline, alpha); break;
        case ShapeWraith: DrawWraith(frame, hull, accent, outline, alpha); break;
    }

    if (invulnerable) DrawShield(position, radius);
}
