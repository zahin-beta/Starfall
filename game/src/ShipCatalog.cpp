#include "ShipCatalog.h"

namespace {
const ShipDef kShips[kShipCount] = {
    {
        0, "INTERCEPTOR", "VETERAN FRAME",
        "Balanced all-rounder. Recover from hits faster than anyone else.",
        3, 620.0f, 360.0f, 220.0f, 1.70f, 0.22f, 720.0f, 4.0f,
        0.6f, 1.0f, 1.4f, 0.0f, 0.0f, 0,
        Color{77, 210, 255, 255}, Color{225, 250, 255, 255}, ShapeInterceptor
    },
    {
        1, "STRIKER", "AFTERBURNER",
        "Fastest ship in the game but hull 2. Reverse thrust hits full power.",
        2, 840.0f, 480.0f, 200.0f, 1.30f, 0.20f, 800.0f, 3.5f,
        1.0f, 1.15f, 1.0f, 0.0f, 0.0f, 0,
        Color{255, 112, 86, 255}, Color{255, 224, 168, 255}, ShapeStriker
    },
    {
        2, "BULWARK", "REINFORCED HULL",
        "Hull 5 and barely moves. Takes half knockback from everything.",
        5, 420.0f, 250.0f, 150.0f, 2.30f, 0.42f, 620.0f, 5.5f,
        0.6f, 0.5f, 1.0f, 0.0f, 0.0f, 0,
        Color{154, 174, 204, 255}, Color{214, 230, 250, 255}, ShapeBulwark
    },
    {
        3, "VIPER", "HOT THRUSTERS",
        "Turns on a dime and shoots fast. Gains 20% top speed while thrusting.",
        3, 640.0f, 375.0f, 340.0f, 1.95f, 0.15f, 640.0f, 3.5f,
        0.6f, 1.0f, 1.0f, 0.0f, 0.20f, 0,
        Color{118, 236, 162, 255}, Color{222, 255, 232, 255}, ShapeViper
    },
    {
        4, "LANCER", "PIERCING ROUNDS",
        "Slow to swing and slow to fire, but every shot is fast and pierces one asteroid.",
        3, 520.0f, 330.0f, 165.0f, 1.55f, 0.55f, 1050.0f, 4.5f,
        0.6f, 1.0f, 1.0f, 0.0f, 0.0f, 1,
        Color{216, 140, 252, 255}, Color{246, 222, 255, 255}, ShapeLancer
    },
    {
        5, "WRAITH", "LIGHT FRAME",
        "Floaty and hard to stop. Regains 1 hull after 8 seconds without damage.",
        4, 560.0f, 400.0f, 235.0f, 0.85f, 0.30f, 700.0f, 4.0f,
        0.6f, 1.2f, 1.0f, 8.0f, 0.0f, 0,
        Color{116, 144, 255, 255}, Color{186, 204, 255, 255}, ShapeWraith
    },
};
}

const ShipDef& GetShip(int id) {
    if (id < 0 || id >= kShipCount) return kShips[0];
    return kShips[id];
}

const ShipDef* GetAllShips() { return kShips; }
