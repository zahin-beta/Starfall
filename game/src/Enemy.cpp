#include "Enemy.h"
void Enemy::Update(float deltaTime) { (void)deltaTime; }
void Enemy::Draw() const {
    if (active) DrawCircleV(position, 18.0f, RED);
}
