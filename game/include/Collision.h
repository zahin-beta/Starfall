#pragma once
#include "raylib.h"
class Collision {
public:
    static bool CirclesOverlap(Vector2 first, float firstRadius, Vector2 second, float secondRadius);
};
