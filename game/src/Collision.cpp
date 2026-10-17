#include "Collision.h"
bool Collision::CirclesOverlap(Vector2 first, float firstRadius, Vector2 second, float secondRadius) {
    const float dx = first.x - second.x, dy = first.y - second.y;
    const float combinedRadius = firstRadius + secondRadius;
    return dx * dx + dy * dy <= combinedRadius * combinedRadius;
}
