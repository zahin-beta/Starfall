#pragma once
#include "ShipCatalog.h"
#include "raylib.h"

namespace ShipRenderer {
void DrawShip(const ShipDef& ship, Vector2 position, float aimAngle, float radius,
              float throttle, bool reversing, bool invulnerable);
}
