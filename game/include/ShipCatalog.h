#pragma once
#include "raylib.h"
#include <cstdint>

enum ShipShape : uint8_t {
    ShapeInterceptor = 0,
    ShapeStriker,
    ShapeBulwark,
    ShapeViper,
    ShapeLancer,
    ShapeWraith,
};

struct ShipDef {
    uint8_t id = 0;
    const char* name = "INTERCEPTOR";
    const char* passiveName = "VETERAN FRAME";
    const char* blurb = "Balanced all-rounder.";
    int health = 3;
    float thrust = 620.0f;
    float maxSpeed = 360.0f;
    float turnSpeed = 220.0f;
    float drag = 1.7f;
    float fireInterval = 0.22f;
    float bulletSpeed = 720.0f;
    float bulletRadius = 4.0f;
    float reverseMultiplier = 0.6f;
    float knockbackTaken = 1.0f;
    float invulnMultiplier = 1.0f;
    float regenDelay = 0.0f;
    float speedBonusWhileThrusting = 0.0f;
    int pierce = 0;
    Color hullColor{77, 210, 255, 255};
    Color accentColor{225, 250, 255, 255};
    ShipShape shape = ShapeInterceptor;
};

constexpr int kShipCount = 6;
constexpr float kShipRadius = 18.0f;

const ShipDef& GetShip(int id);
const ShipDef* GetAllShips();
